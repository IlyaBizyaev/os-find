#ifndef PROGRAMCONDITIONS_H
#define PROGRAMCONDITIONS_H

#include <vector>
#include <map>
#include <string>

/*!
 * Stores program environmemntal variables and arguments,
 * converts them to C arrays and manages them in memory
 */
class ProgramConditions
{
public:
    ProgramConditions(const std::vector<std::string>& args,
                      const std::map<std::string, std::string>& env);
    ~ProgramConditions();

    char** get_argv();
    char** get_envp();
private:
    char** m_argv;
    char** m_envp;
};

#endif // PROGRAMCONDITIONS_H
