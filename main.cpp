#include "rename_object.hpp"
#include "functions.hpp"
#include <climits>
#include <iostream>
#include <memory>
#include <restbed>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <string>
#include <chrono>

#include "jwt.hpp"
#include "filesystem/filesystem.hpp"
#include "config/config.hpp"
#include "fileobject.hpp"

std::vector< RenameLibrary > libraries{};
Configuration cfg;

void sendResponse( const std::string &response, int status_code,
                   const std::shared_ptr< restbed::Session > &session ) {
    session->close( status_code, response,
                    { { "Content-Length", std::to_string( response.length() ) },
                      { "Access-Control-Allow-Origin", "*" } } );
}

void performPostFunc(
    const std::shared_ptr< restbed::Session > &session,
    const std::function< void(
        const std::shared_ptr< restbed::Session >,
        rapidjson::GenericDocument< rapidjson::UTF8<> > & ) > &callback ) {
    const auto request = session->get_request();

    int content_length = request->get_header( "Content-Length", 0 );

    session->fetch(
        content_length,
        [callback]( const std::shared_ptr< restbed::Session > &session,
                    const restbed::Bytes &body ) {
            rapidjson::Document doc;
            doc.Parse( reinterpret_cast< const char * >( body.data() ),
                       body.size() );
            if ( doc.HasParseError() ) {
                sendResponse( "ERROR: Invalid body!", 401, session );
                return;
            }
            callback( session, doc );
        } );
}

bool verifyLogin( const std::shared_ptr< restbed::Session > &session,
                  rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( doc.FindMember( "token" ) == doc.MemberEnd() ||
         !doc["token"].IsString() ) {
        sendResponse( "ERROR: Invalid token!", 401, session );
        return false;
    }
    const auto *token = doc["token"].GetString();
    auto res = verifyJWT( token );
    if ( !res ) {
        sendResponse( "ERROR: Invalid token!", 401, session );
    }
    return res;
}

std::vector< std::pair< RenameLibrary *, size_t > > getLibraries() {
    std::vector< std::pair< RenameLibrary *, size_t > > result{};
    for ( size_t i = 0; i < libraries.size(); i++ ) {
        result.emplace_back( &libraries[i], i );
    }
    return result;
}

std::string getLibrariesJson() {
    auto libraries = getLibraries();
    std::ostringstream result;
    result << "{\n  \"libraries\": [\n";
    if ( !libraries.empty() ) {
        for ( const auto &library : libraries ) {
            result << "  {\n    \"id\": " << library.second << ",\n";
            result << "    \"name\": \"" << safeJson( library.first->getName() )
                   << "\"\n,";
            result << "    \"multiple_files\": "
                   << ( library.first->canRenameMultipleFiles() ? "true"
                                                                : "false" )
                   << "\n  },\n";
        }
        result.seekp( -2, std::ios_base::end );
        result << "\n";
    }
    result << "  ]\n}";
    auto res = result.str();
    return res;
}

void getLibrariesRest( const std::shared_ptr< restbed::Session > &session ) {
    sendResponse( getLibrariesJson(), restbed::OK, session );
}

std::vector< RenameObject > getOptions( const RenameObject &search ) {
    auto library_id = search.getLibraryId();
    if ( library_id >= libraries.size() ) {
        return {};
    }
    auto result = libraries[library_id].getOptions( search );
    for ( auto &res : result ) {
        res.setLibraryId( library_id );
    }
    return result;
}

std::string getOptionsJson( const RenameObject &search ) {
    std::ostringstream res;
    res << "{\n  \"options\": [\n";
    auto options = getOptions( search );
    if ( !options.empty() ) {
        for ( auto &option : options ) {
            res << option.toJson();
            res << ",\n";
        }
        res.seekp( -2, std::ios_base::end );
        res << "\n";
    }
    res << "  ]\n}";
    auto result = res.str();
    return result;
}

void getOptionsRest( const std::shared_ptr< restbed::Session > &session,
                     rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( doc.HasParseError() ) {
        sendResponse( "ERROR: Invalid body!", 401, session );
        return;
    }
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    if ( doc.FindMember( "info" ) == doc.MemberEnd() ||
         !doc["info"].IsObject() ) {
        sendResponse( "ERROR: Invalid search!", 401, session );
        return;
    }
    RenameObject search;
    rapidjson::StringBuffer sb;
    rapidjson::Writer< rapidjson::StringBuffer > writer( sb );
    doc["info"].Accept( writer );
    std::string s = sb.GetString();
    search.fromJson( s );
    if ( search.getPresentedName().empty() ) {
        sendResponse( "Empty search", 401, session );
        return;
    }
    sendResponse( getOptionsJson( search ), 200, session );
}

std::vector< std::unordered_map< std::string, std::string > >
getCustomKeys( size_t library_id ) {
    if ( library_id >= libraries.size() ) {
        return {};
    }
    return libraries[library_id].getCustomKeys();
}

std::vector< std::pair< string, string > >
getFieldOptions( size_t library_id, const std::string &field ) {
    if ( library_id >= libraries.size() ) {
        return {};
    }
    return libraries[library_id].getCustomKeyOptions( field );
}

std::string getFieldDefault( size_t library_id, const std::string &field ) {
    if ( library_id >= libraries.size() ) {
        return "";
    }
    return libraries[library_id].getCustomKeyDefault( field );
}

std::string getCustomKeysJson( size_t library_id ) {
    std::ostringstream res;
    res << "{\n  \"custom_keys\": [\n";
    auto custom_keys = getCustomKeys( library_id );
    if ( !custom_keys.empty() ) {
        for ( auto &key : custom_keys ) {
            res << "    {\n";
            res << "      \"name\": \"" << safeJson( key["name"] ) << "\",\n";
            res << "      \"type\": \"" << safeJson( key["type"] ) << "\",\n";
            res << "      \"input\": " << safeJson( key["input"] ) << "\n";
            res << "    },\n";
        }
        res.seekp( -2, std::ios_base::end );
        res << "\n";
    }
    res << "  ]\n}";
    return res.str();
}

std::string getFieldOptionsJson( size_t library_id, const std::string &field ) {
    std::ostringstream res;
    res << "{\n  \"options\": [\n";
    auto options = getFieldOptions( library_id, field );
    if ( !options.empty() ) {
        for ( auto &option : options ) {
            res << "    {\n";
            // TODO replace first/second with a proper name, create option
            // struct or something
            res << "      \"code\": \"" << safeJson( option.first ) << "\",\n";
            res << "      \"name\": \"" << safeJson( option.second ) << "\"\n";
            res << "    },\n";
        }
        res.seekp( -2, std::ios_base::end );
        res << "\n";
    }
    res << "  ]\n}";
    return res.str();
}

std::string getFieldDefaultJson( size_t library_id, const std::string &field ) {
    std::ostringstream res;
    res << "{\n  \"default\": \"" << getFieldDefault( library_id, field )
        << "\"}";
    return res.str();
}

void getCustomKeysRest( const std::shared_ptr< restbed::Session > &session,
                        rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( doc.FindMember( "library_id" ) == doc.MemberEnd() ||
         !doc["library_id"].IsUint64() ) {
        sendResponse( "ERROR: Invalid library_id!", 401, session );
        return;
    }
    size_t library_id = doc["library_id"].GetUint64();
    sendResponse( getCustomKeysJson( library_id ), 200, session );
}

void getFieldOptionsRest(
    const std::shared_ptr< restbed::Session > &session,
    rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( doc.FindMember( "library_id" ) == doc.MemberEnd() ||
         !doc["library_id"].IsUint64() ) {
        sendResponse( "ERROR: Invalid library_id!", 401, session );
        return;
    }
    if ( doc.FindMember( "field" ) == doc.MemberEnd() ||
         !doc["field"].IsString() ) {
        sendResponse( "ERROR: Invalid field!", 401, session );
        return;
    }
    auto library_id = doc["library_id"].GetUint64();
    auto field = doc["field"].GetString();
    sendResponse( getFieldOptionsJson( library_id, field ), 200, session );
}

void getFieldDefaultRest(
    const std::shared_ptr< restbed::Session > &session,
    rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( doc.FindMember( "library_id" ) == doc.MemberEnd() ||
         !doc["library_id"].IsUint64() ) {
        sendResponse( "ERROR: Invalid library_id!", 401, session );
        return;
    }
    if ( doc.FindMember( "field" ) == doc.MemberEnd() ||
         !doc["field"].IsString() ) {
        sendResponse( "ERROR: Invalid field!", 401, session );
        return;
    }
    auto library_id = doc["library_id"].GetUint64();
    auto field = doc["field"].GetString();
    sendResponse( getFieldDefaultJson( library_id, field ), 200, session );
}

std::pair< bool, std::string > renamePath( std::string path,
                                           const RenameObject &renamer ) {
    if ( renamer.getLibraryId() >= libraries.size() ) {
        return { false, "Invalid library id" };
    }
    if ( path[0] != '/' ) {
        path = cfg.getSourcePath() + "/" + path;
    }

    auto canon_path = FSLib::canonical( path );
    if ( canon_path.substr( 0, cfg.getSourcePath().length() ) !=
         cfg.getSourcePath() ) {
        return { false, "Invalid path" };
    }

    if ( !FSLib::exists( path ) ) {
        return { false, "Source doesn't exist" };
    }
    return { libraries[renamer.getLibraryId()].renamePath( path, renamer ),
             "Library error" };
}

std::string renamePathJson( const std::string &path,
                            const RenameObject &renamer ) {
    std::ostringstream res;
    res << "{\n  \"success\": ";
    auto rename_result = renamePath( path, renamer );
    if ( rename_result.first ) {
        res << "true";
    } else {
        res << "false";
    }
    res << ",\n";
    if ( !rename_result.first ) {
        res << "  \"error\": \"" << safeJson( rename_result.second ) << "\"\n";
    }
    res << "}";
    return res.str();
}

void renamePathRest( const std::shared_ptr< restbed::Session > &session,
                     rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    if ( doc.FindMember( "path" ) == doc.MemberEnd() ||
         !doc["path"].IsString() ) {
        // TODO validate path, also validate against config
        sendResponse( "ERROR: Invalid path!", 401, session );
        return;
    }
    if ( doc.FindMember( "info" ) == doc.MemberEnd() ||
         !doc["info"].IsObject() ) {
        sendResponse( "ERROR: Invalid info!", 401, session );
        return;
    }
    std::string path = doc["path"].GetString();
    RenameObject renamer;
    rapidjson::StringBuffer sb;
    rapidjson::Writer< rapidjson::StringBuffer > writer( sb );
    doc["info"].Accept( writer );
    std::string s = sb.GetString();
    renamer.fromJson( s );
    sendResponse( renamePathJson( path, renamer ), 200, session );
}

std::string getFilesJson() {
    std::ostringstream res;
    res << "{\n  \"files\": [\n";
    auto files = getFilesInSource( cfg.getSourcePath() );
    if ( !files.empty() ) {
        for ( const auto &file : files ) {
            res << "    {\n      \"path\": \"" << safeJson( file.getPath() )
                << "\",\n";
            res << "      \"depth\": " << file.getDepth() << ",\n";
            res << "      \"type\": \""
                << ( file.getFileType() == TYPE_FILE ? "file" : "directory" )
                << "\"\n    },\n";
        }
        res.seekp( -2, std::ios_base::end );
    }
    res << "\n  ]\n}";
    return res.str();
}

void getFilesRest( const std::shared_ptr< restbed::Session > &session,
                   rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    sendResponse( getFilesJson(), restbed::OK, session );
}

std::vector< std::pair< uint64_t, std::string > > getTargets() {
    std::vector< std::pair< uint64_t, std::string > > result;
    const auto &targets = cfg.getTargetPaths();
    for ( uint64_t i = 0; i < targets.size(); i++ ) {
        result.emplace_back( i, targets[i].second );
    }
    return result;
}

std::string getTargetsJson() {
    std::ostringstream res;
    res << "{\n  \"targets\": [\n";
    auto targets = getTargets();
    if ( !targets.empty() ) {
        for ( const auto &target : targets ) {
            res << "  {\n"
                << "    \"id\": " << target.first << ",\n";
            res << "    \"name\": \"" << safeJson( target.second )
                << "\"\n  },\n";
        }
        res.seekp( -2, std::ios_base::end );
    }
    res << "\n  ]\n}";
    return res.str();
}

void getTargetsRest( const std::shared_ptr< restbed::Session > &session,
                     rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    sendResponse( getTargetsJson(), restbed::OK, session );
}

std::string getTargetDirectoriesJson( uint64_t id ) {
    if ( id >= cfg.getTargetPaths().size() ) {
        return "";
    }
    std::ostringstream res;
    res << "{\n  \"target_directories\": [\n";
    auto dirs = getTargetDirectories( cfg.getTargetPaths()[id].first );
    if ( !dirs.empty() ) {
        for ( const auto &dir : dirs ) {
            res << "  \"" << safeJson( dir.getPath() ) << "\",\n";
        }
        res.seekp( -2, std::ios_base::end );
    }
    res << "\n  ]\n}";
    return res.str();
}

void getTargetDirectoriesRest(
    const std::shared_ptr< restbed::Session > &session,
    rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    uint64_t id = 0;
    if ( doc.FindMember( "path_id" ) == doc.MemberEnd() ||
         !doc["path_id"].IsUint64() ||
         ( id = doc["path_id"].GetUint64() ) >= cfg.getTargetPaths().size() ) {
        sendResponse( "ERROR: Invalid path_id!", 401, session );
        return;
    }
    sendResponse( getTargetDirectoriesJson( id ), restbed::OK, session );
}

std::pair< bool, std::string > move( std::string path, uint64_t target_id,
                                     const std::string &containing_dir ) {
    if ( path[0] != '/' ) {
        path = cfg.getSourcePath() + "/" + path;
    }
    auto canon_path = FSLib::canonical( path );
    if ( canon_path.substr( 0, cfg.getSourcePath().length() ) !=
         cfg.getSourcePath() ) {
        return { false, "Invalid path" };
    }

    if ( target_id >= cfg.getTargetPaths().size() ) {
        return { false, "Invalid target_id" };
    }
    if ( !FSLib::exists( path ) ) {
        return { false, "Source doesn't exist" };
    }
    auto target_start = cfg.getTargetPaths()[target_id].first;
    auto target_dir = target_start + FSLib::dir_divisor + containing_dir;
    auto target_canon = FSLib::canonical( target_dir );
    if ( target_canon.substr( 0, target_start.length() ) != target_start &&
         !target_canon.empty() ) {
        return { false, "Invalid target" };
    }
    // might result in needless false positives, but better be safe than sorry
    if ( target_canon.empty() &&
         target_dir.find( ".." ) != std::string::npos ) {
        return { false, "Invalid target" };
    }
    if ( !FSLib::exists( target_dir ) ) {
        FSLib::createDirectoryFull( target_dir );
    }
    return { FSLib::rename( path, target_dir + FSLib::dir_divisor +
                                      FSLib::getFileName( path ) ),
             "Library error" };
}

std::string moveJson( const std::string &path, uint64_t target_id,
                      const std::string &containing_dir ) {
    std::ostringstream res;
    res << "{\n  \"success\": ";
    auto move_result = move( path, target_id, containing_dir );
    if ( move_result.first ) {
        res << "true";
    } else {
        res << "false";
    }
    res << ",\n";
    if ( !move_result.first ) {
        res << "  \"error\": \"" << safeJson( move_result.second ) << "\"\n";
    }
    res << "}";
    return res.str();
}

void moveRest( const std::shared_ptr< restbed::Session > &session,
               rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    std::string containing_dir;
    if ( doc.FindMember( "path" ) == doc.MemberEnd() ||
         !doc["path"].IsString() ) {
        // TODO validate path, also validate against config
        sendResponse( "ERROR: Invalid path!", 401, session );
        return;
    }
    uint64_t id = 0;
    if ( doc.FindMember( "target_id" ) == doc.MemberEnd() ||
         !doc["target_id"].IsUint64() ||
         ( id = doc["target_id"].GetUint64() ) >=
             cfg.getTargetPaths().size() ) {
        sendResponse( "ERROR: Invalid target_id!", 401, session );
        return;
    }
    if ( doc.FindMember( "containing_dir" ) != doc.MemberEnd() &&
         doc["containing_dir"].IsString() ) {
        containing_dir = doc["containing_dir"].GetString();
    }
    std::string path = doc["path"].GetString();
    // TODO correct response code
    sendResponse( moveJson( path, id, containing_dir ), 200, session );
}

std::pair< bool, std::string > remove( std::string path ) {
    if ( path[0] != '/' ) {
        path = cfg.getSourcePath() + "/" + path;
    }

    auto canon_path = FSLib::canonical( path );
    if ( canon_path.substr( 0, cfg.getSourcePath().length() ) !=
         cfg.getSourcePath() ) {
        return { false, "Invalid path" };
    }

    if ( !FSLib::exists( path ) ) {
        return { false, "Source doesn't exist" };
    }
    return { FSLib::deleteFile( path ), "Library error" };
}

std::string removeJson( const std::string &path ) {
    std::ostringstream res;
    res << "{\n  \"success\": ";
    auto remove_result = remove( path );
    if ( remove_result.first ) {
        res << "true";
    } else {
        res << "false";
    }
    res << ",\n";
    if ( !remove_result.first ) {
        res << "  \"error\": \"" << safeJson( remove_result.second ) << "\"\n";
    }
    res << "}";
    return res.str();
}

void removeRest( const std::shared_ptr< restbed::Session > &session,
                 rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( !verifyLogin( session, doc ) ) {
        return;
    }
    if ( doc.FindMember( "path" ) == doc.MemberEnd() ||
         !doc["path"].IsString() ) {
        // TODO validate path, also validate against config
        sendResponse( "ERROR: Invalid path!", 401, session );
        return;
    }
    std::string path = doc["path"].GetString();
    // TODO correct response code
    sendResponse( removeJson( path ), 200, session );
}

std::string loginJson( const std::string &user ) {
    std::ostringstream res;
    res << "{\n  \"token\": \"" << createLoginToken( user ) << "\"\n}";
    return res.str();
}

void loginRest( const std::shared_ptr< restbed::Session > &session,
                rapidjson::GenericDocument< rapidjson::UTF8<> > &doc ) {
    if ( doc.FindMember( "user" ) == doc.MemberEnd() ||
         !doc["user"].IsString() ) {
        sendResponse( "ERROR: Invalid user!", 401, session );
        return;
    }
    if ( doc.FindMember( "password" ) == doc.MemberEnd() ||
         !doc["password"].IsString() ) {
        sendResponse( "ERROR: Invalid password!", 401, session );
        return;
    }
    const auto *user = doc["user"].GetString();
    const auto *password = doc["password"].GetString();
    bool valid = false;
    for ( const auto &user_cfg : cfg.getUsers() ) {
        if ( user_cfg.first == user ) {
            valid = user_cfg.second == password;
            break;
        }
    }
    if ( valid ) {
        sendResponse( loginJson( user ), 200, session );
    } else {
        sendResponse( "Invalid user/password", 401, session );
    }
}

void getOptionsCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getOptionsRest );
}

void getCustomKeysCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getCustomKeysRest );
}

void getFieldOptionsCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getFieldOptionsRest );
}

void getFieldDefaultCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getFieldDefaultRest );
}

void renameCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, renamePathRest );
}

void getFilesCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getFilesRest );
}

void getTargetsCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getTargetsRest );
}

void getTargetDirectoriesCall(
    const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, getTargetDirectoriesRest );
}

void moveCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, moveRest );
}

void removeCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, removeRest );
}

void loginCall( const std::shared_ptr< restbed::Session > &session ) {
    performPostFunc( session, loginRest );
}

int main( int argc, char **argv ) {
    cfg.readConfiguration( "/etc/renameserver/main.cfg" );

    libraries = getLibraries( cfg.getLibraries() );
    for ( auto &library : libraries ) {
        library.init( library.config );
    }

    restbed::Service service;

    auto get_libraries = std::make_shared< restbed::Resource >();
    get_libraries->set_path( "/get_libraries" );
    get_libraries->set_method_handler( "GET", getLibrariesRest );
    service.publish( get_libraries );

    auto search = std::make_shared< restbed::Resource >();
    search->set_path( "/search" );
    search->set_method_handler( "POST", getOptionsCall );
    service.publish( search );

    auto custom_fields = std::make_shared< restbed::Resource >();
    custom_fields->set_path( "/get_custom_fields" );
    custom_fields->set_method_handler( "POST", getCustomKeysCall );
    service.publish( custom_fields );

    auto field_options = std::make_shared< restbed::Resource >();
    field_options->set_path( "/get_field_options" );
    field_options->set_method_handler( "POST", getFieldOptionsCall );
    service.publish( field_options );

    auto field_default = std::make_shared< restbed::Resource >();
    field_default->set_path( "/get_field_default" );
    field_default->set_method_handler( "POST", getFieldDefaultCall );
    service.publish( field_default );

    auto rename_path = std::make_shared< restbed::Resource >();
    rename_path->set_path( "/rename" );
    rename_path->set_method_handler( "POST", renameCall );
    service.publish( rename_path );

    auto get_files = std::make_shared< restbed::Resource >();
    get_files->set_path( "/get_files" );
    get_files->set_method_handler( "POST", getFilesCall );
    service.publish( get_files );

    auto get_targets = std::make_shared< restbed::Resource >();
    get_targets->set_path( "/get_targets" );
    get_targets->set_method_handler( "POST", getTargetsCall );
    service.publish( get_targets );

    auto get_target_directories = std::make_shared< restbed::Resource >();
    get_target_directories->set_path( "/get_target_directories" );
    get_target_directories->set_method_handler( "POST",
                                                getTargetDirectoriesCall );
    service.publish( get_target_directories );

    auto move = std::make_shared< restbed::Resource >();
    move->set_path( "/move" );
    move->set_method_handler( "POST", moveCall );
    service.publish( move );

    auto remove = std::make_shared< restbed::Resource >();
    remove->set_path( "/remove" );
    remove->set_method_handler( "POST", removeCall );
    service.publish( remove );

    auto login = std::make_shared< restbed::Resource >();
    login->set_path( "/login" );
    login->set_method_handler( "POST", loginCall );
    service.publish( login );

    auto settings = std::make_shared< restbed::Settings >();
    settings->set_port( 1984 );
    settings->set_default_header( "Connection", "close" );

    service.start( settings );

    return 0;
}
