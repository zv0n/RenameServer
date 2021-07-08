#include "../network.hpp"
#include <iostream>
#include <memory>

const wchar_t *acceptTypes[] = { L"text/*", L"application/json", nullptr };

Request::Request() {
    // Start connection
    _hInternet = InternetOpen( L"WinInet/1.0", INTERNET_OPEN_TYPE_PRECONFIG,
                               nullptr, nullptr, 0 );
    if ( !_hInternet )
        std::wcerr << "ERROR InternetOpen: " << GetLastError() << std::endl;
}

Request::~Request() {
    if ( _hConnect )
        InternetCloseHandle( _hConnect );
    if ( _hInternet )
        InternetCloseHandle( _hInternet );
}

std::string sendRequest( HINTERNET hRequest, const std::wstring &headers,
                         const std::string &optional, int &status_code ) {
    std::string response{};
    response.reserve( 10000 );

    // have to do a copy because c_str returns const
    std::unique_ptr< char > opt( new char[optional.length()] );
    memcpy( opt.get(), optional.c_str(), optional.length() );

    auto requestSent =
        HttpSendRequest( hRequest, headers.c_str(), headers.length(), opt.get(),
                         optional.length() );

    if ( !requestSent ) {
        status_code = -1;
        std::wcerr << "ERROR HttpSendRequest: " << GetLastError() << std::endl;
        return "";
    }

    char dataReceived[4096];
    unsigned long numberOfBytesRead = 0;
    // read while InternetReadFile returns true and reads more than 0 bytes
    while (
        InternetReadFile( hRequest, dataReceived, 4096, &numberOfBytesRead ) &&
        numberOfBytesRead ) {
        response.append( dataReceived, numberOfBytesRead );
    }

    wchar_t responseText[256];
    DWORD responseTextSize = sizeof( responseText );

    HttpQueryInfo( hRequest, HTTP_QUERY_STATUS_CODE, &responseText,
                   &responseTextSize, NULL );
    status_code = std::stoi( responseText );

    return response;
}

std::string Request::get( const std::wstring &url ) {
    HINTERNET hRequest =
        HttpOpenRequest( _hConnect, L"GET", url.c_str(), nullptr, nullptr,
                         acceptTypes, INTERNET_FLAG_SECURE, 0 );
    if ( !hRequest )
        std::wcerr << "ERROR HttpOpenRequest: " << GetLastError() << std::endl;

    return sendRequest( hRequest, _headers, "", status_code );
}

std::string Request::post( const std::wstring &url, const std::string &data ) {
    HINTERNET hRequest =
        HttpOpenRequest( _hConnect, L"POST", url.c_str(), nullptr, nullptr,
                         acceptTypes, INTERNET_FLAG_SECURE, 0 );
    if ( !hRequest )
        std::wcerr << "ERROR HttpOpenRequest: " << GetLastError() << std::endl;

    return sendRequest( hRequest, _headers, data, status_code );
}

void Request::addHeader( const std::wstring &header ) {
    _headers += header + L"\r\n";
}

void Request::clearHeader() {
    _headers = L"";
}

bool Request::initSuccessful() {
    return _hInternet != nullptr;
}

void Request::setServer( const string &server ) {
    // connect to server
    _hConnect = InternetConnect( _hInternet, server.c_str(),
                                 INTERNET_DEFAULT_HTTPS_PORT, nullptr, nullptr,
                                 INTERNET_SERVICE_HTTP, 0, 0 );
    if ( !_hConnect )
        std::wcerr << "ERROR InternetConnect: " << GetLastError() << std::endl;
}

int Request::lastResponseCode() {
#ifndef _WIN32
    long code{};
    curl_easy_getinfo( _curl_handle, CURLINFO_RESPONSE_CODE, &code );
    return code;
#else
    return status_code;
#endif
}
