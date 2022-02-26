#include <iomanip>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "functions.hpp"

#ifdef _WIN32

#include <codecvt>
#include <shlobj.h>

constexpr const char_t *dir_divider = L"\\";

#else // UNIX

#include <pwd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

constexpr const char_t *dir_divider = "/";

#endif // UNIX

#ifdef __APPLE__
void error( int status, int i_errno, const char *fmt, ... ) {
    fprintf( stderr, "%s - ", strerror( i_errno ) );
    va_list args;
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fputc( '\n', stderr );
    exit( status );
}
#endif

#ifdef _WIN32

// functions to convert between string and wstring

std::string wstring_to_utf8( const std::wstring &wstring ) {
    std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > wconv;
    return wconv.to_bytes( wstring );
}

std::wstring utf8_to_wstring( const std::string &utf8 ) {
    std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > wconv;
    return wconv.from_bytes( utf8 );
}

#endif // _WIN32

// encode url so it's valid even with UTF-8 characters
string encodeUrl( const string &url ) {
    // stolen from here -
    // https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
#ifdef _WIN32
    std::wstringstream encoded;
    auto url_c = wstring_to_utf8( url );
#else
    std::stringstream encoded;
    const auto &url_c = url;
#endif
    encoded.fill( '0' );
    encoded << std::hex;
    for ( const auto &x : url_c ) {
        if ( isalnum( static_cast< unsigned char >( x ) ) || x == '-' ||
             x == '_' || x == '.' || x == '~' || x == '+' ) {
            encoded << x;
            continue;
        }
        encoded << std::uppercase << '%' << std::setw( 2 );
        encoded << int( static_cast< unsigned char >( x ) ) << std::nouppercase;
    }
    return encoded.str();
}

