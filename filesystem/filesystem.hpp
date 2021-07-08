#ifndef FSLIB_H
#define FSLIB_H

#include <string>

#ifdef _WIN32

#include <Windows.h>

using string = std::wstring;
using char_t = wchar_t;

#else

#include <dirent.h>

using string = std::string;
using char_t = char;

#endif

// windows version stolen from
// http://www.martinbroadhurst.com/list-the-files-in-a-directory-in-c.html

namespace FSLib {

bool exists( const string &path );
bool isDirectory( const string &path );
bool rename( const string &file_a, const string &file_b );
string canonical( const string &path );
bool createDirectoryFull( const string &path );
string getContainingDirectory( const string &path );
string getFileName( const string &path );
string getFileExtension( const string &path );
extern char dir_divisor;

class Directory {
public:
    Directory() = delete;
    explicit Directory( const string &path_ );
    explicit Directory( const Directory &d ) = default;
    explicit Directory( Directory &&d ) = default;

    class Iterator {
    public:
        explicit Iterator( const Directory &d_ );
        ~Iterator();

#ifdef _WIN32
        explicit Iterator( bool ended_ );
#else
        Iterator( const Directory &d_, const struct dirent *current_entry_ );
#endif

        Iterator() = delete;

        Iterator( const Iterator &i ) = default;

        Iterator( Iterator &&i ) = default;

        char_t const *operator*() const;

        Iterator &operator++();

        bool operator==( const Iterator &i_other ) const;

        Iterator operator++( int ) {
            Iterator ret( *this );
            operator++();
            return ret;
        }

        bool operator!=( const Iterator &i_other ) const {
            return !( i_other == *this );
        }

    private:
#ifndef _WIN32
        DIR *d{};
        const struct dirent *current_entry{};
#else
        HANDLE hFind{};
        WIN32_FIND_DATA data{};
        bool ended{ false };
#endif
    };

    using iterator = Iterator;
    using const_iterator = Iterator;

    iterator end();

    const_iterator end() const;

    iterator begin() {
        return Iterator( *this );
    }

    const_iterator begin() const {
        return Iterator( *this );
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }

    const char_t *path() const {
        return dir_path.c_str();
    }

#ifdef _WIN32
    const char_t *validPath() const {
        return dir_path.substr( 0, dir_path.length() - 2 ).c_str();
    }
#endif

private:
    string dir_path;
};
} // namespace FSLib

#endif
