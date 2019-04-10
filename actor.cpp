#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "actor.h"
#include "programconditions.h"

int execute_program(const std::string& path,
                    const std::vector<std::string>& args,
                    const std::map<std::string, std::string>& env)
{
    pid_t pid = fork();
    if (!pid) {
        // We are a child
        ProgramConditions conditions(args, env);
        execve(path.c_str(), conditions.get_argv(), conditions.get_envp());
        // This code will only execute if execve has failed
        std::cerr << "Failed to execute the program: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // We are a parent
        int status;
        pid_t result = waitpid(pid, &status, 0);
        if (result > 0) {
            if (WIFEXITED(status)) {
                // terminated normally
                int exit_code = WEXITSTATUS(status);
                return exit_code;
            } else { // WIFSIGNALED(status)
                // terminated due to an uncaught signal
                int signal_number = WTERMSIG(status);
                return -signal_number;
            }
        } else {
            // Process wait failure
            std::error_code ec(errno, std::system_category());
            throw std::system_error(ec, "Failed to wait for child return");
        }
    } else {
        // Fork failure
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Failed to spawn child process");
    }
}

void Actor::set_executable(const std::string &executable)
{
    m_args = { executable };
}

void Actor::process(const std::string &filename)
{
    if (m_args.empty()) {
        std::cout << filename << std::endl;
    } else {
        m_args.push_back(filename);
    }
}

void Actor::submit(const std::map<std::string, std::string> &env)
{
    if (!m_args.empty()) {
        execute_program(m_args.front(), m_args, env);
    }
}
