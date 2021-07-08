#include "../filesystem.hpp"
#include <Shlwapi.h>
#include <cstring>
#include <stdexcept>
#include <windows.h>

char FSLib::dir_divisor = '\\';

FSLib::Directory::Directory( const string &path_ ) : dir_path( path_ ) {
    // need to append \\* for windows to search files in directory
    dir_path.append( L"\\*" );
}

FSLib::Directory::Iterator::Iterator( const Directory &d_ ) {
    if ( !exists( d_.validPath() ) || !isDirectory( d_.validPath() ) ) {
        throw std::runtime_error(
            "Directory either doesn't exist or isn't a directory" );
    }
    hFind = FindFirstFileW( d_.path(), &data );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        if ( !wcscmp( data.cFileName, L"." ) ||
             !wcscmp( data.cFileName, L".." ) ) {
            ++( *this );
        }
    } else {
        ended = true;
    }
}

FSLib::Directory::Iterator::~Iterator() {
    if ( hFind )
        FindClose( hFind );
}

// this is definitely not a good way to create the "end" iterator
// but it was the only way I thought of with my limited knowledge of
// windows.h
FSLib::Directory::Iterator::Iterator( bool ended_ ) : ended( ended_ ) {}

bool FSLib::exists( const string &path ) {
    struct _stat path_stat;
    return _wstat( path.c_str(), &path_stat ) == 0;
}

string FSLib::canonical( const string &path ) {
    char_t *full_path = new char_t[MAX_PATH];
    char_t *canonical_path = new char_t[MAX_PATH];

    auto failed = !GetFullPathName( path.c_str(), MAX_PATH, full_path, NULL );

    if ( failed ) {
        delete[] canonical_path;
        delete[] full_path;
        return string();
    }

    failed = !PathCanonicalizeW( canonical_path, full_path );

    delete[] full_path;

    if ( failed ) {
        delete[] canonical_path;
        return string();
    }

    string canonical_string{ canonical_path };
    delete[] canonical_path;
    return canonical_string;
}

bool FSLib::isDirectory( const string &path ) {
    struct _stat path_stat;

    if ( _wstat( path.c_str(), &path_stat ) != 0 )
        return false;

    return path_stat.st_mode & _S_IFDIR;
}

bool FSLib::rename( const string &file_a, const string &file_b ) {
    return MoveFileExW( file_a.c_str(), file_b.c_str(),
                        MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING );
}

bool FSLib::createDirectoryFull( const string &path ) {
    uint64_t pos = path.find_first_of( L":", 0 ) + 2;
    if ( pos == string::npos )
        pos = 0;
    // go through all directories leading to the last one
    // and create them if they don't exist
    do {
        // get partial directory path
        pos = path.find_first_of( L"\\", pos );
        auto dirname = path.substr( 0, pos );
        // create it if it doesn't exist
        if ( !FSLib::exists( dirname ) ) {
            if ( !CreateDirectoryW( dirname.c_str(), NULL ) )
                return false;
        }
        pos++;
    } while ( pos < path.length() && pos != 0 );
    return true;
}

FSLib::Directory::iterator FSLib::Directory::end() {
    return Iterator( true );
}

FSLib::Directory::const_iterator FSLib::Directory::end() const {
    return Iterator( true );
}

char_t const *FSLib::Directory::Iterator::operator*() const {
    return data.cFileName;
}

FSLib::Directory::Iterator &FSLib::Directory::Iterator::operator++() {
    if ( ended == true )
        return *this;
    // skip . and ..
    if ( FindNextFileW( hFind, &data ) == 0 ) {
        ended = true;
    } else if ( !wcscmp( data.cFileName, L"." ) ||
                !wcscmp( data.cFileName, L".." ) ) {
        return operator++();
    }
    return *this;
}

bool FSLib::Directory::Iterator::operator==( const Iterator &i_other ) const {
    return i_other.ended == ended;
}

string FSLib::getContainingDirectory( const string &path ) {
    auto pos = path.find_last_of('\\');
    if(pos == string::npos) {
        return ".";
    }
    return path.substr(0, pos);
}

string FSLib::getFileName( const string &path ) {
    auto pos = path.find_last_of('\\');
    if(pos == string::npos) {
        return path;
    }
    return path.substr(pos+1);
}

string FSLib::getFileExtension( const string &path ) {
    auto pos = path.find_last_of('.');
    if(pos == string::npos) {
        return "";
    }
    return path.substr(pos + 1);
}
