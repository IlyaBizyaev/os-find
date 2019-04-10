#include <sys/types.h>
#include <unistd.h>

#include <libgen.h>
#include <fnmatch.h>
#include <string.h>
#include <memory>
#include <limits>
#include <system_error>

#include "query.h"

Query::Query() : m_criteria(SC_NONE),
    m_min_size(0), m_max_size(std::numeric_limits<size_t>::max()) {}

bool Query::validate_wildcard(const std::string& wildcard)
{
    auto result = fnmatch(wildcard.c_str(), "", 0);
    return result == 0 || result == FNM_NOMATCH;
}

void Query::set_wildcard(std::string wildcard)
{
    m_wildcard = wildcard;
    m_criteria |= SC_NAME;
}

void Query::set_inode(size_t inode)
{
    m_inode = inode;
    m_criteria |= SC_INODE;
}

void Query::set_min_size(size_t min_size)
{
    m_min_size = std::max(m_min_size, min_size);
    m_criteria |= SC_SIZE;
}

void Query::set_max_size(size_t max_size)
{
    m_max_size = std::min(m_max_size, max_size);
    m_criteria |= SC_SIZE;
}

void Query::set_exact_size(size_t size)
{
    m_min_size = m_max_size = size;
    m_criteria |= SC_SIZE;
}

void Query::set_hardlinks(size_t hardlinks)
{
    m_hardlinks = hardlinks;
    m_criteria |= SC_HARDLINKS;
}

bool Query::validate_size_range() const
{
    return m_min_size <= m_max_size;
}

bool Query::matches(const std::string &file_path, const linux_dirent64 * const dirent) const
{
    struct stat st;
    bool stat_requested = false;

    if (m_criteria & SC_NAME) {
        std::string filename;
        if (dirent != nullptr) {
            filename = static_cast<const char*>(dirent->d_name);
        } else {
            auto copy = std::make_unique<char[]>(file_path.length() + 1);
            memcpy(copy.get(), file_path.c_str(), file_path.length() + 1);
            filename = basename(copy.get());
        }
        if (fnmatch(m_wildcard.c_str(), filename.c_str(), 0) != 0) {
            return false;
        }
    }

    if (m_criteria & SC_INODE) {
        size_t inode;
        if (dirent != nullptr) {
            inode = dirent->d_ino;
        } else {
            st = get_stat(file_path);
            stat_requested = true;
            inode = st.st_ino;
        }
        if (inode != m_inode) {
            return false;
        }
    }

    if (m_criteria & SC_SIZE || m_criteria & SC_HARDLINKS) {
        if (!stat_requested) {
            st = get_stat(file_path);
        }

        if (m_criteria & SC_SIZE) {
            auto sz = static_cast<size_t>(st.st_size);
            if (sz < m_min_size || sz > m_max_size) {
                return false;
            }
        }

        if (m_criteria & SC_HARDLINKS) {
            if (st.st_nlink != m_hardlinks) {
                return false;
            }
        }
    }

    return true;
}

struct stat Query::get_stat(const std::string &file_path) const
{
    struct stat stat_buf;

    if (stat(file_path.c_str(), &stat_buf) == -1) {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "stat failed");
    }

    return stat_buf;
}
