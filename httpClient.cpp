#include "httpClient.hpp"


int main(){
    CURLcode globalHandleRet = curl_global_init(CURL_GLOBAL_ALL); //Global initialisation for CURL
    CURL * curl = curl_easy_init(); //Create CURL easy handle
    if(curl) {
        CURLcode res;
        std::cout << "Success" << "\n";
        curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
        res = curl_easy_perform(curl);
        std::cout << res << "\n";
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
    
}