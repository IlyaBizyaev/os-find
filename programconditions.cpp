#include <string.h>

#include "programconditions.h"

ProgramConditions::ProgramConditions(const std::vector<std::string>& args,
                                     const std::map<std::string, std::string>& env)
{
    m_argv = new char*[args.size() + 1];
    m_envp = new char*[env.size() + 1];
    m_argv[args.size()] = nullptr;
    m_envp[env.size()] = nullptr;

    for (size_t i = 0; i < args.size(); ++i) {
        m_argv[i] = new char[args[i].length() + 1];
        strcpy(m_argv[i], args[i].c_str());
    }

    auto it = env.begin();
    for (size_t i = 0; i < env.size(); ++i, ++it) {
        std::string assignment = it->first + "=" + it->second;
        m_envp[i] = new char[assignment.length() + 1];
        strcpy(m_envp[i], assignment.c_str());
    }
}

ProgramConditions::~ProgramConditions()
{
    for (auto argv = m_argv; *argv != nullptr; ++argv) {
        delete[] *argv;
    }
    for (auto envp = m_envp; *envp != nullptr; ++envp) {
        delete[] *envp;
    }
    delete[] m_argv;
    delete[] m_envp;
}

char** ProgramConditions::get_argv()
{
    return m_argv;
}

char** ProgramConditions::get_envp()
{
    return m_envp;
}
