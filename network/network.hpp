#ifndef NETWORK_HPP
#define NETWORK_HPP

#ifdef _WIN32

#include <Windows.h>
#include <WinInet.h>
#include <string>

using string = std::wstring;

#else

#include <curl/curl.h>
#include <string>

using string = std::string;

#endif

class Request {
public:
    Request();
    ~Request();
    std::string get( const string &url );
    std::string post( const string &url, const std::string &data );
    void addHeader( const string &header );
    void clearHeader();
    bool initSuccessful();
    void setServer( const string &server );
    int lastResponseCode();

private:
#ifdef _WIN32
    HINTERNET _hInternet;
    HINTERNET _hConnect = nullptr;
    std::wstring _headers = L"";
    int status_code{};
#else
    CURL *_curl_handle = nullptr;
    struct curl_slist *_chunk = nullptr;
    string _server;
#endif
};

#endif
