// API - 2ebc8e784a4072da457fae5c0d291e48
// API READ ONLY -
// eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiIyZWJjOGU3ODRhNDA3MmRhNDU3ZmFlNWMwZDI5MWU0OCIsInN1YiI6IjYwZTJlNGI5MjJlNDgwMDA2MDJmZDMzMyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.c0y7bTCI5KSsfQRw7igPx1FR40mbMF6hGTJTHn0HXH8
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

const std::vector< std::pair< std::string, std::string > > languages = {
    { "aa", "Afar" },
    { "ab", "Abkhazian" },
    { "ae", "Avestan" },
    { "af", "Afrikaans" },
    { "ak", "Akan" },
    { "am", "Amharic" },
    { "an", "Aragonese" },
    { "ar", "العربية" },
    { "as", "Assamese" },
    { "av", "Avaric" },
    { "ay", "Aymara" },
    { "az", "Azərbaycan" },
    { "ba", "Bashkir" },
    { "be", "беларуская мова" },
    { "bg", "български език" },
    { "bi", "Bislama" },
    { "bm", "Bamanankan" },
    { "bn", "বাংলা" },
    { "bo", "Tibetan" },
    { "br", "Breton" },
    { "bs", "Bosanski" },
    { "ca", "Català" },
    { "ce", "Chechen" },
    { "ch", "Finu' Chamorro" },
    { "cn", "广州话 / 廣州話" },
    { "co", "Corsican" },
    { "cr", "Cree" },
    { "cs", "Český" },
    { "cu", "Slavic" },
    { "cv", "Chuvash" },
    { "cy", "Cymraeg" },
    { "da", "Dansk" },
    { "de", "Deutsch" },
    { "dv", "Divehi" },
    { "dz", "Dzongkha" },
    { "ee", "Èʋegbe" },
    { "el", "ελληνικά" },
    { "en", "English" },
    { "eo", "Esperanto" },
    { "es", "Español" },
    { "et", "Eesti" },
    { "eu", "euskera" },
    { "fa", "فارسی" },
    { "ff", "Fulfulde" },
    { "fi", "suomi" },
    { "fj", "Fijian" },
    { "fo", "Faroese" },
    { "fr", "Français" },
    { "fy", "Frisian" },
    { "ga", "Gaeilge" },
    { "gd", "Gaelic" },
    { "gl", "Galego" },
    { "gn", "Guarani" },
    { "gu", "Gujarati" },
    { "gv", "Manx" },
    { "ha", "Hausa" },
    { "he", "עִבְרִית" },
    { "hi", "हिन्दी" },
    { "ho", "Hiri Motu" },
    { "hr", "Hrvatski" },
    { "ht", "Haitian; Haitian Creole" },
    { "hu", "Magyar" },
    { "hy", "Armenian" },
    { "hz", "Herero" },
    { "ia", "Interlingua" },
    { "id", "Bahasa indonesia" },
    { "ie", "Interlingue" },
    { "ig", "Igbo" },
    { "ii", "Yi" },
    { "ik", "Inupiaq" },
    { "io", "Ido" },
    { "is", "Íslenska" },
    { "it", "Italiano" },
    { "iu", "Inuktitut" },
    { "ja", "日本語" },
    { "jv", "Javanese" },
    { "ka", "ქართული" },
    { "kg", "Kongo" },
    { "ki", "Kikuyu" },
    { "kj", "Kuanyama" },
    { "kk", "қазақ" },
    { "kl", "Kalaallisut" },
    { "km", "Khmer" },
    { "kn", "Kannada" },
    { "ko", "한국어/조선말" },
    { "kr", "Kanuri" },
    { "ks", "Kashmiri" },
    { "ku", "Kurdish" },
    { "kv", "Komi" },
    { "kw", "Cornish" },
    { "ky", "Kirghiz" },
    { "la", "Latin" },
    { "lb", "Letzeburgesch" },
    { "lg", "Ganda" },
    { "li", "Limburgish" },
    { "ln", "Lingala" },
    { "lo", "Lao" },
    { "lt", "Lietuvių" },
    { "lu", "Luba-Katanga" },
    { "lv", "Latviešu" },
    { "mg", "Malagasy" },
    { "mh", "Marshall" },
    { "mi", "Maori" },
    { "mk", "Macedonian" },
    { "ml", "Malayalam" },
    { "mn", "Mongolian" },
    { "mo", "Moldavian" },
    { "mr", "Marathi" },
    { "ms", "Bahasa melayu" },
    { "mt", "Malti" },
    { "my", "Burmese" },
    { "na", "Nauru" },
    { "nb", "Bokmål" },
    { "nd", "Ndebele" },
    { "ne", "Nepali" },
    { "ng", "Ndonga" },
    { "nl", "Nederlands" },
    { "nn", "Norwegian Nynorsk" },
    { "no", "Norsk" },
    { "nr", "Ndebele" },
    { "nv", "Navajo" },
    { "ny", "Chichewa; Nyanja" },
    { "oc", "Occitan" },
    { "oj", "Ojibwa" },
    { "om", "Oromo" },
    { "or", "Oriya" },
    { "os", "Ossetian; Ossetic" },
    { "pa", "ਪੰਜਾਬੀ" },
    { "pi", "Pali" },
    { "pl", "Polski" },
    { "ps", "پښتو" },
    { "pt", "Português" },
    { "qu", "Quechua" },
    { "rm", "Raeto-Romance" },
    { "rn", "Kirundi" },
    { "ro", "Română" },
    { "ru", "Pусский" },
    { "rw", "Kinyarwanda" },
    { "sa", "Sanskrit" },
    { "sc", "Sardinian" },
    { "sd", "Sindhi" },
    { "se", "Northern Sami" },
    { "sg", "Sango" },
    { "sh", "Serbo-Croatian" },
    { "si", "සිංහල" },
    { "sk", "Slovenčina" },
    { "sl", "Slovenščina" },
    { "sm", "Samoan" },
    { "sn", "Shona" },
    { "so", "Somali" },
    { "sq", "shqip" },
    { "sr", "Srpski" },
    { "ss", "Swati" },
    { "st", "Sotho" },
    { "su", "Sundanese" },
    { "sv", "svenska" },
    { "sw", "Kiswahili" },
    { "ta", "தமிழ்" },
    { "te", "తెలుగు" },
    { "tg", "Tajik" },
    { "th", "ภาษาไทย" },
    { "ti", "Tigrinya" },
    { "tk", "Turkmen" },
    { "tl", "Tagalog" },
    { "tn", "Tswana" },
    { "to", "Tonga" },
    { "tr", "Türkçe" },
    { "ts", "Tsonga" },
    { "tt", "Tatar" },
    { "tw", "Twi" },
    { "ty", "Tahitian" },
    { "ug", "Uighur" },
    { "uk", "Український" },
    { "ur", "اردو" },
    { "uz", "ozbek" },
    { "ve", "Venda" },
    { "vi", "Tiếng Việt" },
    { "vo", "Volapük" },
    { "wa", "Walloon" },
    { "wo", "Wolof" },
    { "xh", "Xhosa" },
    { "xx", "No Language" },
    { "yi", "Yiddish" },
    { "yo", "Èdè Yorùbá" },
    { "za", "Zhuang" },
    { "zh", "普通话" },
    { "zu", "Zulu" }
};

string _moviedb_api_token{};
Request _moviedb_request;

struct MovieNames {
    string name;
    string original_name;
};

struct MovieInfo {
    string name;
    string original_name;
    string year;
    string id;
};

bool init( const string &config_path ) {
    Request &request = _moviedb_request;
#ifdef _WIN32
    request.setServer( TEXT( "api.themoviedb.org/3" ) );
#else
    request.setServer( "https://api.themoviedb.org/3" );
#endif
    _moviedb_api_token =
        "eyJhbGciOiJIUzI1NiJ9."
        "eyJhdWQiOiIyZWJjOGU3ODRhNDA3MmRhNDU3ZmFlNWMwZDI5MWU0OCIsInN1YiI6IjYwZT"
        "JlNGI5MjJlNDgwMDA2MDJmZDMzMyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9u"
        "IjoxfQ.c0y7bTCI5KSsfQRw7igPx1FR40mbMF6hGTJTHn0HXH8";
    return true;
}

bool hasKey( const rapidjson::GenericValue< rapidjson::UTF8<> > &object,
             const std::string &key ) {
    return object.FindMember( key.c_str() ) != object.MemberEnd();
}

std::vector< MovieInfo >
searchMovie( const string &movie, const string &language, const string &year ) {
    Request &request = _moviedb_request;
    request.addHeader( TEXT( "Accept: application/json" ) );
    request.addHeader( TEXT( "Authorization: Bearer " ) + _moviedb_api_token );

    auto encoded_show = encodeUrl( movie );
    int pages = 0;
    int cur_page = 0;

    std::vector< MovieInfo > ret;

    do {
        cur_page++;
        rapidjson::Document json;
        auto request_uri = TEXT( "/search/movie?query=" ) + encoded_show +
                           TEXT( "&language=" ) + language + TEXT( "&page=" ) +
                           toString( std::to_string( cur_page ) );
        if ( !year.empty() ) {
            request_uri += TEXT( "&year=" ) + year;
        }
        json.Parse( request.get( request_uri ).c_str() );
        if ( json.HasParseError() )
            return {};

        pages = json["total_pages"].GetInt();

        const rapidjson::Value &results = json["results"];
        if ( !results.IsArray() ) {
            return {};
        }

        // find all possible movies
        for ( size_t i = 0; i < results.Size(); i++ ) {
            if ( !hasKey( results[i], "title" ) ||
                 !hasKey( results[i], "id" ) ||
                 !hasKey( results[i], "release_date" ) ||
                 !hasKey( results[i], "original_title" ) ) {
                continue;
            }
            auto movie = toString( results[i]["title"].GetString() );
            auto id = toString( std::to_string( results[i]["id"].GetInt() ) );
            string year = toString( results[i]["release_date"].GetString() );
            string original =
                toString( results[i]["original_title"].GetString() );
            if ( year.empty() ) {
                year = "0000";
            } else {
                year = year.substr( 0, year.find( '-' ) );
            }
            MovieInfo tmp;
            tmp.name = std::move( movie );
            tmp.id = std::move( id );
            tmp.original_name = std::move( original );
            tmp.year = std::move( year );
            ret.push_back( std::move( tmp ) );
        }
    } while ( cur_page < pages && cur_page < 5 );
    request.clearHeader();
    return ret;
}

RenameObject movieToRenameObject( const MovieInfo &movie,
                                  const std::string &language ) {
    RenameObject result;
    result.setPresentedName( movie.name );
    result.addCustomField( "id", movie.id );
    result.addCustomField( "language", language );
    result.addCustomField( "year", movie.year );
    result.addCustomField( "original_title", movie.original_name );
    result.addCustomField( "use_original", "false" );
    return result;
}

std::vector< RenameObject > getOptions( const RenameObject &search ) {
    string lang = "en-US";
    string year = "";
    if ( search.getCustomFields().find( "language" ) !=
         search.getCustomFields().end() ) {
        lang = search.getCustomFields().at( "language" );
    }
    if ( search.getCustomFields().find( "year" ) !=
         search.getCustomFields().end() ) {
        year = search.getCustomFields().at( "year" );
    }
    string name = search.getPresentedName();
    auto possibilities = searchMovie( name, lang, year );
    std::vector< RenameObject > result{};
    for ( auto &movie : possibilities ) {
        result.push_back( movieToRenameObject( movie, lang ) );
    }
    return result;
}

std::string removeIllegalCharacters( const std::string &input ) {
    // replace '/' with '|'
    std::string ret = input;
    for ( size_t i = 0; i < ret.size(); i++ ) {
        if ( ret[i] == '/' ) {
            ret[i] = '|';
        }
    }
    // replace characters illegal in windows
    ret.erase( std::remove_if( ret.begin(), ret.end(),
                               []( char_t x ) {
                                   return x == '?' || x == '"' || x == '\\' ||
                                          x == '*';
                               } ),
               ret.end() );
    for ( size_t i = 0; i < ret.size(); i++ ) {
        if ( ret[i] == '|' ) {
            ret[i] = '-';
        } else if ( ret[i] == '<' ) {
            ret.erase( i, 1 );
            ret.insert( i, TEXT( "＜" ) );
        } else if ( ret[i] == '>' ) {
            ret.erase( i, 1 );
            ret.insert( i, TEXT( "＞" ) );
        } else if ( ret[i] == ':' ) {
            ret[i] = ' ';
            ret.insert( i + 1, TEXT( "- " ) );
        }
    }
    for ( size_t i = 0; i < ret.size(); i++ ) {
        if ( ret[i] == ' ' && ret[i + 1] == ' ' ) {
            ret.erase( i, 1 );
            i--;
        }
    }
    return ret;
}

MovieNames movieFromId( const string &id, const string &language ) {
    string uri = "/movie/" + id + "?language=" + language;

    Request &request = _moviedb_request;
    request.addHeader( TEXT( "Accept: application/json" ) );
    request.addHeader( TEXT( "Authorization: Bearer " ) + _moviedb_api_token );
    rapidjson::Document json;
    json.Parse( request.get( uri ).c_str() );
    if ( json.HasParseError() ) {
        return { "", "" };
    }
    return { removeIllegalCharacters( json["title"].GetString() ),
             removeIllegalCharacters( json["original_title"].GetString() ) };
}

bool renameMovie( const string &path, const string &name, const string &year ) {
    return FSLib::rename(
        path, FSLib::canonical( FSLib::getContainingDirectory( path ) ) + "/" +
                  name + " (" + year + ")." + FSLib::getFileExtension( path ) );
}

bool renamePath( const string &path, const RenameObject &renamer ) {
    string id = "";
    string lang = "en-US";
    MovieNames movie = { "", "" };
    string year = "";
    bool use_original = false;

    if ( renamer.getCustomFields().find( "language" ) !=
         renamer.getCustomFields().end() ) {
        lang = renamer.getCustomFields().at( "language" );
    }

    if ( renamer.getCustomFields().find( "year" ) !=
         renamer.getCustomFields().end() ) {
        year = renamer.getCustomFields().at( "year" );
    }

    if ( renamer.getCustomFields().find( "use_original" ) !=
         renamer.getCustomFields().end() ) {
        auto use = renamer.getCustomFields().at( "use_original" );
        use_original = ( use == "true" || use == "True" || use == "TRUE" );
    }

    if ( renamer.getCustomFields().find( "id" ) ==
             renamer.getCustomFields().end() ||
         renamer.getCustomFields().at( "id" ).empty() ) {
        auto results = searchMovie( renamer.getPresentedName(), lang, year );
        if ( results.empty() )
            return false;
        id = results[0].id;
        movie = { removeIllegalCharacters( results[0].name ),
                  removeIllegalCharacters( results[0].original_name ) };
        year = results[0].year;
    } else {
        id = renamer.getCustomFields().at( "id" );
        movie = movieFromId( id, lang );
    }

    if ( ( !use_original && movie.name == "" ) ||
         ( use_original && movie.original_name == "" ) )
        return false;

    return renameMovie( path, use_original ? movie.original_name : movie.name,
                        year );
}

std::vector< std::unordered_map< string, string > > getCustomKeys() {
    return { {
                 { "name", "id"},
                 { "display_name", "Movie ID" },
                 {"type", NUM_TYPE},
                 {"input", "true"}
             },
             {
                 { "name", "language" },
                 { "display_name", "Language to use" },
                 { "type", MULTICHOICE_TYPE },
                 { "input", "true" },
             },
             {
                 { "name", "year" },
                 { "display_name", "Year of release" },
                 { "type", YEAR_TYPE },
                 { "input", "true" },
             },
             {
                 { "name", "original_title" },
                 { "display_name", "" },
                 { "type", STRING_TYPE },
                 { "input", "false" },
             },
             {
                 { "name", "use_original" },
                 { "display_name", "Use the original language title" },
                 { "type", BOOL_TYPE },
                 { "input", "true" },
             },
           };
}

std::vector< std::pair< string, string > >
getCustomKeyOptions( const string &key ) {
    if ( key == "language" ) {
        return languages;
    }
    return {};
}

const string getCustomKeyDefault( const string &key ) {
    if ( key == "language" ) {
        return "en";
    }
    return "";
}

const string choiceDisplay() {
    return "\%name (\%original_title) (\%year)";
}

const string getName() {
    return "TheMovieDB";
}

const bool canRenameMultipleFiles() {
    return false;
}

const bool shouldPerformSearch() {
    return true;
}
