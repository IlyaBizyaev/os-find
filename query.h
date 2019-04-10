#ifndef QUERY_H
#define QUERY_H

#include <sys/stat.h>

#include <string>

#include "platform.h"

/*!
 * Stores and verifies files search queries
 */
class Query
{
public:
    Query();

    static bool validate_wildcard(const std::string& wildcard);

    void set_wildcard(std::string wildcard);
    void set_inode(size_t inode);
    void set_min_size(size_t min_size);
    void set_max_size(size_t max_size);
    void set_exact_size(size_t size);
    void set_hardlinks(size_t hardlinks);

    bool validate_size_range() const;

    bool matches(const std::string& file_path, linux_dirent64 const * const dirent = nullptr) const;
private:
    struct stat get_stat(const std::string& file_path) const;

    enum SearchCriterion
    {
        SC_NONE      = 0,
        SC_NAME      = 1 << 0,
        SC_INODE     = 1 << 1,
        SC_SIZE      = 1 << 2,
        SC_HARDLINKS = 1 << 3
    };

    int m_criteria;
    std::string m_wildcard;
    size_t m_inode;
    size_t m_hardlinks;
    size_t m_min_size, m_max_size;
};

#endif // QUERY_H
