#include <unistd.h>

#include <sstream>

#include "environment.h"

Environment::Environment(char *envp[])
{
    for (; *envp != nullptr; ++envp) {
        set_variable(std::string(*envp));
    }
    if (m_environ.find("PATH") == m_environ.end()) {
        // Assuming default value for LSB
        set_variable("PATH=/bin:/sbin:/usr/bin:/usr/sbin");
    }
}

void Environment::update_paths(const std::string& path_value)
{
    m_paths.clear();
    std::stringstream ss(path_value);

    std::string segment;
    while (std::getline(ss, segment, ':'))
    {
       m_paths.push_back(segment);
    }
}

void Environment::set_variable(const std::string& assignment)
{
    size_t eq = assignment.find('=');
    std::string key, value;
    if (eq != std::string::npos) {
        key = assignment.substr(0, eq);
        value = assignment.substr(eq + 1);
    } else {
        key = assignment;
        value = "";
    }

    m_environ[key] = value;
    if (key == "PATH") {
        update_paths(value);
    }
}

void Environment::unset_variable(const std::string& var)
{
    m_environ.erase(var);
}

const std::map<std::string, std::string>& Environment::get_variables()
{
    return m_environ;
}

std::string Environment::find_executable(const std::string& filename) const
{
    if (executable_exists(filename)) {
        return filename;
    }
    for (const auto& prefix : m_paths) {
        std::string attempt = prefix + "/" + filename;
        if (executable_exists(attempt)) {
            return attempt;
        }
    }
    return "";
}

bool Environment::executable_exists(const std::string &filename) const
{
    return access(filename.c_str(), X_OK) != -1;
}
