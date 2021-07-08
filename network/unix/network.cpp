#include "../network.hpp"
#include <iostream>
#include <memory>

size_t writeCallback( void *contents, size_t size, size_t nmemb,
                      void *target ) {
    *static_cast< std::string * >( target ) +=
        std::string( static_cast< char * >( contents ), size * nmemb );
    return size * nmemb;
}

Request::Request() {
    curl_global_init( CURL_GLOBAL_ALL );
    _curl_handle = curl_easy_init();
    if ( _curl_handle == NULL ) {
        std::cerr << "ERROR curl_easy_init" << std::endl;
    }
    curl_easy_setopt( _curl_handle, CURLOPT_WRITEFUNCTION, writeCallback );
    curl_easy_setopt( _curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
}

Request::~Request() {
    curl_easy_cleanup( _curl_handle );
    curl_slist_free_all( _chunk );
    curl_global_cleanup();
}

void cleanUp( CURL *curl_handle ) {
    curl_easy_setopt( curl_handle, CURLOPT_POST, 0 );
    curl_easy_setopt( curl_handle, CURLOPT_POSTFIELDS, "" );
}

std::string Request::get( const std::string &url ) {
    // get rid of garbage
    cleanUp( _curl_handle );
    std::string response;
    response.reserve( 100000 );
    curl_easy_setopt( _curl_handle, CURLOPT_WRITEDATA,
                      static_cast< void * >( &response ) );
    curl_easy_setopt( _curl_handle, CURLOPT_URL, ( _server + url ).c_str() );
    curl_easy_setopt( _curl_handle, CURLOPT_HTTPGET, 1 );
    auto res = curl_easy_perform( _curl_handle );
    if ( res != CURLE_OK ) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror( res )
                  << std::endl;
        return "";
    }
    return response;
}

std::string Request::post( const std::string &url, const std::string &data ) {
    std::string response;
    response.reserve( 100000 );
    curl_easy_setopt( _curl_handle, CURLOPT_URL, ( _server + url ).c_str() );
    curl_easy_setopt( _curl_handle, CURLOPT_POST, 1 );
    curl_easy_setopt( _curl_handle, CURLOPT_POSTFIELDS, data.c_str() );
    curl_easy_setopt( _curl_handle, CURLOPT_WRITEDATA,
                      static_cast< void * >( &response ) );
    auto res = curl_easy_perform( _curl_handle );
    if ( res != CURLE_OK ) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror( res )
                  << std::endl;
        return "";
    }
    return response;
}

void Request::addHeader( const std::string &header ) {
    _chunk = curl_slist_append( _chunk, header.c_str() );
    curl_easy_setopt( _curl_handle, CURLOPT_HTTPHEADER, _chunk );
}

void Request::clearHeader() {
    curl_slist_free_all( _chunk );
    _chunk = nullptr;
    curl_easy_setopt( _curl_handle, CURLOPT_HTTPHEADER, _chunk );
}

bool Request::initSuccessful() {
    return _curl_handle != nullptr;
}

void Request::setServer( const string &server ) {
    _server = server;
}

int Request::lastResponseCode() {
    long code{};
    curl_easy_getinfo( _curl_handle, CURLINFO_RESPONSE_CODE, &code );
    return code;
}
