#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <limits>
#include <system_error>

#include "actor.h"
#include "environment.h"
#include "platform.h"
#include "query.h"

void print_help()
{
    std::cout << "find - locate specified files recursively in the provided path\n"
                 "find <path> [flags]\n"
                 "\n"
                 "Supported arguments are:\n"
                 "-name   <name>    - filename mathes name (wildcard)\n"
                 "-inum   <num>     - inode number equals num\n"
                 "-size   <[-=+]sz> - filter by size (less than, exactly, more than)\n"
                 "-nlinks <num>     - hardlink count equals num\n"
                 "-exec   <ex>      - pass located files to executable ex\n" << std::flush;
}

std::vector<std::string> read_arguments(int argc, char* const argv[])
{
    std::vector<std::string> result;
    result.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        result.emplace_back(argv[i]);
    }
    return result;
}

int check_directory(const std::string &filename)
{
    struct stat statbuf;
    int status = stat(filename.c_str(), &statbuf);
    if (!status) {
        if (S_ISDIR(statbuf.st_mode)) {
            return 0;
        } else {
            return ENOTDIR;
        }
    } else {
        return errno;
    }
}

void run_search(const std::string& root_path, const Query& query, Actor& actor)
{
    if (query.matches(root_path)) {
        actor.process(root_path);
    }

    std::queue<std::string> queue;
    queue.push(root_path);

    const size_t BUF_SIZE = 65536;
    static char buf[BUF_SIZE];

    while (!queue.empty()) {
        std::string path = queue.front();
        queue.pop();

        const int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);

        if (fd == -1) {
            std::cerr << "Failed to open directory '" << path << "': " << strerror(errno) << std::endl;
            continue;
        }

        for (;;) {
            auto nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
            if (nread == -1) {
                std::cerr << "Failed to list contents for '" << path << "': " << strerror(errno) << std::endl;
                break;
            }

            if (nread == 0) {
                break;
            }

            struct linux_dirent64 *d;
            for (int pos = 0; pos < nread; pos += d->d_reclen) {
                d = reinterpret_cast<struct linux_dirent64*>(buf + pos);

                std::string name = static_cast<const char*>(d->d_name);
                if (name == "." || name == "..") {
                    continue;
                }
                std::string file_path = path + name;
                if (d->d_type == DT_DIR) {
                    queue.push(file_path + PATH_SEPARATOR);
                }
                // TODO: optionally follow symlinks (d_type == DT_LNK)

                try {
                    if (query.matches(file_path, d)) {
                        actor.process(file_path);
                    }
                } catch (std::system_error& e) {
                    std::cerr << "Failed to request file information: " << e.what();
                }
            }
        }
        if (close(fd) == -1) {
            std::cerr << "Failed to close directory descriptor: " << strerror(errno);
        }
    }
}

bool parse_arguments(const std::vector<std::string>& args, const Environment& environ,
                     std::string& root_path, Query& query, Actor& actor)
{
    if (args.size() <= 1 || args[1] == "-help" || args[1] == "--help") {
        print_help();
        return false;
    }
    root_path = args[1];
    if (root_path.back() != PATH_SEPARATOR) {
        root_path += PATH_SEPARATOR;
    }
    int directory_status = check_directory(root_path);
    if (directory_status != 0) {
        std::cerr << "Cannot search in '" << root_path << "': " << strerror(directory_status) << std::endl;
        return false;
    }

    for (size_t i = 2; i < args.size(); i += 2) {
        const auto& argument = args[i];
        if (i + 1 >= args.size()) {
            std::cerr << "No value provided for argument " << argument << std::endl;
            return false;
        }
        const auto& value = args[i + 1];
        if (argument == "-name") {
            if (Query::validate_wildcard(value)) {
                query.set_wildcard(value);
            } else {
                std::cerr << "'" << value << "' is not a valid regexp" << std::endl;
                return false;
            }
        } else if (argument == "-inum") {
            std::istringstream ss(value);
            size_t inode;
            if (ss >> inode) {
                query.set_inode(inode);
            } else {
                std::cerr << "'" << value << "'is not a valid inode number" << std::endl;
                return false;
            }
        } else if (argument == "-size") {
            std::string num = value;
            char range = num.front();
            if (range == '=' || range == '+' || range == '-') {
                num = num.substr(1);
            }

            std::istringstream ss(num);
            size_t size;
            if (ss >> size) {
                switch (range) {
                case '-':
                    if (!size) {
                        std::cerr << "Files cannot have negative sizes" << std::endl;
                        return false;
                    }
                    query.set_max_size(size - 1);
                    break;
                case '+':
                    if (size == std::numeric_limits<decltype (size)>::max()) {
                        std::cerr << "Cannot search files bigger than maximum size" << std::endl;
                        return false;
                    }
                    query.set_min_size(size + 1);
                    break;
                case '=':
                default:
                    query.set_exact_size(size);
                }
            } else {
                std::cerr << "'" << value << "'is not a valid size" << std::endl;
                return false;
            }
        } else if (argument == "-nlinks") {
            std::istringstream ss(value);
            size_t hardlinks;
            if (ss >> hardlinks) {
                query.set_hardlinks(hardlinks);
            } else {
                std::cerr << "'" << value << "'is not a valid hardlinks count" << std::endl;
            }
        } else if (argument == "-exec") {
            auto executable_name = environ.find_executable(value);
            if (!executable_name.empty()) {
                actor.set_executable(executable_name);
            } else {
                std::cerr << "Cannot find executable '" << value << "' in PATH" << std::endl;
                return false;
            }
        } else {
            std::cerr << "Unrecognized argument: " << argument << std::endl;
            return false;
        }
    }

    if (!query.validate_size_range()) {
        std::cerr << "Specified size range is invalid" << std::endl;
    }
    return true;
}

int main(int argc, char *argv[], char *envp[])
{
    auto args = read_arguments(argc, argv);
    Environment environ(envp);
    std::string root_path;
    Query query;
    Actor actor;

    if (!parse_arguments(args, environ, root_path, query, actor)) {
        return EXIT_FAILURE;
    }

    run_search(root_path, query, actor);

    try {
        actor.submit(environ.get_variables());
    } catch (std::system_error& e) {
        std::cerr << "Failed to execute the recepient: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
