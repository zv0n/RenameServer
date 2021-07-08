#ifndef TV_FUNCTIONS_H
#define TV_FUNCTIONS_H

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../network/network.hpp"

#ifdef _WIN32

using string = std::wstring;
using char_t = wchar_t;

std::wstring utf8_to_wstring( const std::string &utf8 );

#else

using string = std::string;
using char_t = char;

#define TEXT( x ) x

#endif

#ifdef __APPLE__
void error( int status, int i_errno, const char *fmt, ... );
#endif

// CLI functions
const string getDefaultPattern();

string encodeUrl( const string &url );

void iterateFS( std::map< int, std::map< int, string > > &seasons,
                const string &path );

string compilePattern( const string &pattern, int season, int episode,
                       const string &filename, const string &episodeName,
                       const string &showName );
#endif
