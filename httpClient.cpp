#include "httpClient.hpp"

//#pragma comment(lib, "Ws2_32.lib") //Linking instructions for Visual C++, will not work for g++. Link manually instead 
//-lws2_32 flag added to g++


    HttpClient::HttpClient(HttpClient::REQUEST_TYPE requestType){
            //Initialise WinSocksDLL
            this -> iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
            if(this -> iResult != 0){
                std::cout << "WinSock failed to start" << "\n";
            }
            this -> port = DEFAULT_PORT;
            this -> result=NULL;
            this -> ptr = NULL;
            this -> hints = {};
            this -> requestType = requestType;   
            this -> hints.ai_family = AF_UNSPEC;
            this -> hints.ai_socktype = SOCK_STREAM;
            this -> hints.ai_protocol = IPPROTO_TCP;

    };
    void HttpClient::createSocket(){
        std::cout << "Resolving ip address from hostname "<<  this -> hostname  << " on port " << this -> port << "\n";
        
        iResult = getaddrinfo(this -> hostname, this -> port, &hints, &result);
        if(iResult != 0){
            std::cout << "Requested Server failed. Error Code:  " << iResult << "\t Error Description: "<< gai_strerror(iResult) << "\n";
            WSACleanup();
            return;
        }
        this -> ptr = this -> result;
        currSocket = INVALID_SOCKET;
        //Initialise socket object from the response we receive from DNS resolution of IP address from HOSTNAME 
        std::cout << "Initialising Socket" << "\n";
        currSocket = socket(this -> ptr -> ai_family,this -> ptr -> ai_socktype, this -> ptr -> ai_protocol);
        if (currSocket == INVALID_SOCKET) 
            {
                std::cout << "Error at socket(): " <<  WSAGetLastError() << "\n";
                freeaddrinfo(this -> ptr);
                WSACleanup();
                return;
            }

        iResult = connect(currSocket,this -> ptr -> ai_addr,(int)this -> ptr -> ai_addrlen); 

        if(iResult == SOCKET_ERROR)
            {
                std::cout << "Socket Error" << "\n";
                closesocket(currSocket);
                currSocket = INVALID_SOCKET;
                return;
            }
        return;
    }
    void HttpClient::request(std::string query)
    {
        char *buffer = (char*)std::calloc(DEFAULT_BUFLEN,sizeof(char));
        std::string requestType;
        //const char* httpRequest = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
        if(this -> requestType == REQUEST_TYPE::GET){
            requestType = "GET ";  
        }
        std::string host = this -> hostname;
        std::string httpRequest = requestType + "/" + query + " HTTP/1.1\r\n" + "Host: " + host +"\r\n\r\n"; 
        std::cout << httpRequest << "\n";
        send(currSocket, httpRequest.c_str(), strlen(httpRequest.c_str()), 0);
        iResult = recv(currSocket,buffer,DEFAULT_BUFLEN,0);
        if(iResult > 0 ){
                std::cout << "Bytes received: " << iResult << "\n";
            }
            else if(iResult == 0){
                std::cout << "no data received" << "\n";
            }
            else{
                std::cout << "recv failed : " << WSAGetLastError() << "\n";
            }
        freeaddrinfo(this -> ptr);
        free(buffer);

        if (currSocket == INVALID_SOCKET) {
            std::cout << "Unable to connect to server!" << "\n";
            closesocket(currSocket);
            WSACleanup();
            return;
        }

        };


int main(){
    enum HttpClient::REQUEST_TYPE requestType= HttpClient::REQUEST_TYPE::GET;
    PCSTR hostname = "youtube.com";
    HttpClient client (requestType);
    client.hostname = hostname;
    
    std::cout << "HTTP Client Created" << "\n";  

    client.createSocket();
    std::string a = ""; 
    client.request(a);
  
    return 0;
}