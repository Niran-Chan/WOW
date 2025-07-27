#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef HTTPCLIENT // include guard
#define HTTPCLIENT
#include <iostream>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#define DEFAULT_BUFLEN 8192 //Default buffer length of 8KB

constexpr PCSTR DEFAULT_PORT = "443";



class HttpClient {
    public:
   
        enum REQUEST_TYPE {
            GET,
            POST,
            PUT,
            PATCH,
            DEL

        };
        
        REQUEST_TYPE requestType;
        PCSTR hostname;
        PCSTR port;

        int iResult;

        WSADATA wsaData;

        //ip address info struct initialised to hold information from transactions
        addrinfo *result;
        addrinfo *ptr;
        addrinfo hints;

        SOCKET currSocket;
        HttpClient(REQUEST_TYPE requestType);
        void createSocket();
        void request(std::string query);

};


#endif /* HTTPCLIENT */