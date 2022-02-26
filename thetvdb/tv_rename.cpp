#include <algorithm>
#include <iostream>
#include <map>

#include "../filesystem/filesystem.hpp"
#include "functions.hpp"
#include "../library.hpp"

#ifdef _WIN32

#include "rapidjson/document.h"

#else

#include <rapidjson/document.h>

#endif

#include <sstream>
#include <vector>

#ifdef _WIN32

#include <windows.h>

#include <codecvt>
#include <fcntl.h>
#include <io.h>
#include <locale>

constexpr const char_t *_tv_rename_dir_divider = L"\\";

#define toString( a ) utf8_to_wstring( a )

#else

constexpr const char_t *_tv_rename_dir_divider = "/";

#define toString( a ) a

#endif

// TV flags
#define TV_CHDIR 0x0100
#define TV_TRUST 0x0200
#define TV_LINUX 0x0400
#define TV_DVD 0x0800

string _tv_rename_api_token{};
Request _tv_rename_request;

bool authenticate( const std::string &api_key ) {
    Request &request = _tv_rename_request;
#ifdef _WIN32
    request.setServer( TEXT( "api.thetvdb.com" ) );
#else
    request.setServer( "https://api.thetvdb.com" );
#endif
    request.addHeader( TEXT( "Accept: application/json" ) );
    request.addHeader( TEXT( "Content-Type: application/json" ) );
    rapidjson::Document json;
    json.Parse(
        request.post( TEXT( "/login" ), "{ \"apikey\": \"" + api_key + "\" }" )
            .c_str() );
    if ( json.HasParseError() )
        return false;

    _tv_rename_api_token = toString( json["token"].GetString() );
    request.clearHeader();
    // TODO check return code
    return true;
}

bool init(const string &config_path) {
    return authenticate( "42B66F5E-C6BF-423F-ADF9-CC97163472F6" );
}

std::vector< std::pair< string, string > >
searchShow( const string &show, const string &language ) {
    Request &request = _tv_rename_request;
    request.addHeader( TEXT( "Accept: application/json" ) );
    request.addHeader( TEXT( "Authorization: Bearer " ) +
                       _tv_rename_api_token );
    request.addHeader( TEXT( "Accept-Language: " ) + language );

    auto encoded_show = encodeUrl( show );

    rapidjson::Document json;
    json.Parse(
        request.get( TEXT( "/search/series?name=" ) + encoded_show ).c_str() );
    if ( json.HasParseError() )
        return {};

    const rapidjson::Value &results = json["data"];
    if ( !results.IsArray() ) {
        return {};
    }

    std::vector< std::pair< string, string > > ret;

    // find all possible shows
    for ( size_t i = 0; i < results.Size(); i++ ) {
        auto show = toString( results[i]["seriesName"].GetString() );
        auto id = toString( std::to_string( results[i]["id"].GetInt() ) );
        ret.emplace_back( show, id );
    }
    request.clearHeader();
    return ret;
}

// get names for all episodes for a given season
std::vector< string > getEpisodeNames( const string &id, const string &season,
                                       const string &language,
                                       bool dvd = false ) {
    Request &request = _tv_rename_request;
    request.addHeader( TEXT( "Accept: application/json" ) );
    request.addHeader( TEXT( "Authorization: Bearer " ) +
                       _tv_rename_api_token );
    request.addHeader( TEXT( "Accept-Language: " ) + language );

    string baseurl = TEXT( "/series/" ) + id + TEXT( "/episodes/query?" );
    if ( dvd )
        baseurl += TEXT( "dvdSeason=" );
    else
        baseurl += TEXT( "airedSeason=" );
    baseurl += season;
    baseurl += TEXT( "&page=" );
    string page = TEXT( "1" );

    std::vector< string > episodes;

    do {
        episodes.resize( episodes.size() * 2 );
        rapidjson::Document json;
        json.Parse( request.get( baseurl + page ).c_str() );
        if ( json.HasParseError() )
            return {};
        if ( json.FindMember( "data" ) == json.MemberEnd() )
            break;

        if ( json["data"].IsArray() ) {
            rapidjson::Value &epdata = json["data"];
            if ( episodes.size() < epdata.Size() )
                episodes.resize( epdata.Size() );

            for ( size_t i = 0; i < epdata.Size(); i++ ) {
                if ( epdata[i]["episodeName"].IsString() ) {
                    size_t index = -1;
                    if ( dvd )
                        index = epdata[i]["dvdEpisodeNumber"].GetInt();
                    else
                        index = epdata[i]["airedEpisodeNumber"].GetInt();

                    if ( index == static_cast< size_t >( -1 ) )
                        continue;

                    if ( index > episodes.size() )
                        episodes.resize( index );
                    index--;
                    episodes[index] =
                        toString( epdata[i]["episodeName"].GetString() );
                    // some eps have whitespace at the end
                    while ( isspace( episodes[index].back() ) )
                        episodes[index].pop_back();
                }
            }
        }
        if ( json["links"]["next"].IsNull() )
            break;
        page = toString( std::to_string( json["links"]["next"].GetInt() ) );
    } while ( 1 );
    request.clearHeader();
    return episodes;
}

std::vector< std::tuple< int, string, string, string > >
getRenamedFiles( const string &show, int season, const string &id,
                 const string &language, const string &pattern,
                 const bool &unix_names, const std::map< int, string > &files,
                 bool dvd ) {
    auto season_num = toString( std::to_string( season ) );
    auto episodes = getEpisodeNames( id, season_num, language, dvd );

    if ( episodes.empty() )
        return {};

    if ( files.empty() )
        return {};

    // vector of pairs <ep_num,dir>,<original_name,new_name>
    std::vector< std::tuple< int, string, string, string > > renamed_files;

    for ( const auto &x : files ) {
        auto last = x.second.find_last_of( _tv_rename_dir_divider );
        string og_name;
        string dir;
        if ( last == string::npos ) {
            og_name = x.second;
            dir = TEXT( "." );
        } else {
            og_name = x.second.substr( last + 1 );
            dir = x.second.substr( 0, last );
        }
        unsigned long ep_num = x.first - 1;

        if ( ep_num < episodes.size() ) {
            auto pos = og_name.find_last_of( TEXT( "." ) );
            string og_name_without_extension{};
            string og_name_extension{};

            if ( pos == string::npos ) {
                og_name_without_extension = og_name;
            } else {
                og_name_without_extension = og_name.substr( 0, pos );
                og_name_extension = og_name.substr( pos );
            }

            // get desired filename
            auto name = compilePattern( pattern, season, x.first,
                                        og_name_without_extension,
                                        episodes[ep_num], show ) +
                        og_name_extension;
            // replace '/' with '|'
            for ( size_t i = 0; i < name.size(); i++ ) {
                if ( name[i] == '/' ) {
                    name[i] = '|';
                }
            }
            // replace characters illegal in windows if desired
            if ( !unix_names ) {
                name.erase( std::remove_if( name.begin(), name.end(),
                                            []( char_t x ) {
                                                return x == '?' || x == '"' ||
                                                       x == '\\' || x == '*';
                                            } ),
                            name.end() );
                size_t max{ name.size() };
                for ( size_t i = 0; i < max; i++ ) {
                    if ( name[i] == '|' ) {
                        name[i] = '-';
                    } else if ( name[i] == '<' ) {
                        name.erase( i, 1 );
                        name.insert( i, TEXT( "＜" ) );
                        max = name.size();
                    } else if ( name[i] == '>' ) {
                        name.erase( i, 1 );
                        name.insert( i, TEXT( "＞" ) );
                        max = name.size();
                    } else if ( name[i] == ':' ) {
                        name[i] = ' ';
                        name.insert( i + 1, TEXT( "- " ) );
                        max = name.size();
                    }
                }
                for ( size_t i = 0; i < max; i++ ) {
                    if ( name[i] == ' ' && name[i + 1] == ' ' ) {
                        name.erase( i, 1 );
                        max--;
                        i--;
                    }
                }
            }
            renamed_files.emplace_back( x.first, dir, og_name, name );
        }
    }
    return renamed_files;
}

string showFromId( const string &id ) {
    Request &request = _tv_rename_request;
    request.addHeader( TEXT( "Accept: application/json" ) );
    request.addHeader( TEXT( "Authorization: Bearer " ) +
                       _tv_rename_api_token );

    rapidjson::Document json;
    json.Parse( request.get( TEXT( "/series/" ) + id ).c_str() );
    if ( json.HasParseError() ) {
        return "";
    }
    return json["data"]["seriesName"].GetString();
}

void singleSeason( const string &path, const string &show, int season,
                   string id, const string &language, const string &pattern,
                   const bool &unix_names, const bool &trust,
                   std::map< int, string > *files_ptr, bool dvd ) {

    std::map< int, std::map< int, string > > *found_files = nullptr;

    if ( files_ptr == nullptr ) {
        found_files = new std::map< int, std::map< int, string > >;
        iterateFS( *found_files, path );
        if ( found_files->find( season ) != found_files->end() )
            files_ptr = &( *found_files )[season];
    }

    if ( files_ptr == nullptr ) {
        return;
    }

    auto renamed_files = getRenamedFiles( show, season, id, language, pattern,
                                          unix_names, *files_ptr, dvd );

    if ( renamed_files.empty() )
        goto end;

    for ( const auto &renamed : renamed_files ) {
        FSLib::rename( std::get< 1 >( renamed ) + _tv_rename_dir_divider +
                           std::get< 2 >( renamed ),
                       std::get< 1 >( renamed ) + _tv_rename_dir_divider +
                           std::get< 3 >( renamed ) );
        if ( found_files == nullptr ) {
            files_ptr[0][std::get< 0 >( renamed )] = std::get< 1 >( renamed ) +
                                                     _tv_rename_dir_divider +
                                                     std::get< 3 >( renamed );
        }
    }

end:
    if ( found_files != nullptr ) {
        delete found_files;
    }
}

void allSeasons( const string &id, const string &path, const string &show,
                 const string &language, const string &pattern,
                 const size_t &flags ) {
    std::map< int, std::map< int, string > > seasons;
    // get all season number from this directory and subdirectories
    iterateFS( seasons, path );
    for ( auto &x : seasons ) {
        singleSeason( path, show, x.first, id, language, pattern,
                      flags & TV_LINUX, flags & TV_TRUST, &x.second,
                      flags & TV_DVD );
    }
}

RenameObject showToRenameObject( const std::pair< string, string > &show,
                                 const std::string &language ) {
    RenameObject result;
    result.setPresentedName( show.first );
    result.addCustomField( "id", show.second );
    result.addCustomField( "language", language );
    result.addCustomField( "order", "aired" );
    result.addCustomField( "pattern", getDefaultPattern() );
    return result;
}

std::vector< RenameObject > getOptions( const RenameObject &search ) {
    string lang = "en";
    if ( search.getCustomFields().find( "language" ) !=
         search.getCustomFields().end() ) {
        lang = search.getCustomFields().at( "language" );
    }
    string name = search.getPresentedName();
    auto possibilities = searchShow( name, lang );
    std::vector< RenameObject > result{};
    for ( auto &show : possibilities ) {
        result.push_back( showToRenameObject( show, lang ) );
    }
    return result;
}

bool renamePath( const string &path, const RenameObject &renamer ) {
    string id = "";
    string lang = "en";
    string pattern = getDefaultPattern();
    string show = "";
    size_t flags = TV_TRUST;

    if ( renamer.getCustomFields().find( "language" ) !=
         renamer.getCustomFields().end() ) {
        lang = renamer.getCustomFields().at( "language" );
    }

    if ( renamer.getCustomFields().find( "id" ) ==
             renamer.getCustomFields().end() ||
         renamer.getCustomFields().at( "id" ).empty() ) {
        auto results = searchShow( renamer.getPresentedName(), lang );
        if ( results.empty() )
            return false;
        id = results[0].second;
        show = results[0].first;
    } else if( renamer.getCustomFields().find("id") != renamer.getCustomFields().end()) {
        id = renamer.getCustomFields().at( "id" );
        show = showFromId( id );
    }

    if ( renamer.getCustomFields().find( "pattern" ) !=
         renamer.getCustomFields().end() ) {
        pattern = renamer.getCustomFields().at( "pattern" );
    }

    if ( renamer.getCustomFields().find( "order" ) !=
             renamer.getCustomFields().end() &&
         renamer.getCustomFields().at( "order" ) == "dvd" ) {
        flags |= TV_DVD;
    }

    allSeasons( id, path, show, lang, pattern, flags );
    return true;
}

std::vector< string > getCustomKeys() {
    return { "id", "language", "pattern", "order" };
}

const string getName() {
    return "thetvdb";
}
