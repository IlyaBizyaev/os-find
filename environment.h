#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <map>
#include <string>
#include <vector>

/*!
 * Manages shell environment variables
 * For PATH, keeps an up-to-date searchable paths list
 */
class Environment
{
public:
    explicit Environment(char *envp[]);

    void update_paths(const std::string& path_value);
    void set_variable(const std::string& assignment);
    void unset_variable(const std::string& var);
    const std::map<std::string, std::string>& get_variables();
    std::string find_executable(const std::string& filename) const;
private:
    bool executable_exists(const std::string& filename) const;

    std::map<std::string, std::string> m_environ;
    std::vector<std::string> m_paths;
};

#endif // ENVIRONMENT_H
