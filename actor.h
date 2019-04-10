#ifndef ACTOR_H
#define ACTOR_H

#include <string.h>
#include <vector>
#include <map>

/*!
 * Manages actions for located files
 * By default, prints file list to stdout
 */
class Actor
{
public:
    Actor() = default;

    void set_executable(const std::string& executable);
    void process(const std::string& filename);
    void submit(const std::map<std::string, std::string>& env);
private:
    std::vector<std::string> m_args;
};

#endif // ACTOR_H
