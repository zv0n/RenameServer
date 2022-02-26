#include <iomanip>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "../filesystem/filesystem.hpp"
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

// return true if file contains S[0-9]+E[0-9]+, set
// season_pos to start of season number and ep_pos to start of episode number
bool searchSeason( const char_t *const path, size_t &season_pos,
                   size_t &ep_pos ) {
    size_t cur_pos{};
    while ( path[cur_pos] != '\0' ) {
        if ( ( path[cur_pos] == 's' || path[cur_pos] == 'S' ) &&
             iswdigit( path[cur_pos + 1] ) ) {
            cur_pos++;
            if ( path[cur_pos] == '\0' )
                return false;
            season_pos = cur_pos; // after ++ because we want the first pos to
                                  // point to season's number
            while ( iswdigit( path[cur_pos] ) )
                cur_pos++;
            if ( path[cur_pos] == '\0' )
                return false;
            if ( ( path[cur_pos] == 'e' || path[cur_pos] == 'E' ) &&
                 iswdigit( path[cur_pos + 1] ) ) {
                ep_pos = cur_pos + 1;
                return true;
            }
        }
        cur_pos++;
    }
    return false;
}

void iterateFS( std::map< int, std::map< int, string > > &seasons,
                const string &path ) {
    // season_pos - position of first digit of the season number
    // ep_pos - position of first digit of the episode number
    size_t season_pos{ string::npos };
    size_t ep_pos{ string::npos };
    if ( FSLib::isDirectory( path ) ) {
        for ( const auto p : FSLib::Directory( path ) ) {
            // if p is directory, iterate through it
            if ( FSLib::isDirectory( path + dir_divider + p ) ) {
                iterateFS( seasons, path + dir_divider + p );
                continue;
            }

            // if file is a correct format, add it to file list
            // for its season
            if ( searchSeason( p, season_pos, ep_pos ) )
                seasons[std::stoi( p + season_pos )][std::stoi( p + ep_pos )] =
                    path + dir_divider + p;
        }
    } else if ( searchSeason( path.c_str(), season_pos, ep_pos ) ) {
        seasons[std::stoi( path.c_str() + season_pos )][std::stoi( path.c_str() + ep_pos )] = path;
    }
}

// create file name based on given pattern
string compilePattern( const string &pattern, int season, int episode,
                       const string &filename, const string &episodeName,
                       const string &showName ) {
    string output;

#ifdef _WIN32
    auto season_num = std::to_wstring( season );
    auto ep_num = std::to_wstring( episode );
#else
    auto season_num = std::to_string( season );
    auto ep_num = std::to_string( episode );
#endif

    for ( size_t i = 0; i < pattern.size(); i++ ) {
        // if current character is % check if a pattern follows, otherwise
        // put %
        if ( pattern[i] == '%' ) {
            // check for numbers right after % indicating size of zero
            // padding for numbers
            auto pos = pattern.find_first_not_of( TEXT( "0123456789" ), i + 1 );

            if ( pattern.find( TEXT( "season" ), pos - 1 ) == pos &&
                 pos != i + 1 ) {
                // if season is AFTER numbers, put season number padded
                // with zeros

                // get number of leading zeros
                auto leading = std::stoi( pattern.c_str() + i + 1 );
                // move i to the last char of 'season'
                i = pos + 5;

                // get number of zeros to be put before the season number
                leading -= season_num.size();
                if ( leading < 0 )
                    leading = 0;

                // add padded season to output
                output += string( leading, '0' ) + season_num;
            } else if ( pattern.find( TEXT( "season" ), i ) == i + 1 ) {
                // if season isn't after numbers, just put season number to
                // output
                i += 6;
                output += season_num;
            } else if ( pattern.find( TEXT( "episode" ), pos - 1 ) == pos &&
                        pos != i + 1 ) {
                // same principle as with season after number
                auto leading = std::stoi( pattern.c_str() + i + 1 );

                i = pos + 6;

                leading -= ep_num.size();
                if ( leading < 0 )
                    leading = 0;

                output += string( leading, '0' ) + ep_num;
            } else if ( pattern.find( TEXT( "episode" ), i ) == i + 1 ) {
                // if episode isn't after number, just put the episode number to
                // output
                i += 7;
                output += ep_num;
            } else if ( pattern.find( TEXT( "epname" ), i ) == i + 1 ) {
                // episode name from thetvdb
                i += 6;
                output += episodeName;
            } else if ( pattern.find( TEXT( "show" ), i ) == i + 1 ) {
                // show name from thetvdb
                i += 4;
                output += showName;
            } else if ( pattern.find( TEXT( "filename" ), i ) == i + 1 ) {
                // original file name
                i += 8;
                output += filename;
            } else {
                // output % if no escape sequence was found
                output += '%';
            }
        } else if ( pattern[i] == '\\' ) {
            // possibility to escape %
            if ( pattern[i + 1] == '%' ) {
                output += '%';
                i++;
            } else if ( pattern[i + 1] == '\\' ) {
                output += '\\';
                i++;
            } else {
                output += '\\';
            }
        } else {
            // if char isn't % or / just add it to the output string
            output += pattern[i];
        }
    }

    return output;
}

/* change names of original files to generated new names
 * orig - original filenames
 * renamed - renamed filenames (sorted in the same order as `orig`)
 */
void renameFiles( const std::vector< std::tuple< int, string, string, string > >
                      &renamed_files ) {
    for ( const auto &renamed : renamed_files ) {
        FSLib::rename(
            std::get< 1 >( renamed ) + dir_divider + std::get< 2 >( renamed ),
            std::get< 1 >( renamed ) + dir_divider + std::get< 3 >( renamed ) );
    }
}

// TODO read config
const string getDefaultPattern() {
    return "S%2seasonE%2episode - %epname";
}
