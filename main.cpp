#include <Windows.h> //Include all the special windows types defined later on
#include <iostream>
#include <string>
#include <WinUser.h>
#include <vector>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp> 
#include <unordered_map>

#define UNICODE //Force MACROs to resolve to wide string equivalent instead of legacy ANSI 
#define TRANSLATE_ID   1001
#define CLEAR_ID   1002
#define SETTINGS_ID 1003
#define SETTINGS_APPLY_ID 2001
#define SETTINGS_OK_ID 2002
#define OPEN_PROG_HOTKEY_ID 1
#define SOURCELANG_ID 3001
#define TARGETLANG_ID 3002

using json = nlohmann::json;

HWND windowHandler; //Main Window
HWND settingsWindowHandler;
HWND translateButtonHandler;
HWND translatedTextHandler;
HWND scrollBarHandler;
HWND clearButtonHandler;
HWND sourceTextHandler;
HWND settingsButtonHandler;
HWND sourceLangListHandler;
HWND targetLangListHandler;

//Settings Window
HWND APITextHandler;

std::vector<HWND> windowHandlers; //Just to keep track of all windows currently present

//Custom Class Names used for custom windows
LPCWSTR mainClassName = L"mainClass";
LPCWSTR settingsClassName = L"settingsClass";

//Settings Keys
LPSTR API_KEY = "Translate_API";
//Constants
const int MAX_TEXT_LEN = 4096;
const int MAX_BUFFER_CHAR_ENV = 32767;

//Global Application Maps
std::unordered_map<std::string,std::string> availableLanguages;

//Initialise CURL handle from libcurl    
CURLcode globalHandleRet = curl_global_init(CURL_GLOBAL_ALL); //Global initialisation for CURL

CURL * curl = curl_easy_init(); //Create CURL easy handle
enum APP_MODE{
    ONLINE = 1,
    OFFLINE = 0
};
enum OPACITY_LEVEL{
    LOW = 0,
    HIGH = 255
};
enum FLEX_DIR{
    ROW = 0,
    COL = 1
};
enum API_REQUEST_TYPE{
    TRANSLATE_REQ = 1,
    GET_LANG = 2
};

//Initial global enum definition
enum OPACITY_LEVEL OPACITY = LOW;
APP_MODE APP_STATUS = APP_MODE::OFFLINE;

//Global Variables
std::string sourceLang = "en";
std::string targetLang = "fr";
/*
    request using curl object and url passed in and return response
*/

// Callback to store the API response in a string
size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((const char*)contents, size * nmemb);
    return size * nmemb;
}
std::string request(CURL* curl,std::string url,std::string params,API_REQUEST_TYPE reqType){
    CURLcode res;
    std::string readBuffer; //Get response from the url call
    curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
    if(reqType == API_REQUEST_TYPE::TRANSLATE_REQ){
        //std::cout << "URL:" << url << "\n";
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str()); //Set Post Fields
    }   
    //else if(reqType == API_REQUEST_TYPE::GET_LANG){
    else{
    curl_easy_setopt(curl, CURLOPT_HTTPGET,1L);
        url += std::string("?") + params;
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
        //std::cout << "URL:" << url << "\n";
    } 

    //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr); // libcurl sets content-type automatically
    
    //std::cout << "Sending: " << url << "\n";
    // Write response to a string
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    res = curl_easy_perform(curl);
    if(res == CURLE_OK){
        std::cout << "API request successful!" << "\n";
        std::cout << res << "\n";
        //std::cout << readBuffer << "\n";
        json j = json::parse(readBuffer); //Parse the buffer data as valid JSON
       
        // Pretty-print the whole JSON with indentation
        //std::cout << "\nParsed JSON:\n" << j.dump(4) << "\n";
        if(reqType == API_REQUEST_TYPE::TRANSLATE_REQ){
            if (j.contains("data") && j["data"].contains("translations")) {
                auto translations = j["data"]["translations"];
                if (!translations.empty() && translations[0].contains("translatedText")) {
                    std::string translatedText = translations[0]["translatedText"];
                    //std::cout << "\nTranslated Text: " << translatedText << "\n";
         
                    return translatedText;
                    
                }
            }
            else{
                MessageBoxA(windowHandler,"Failed to retrieve appropriate object from API. Check if object is of correct type!","Translate API failed",MB_OK);
                return "";
            }
        }
   
        else if (reqType == API_REQUEST_TYPE::GET_LANG){
            
            if(!j.empty() && j.contains("data") && j["data"].contains("languages"))
            {
                
                for(int i =0; i < j["data"]["languages"].size();++i){
                    std::string key = j["data"]["languages"][i]["name"];
                    std::string val = j["data"]["languages"][i]["language"];
                    availableLanguages[key]= val;
                }
                        //Explore unordered_map to print
                        /*
                for(const std::pair<const std::string, std::string> &n : availableLanguages){
                    std::cout << "Key: " <<n.first << "\tVal: " << n.second << "\n";
            
                    }
                    */
            }
            
  
            return "";
        }
    }
    else{
        std::cout << "Failed to parse API request: " << GetLastError() << "\n";
    }

    return "";
    
}

void CreateSettingsWindow(){
    int X =0;
    int Y = 0;
    int nWidth = 350;
    int nHeight = 350; 
    settingsWindowHandler = CreateWindowExW(WS_EX_TOPMOST,settingsClassName,L"Settings Window",WS_OVERLAPPEDWINDOW,X,Y,nWidth,nHeight,NULL,NULL,NULL,NULL);
}
/*
    Function to layout similar to flex by using only the handlers that we have already defined. Layout is done relative to the first handle in the vector (parent handle) as offset
*/
void flexLayout(std::vector<HWND> componentHandlers,int parentX,int parentY, int gap,enum FLEX_DIR dir){
    int xGap = gap,yGap =0;
    if(dir == FLEX_DIR::COL)
    {
        yGap = gap;
        xGap = 0;
    }
    int x,y,width,height; 
    x = parentX;
    y = parentY;
    for(int i =0; i < componentHandlers.size();++i){
        HWND currHandler = componentHandlers[i];
        RECT currCoords;
        GetWindowRect(currHandler,&currCoords);
        width = currCoords.right - currCoords.left;
        height = currCoords.bottom - currCoords.top;   
        BOOL ret = SetWindowPos(currHandler,NULL,x,y,width,height,0);
        if(ret == 0){
            std::cout << "Failed in setting new coordinates in flexLayout. Error Code: " << GetLastError() << "\n";
        }
        if(dir == FLEX_DIR::COL) {y = y + height + yGap;}
        else {x = x + width + xGap;}
    }
}
TRACKMOUSEEVENT generateTrackMouseEventObjects(HWND hWnd){
        TRACKMOUSEEVENT tme = {0};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = hWnd;
        tme.dwHoverTime = 10;
        return tme;
}
LRESULT WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
    
   switch(uMsg){
    case WM_CREATE:
        {
        std::vector <HWND> textFieldHandlers;
        std::vector<HWND> buttonHandlers;
        int buttonWidth = 100;
        int buttonHeight = 25;    
        //Create Button for Translate
        translateButtonHandler = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"Translate\n", WS_CHILD | WS_VISIBLE | WS_BORDER,20,20,buttonWidth,buttonHeight,hWnd,(HMENU)TRANSLATE_ID,NULL,NULL);
        if(translateButtonHandler == NULL){
            std::cout << "Error with creating buttonHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(translateButtonHandler);
        buttonHandlers.push_back(translateButtonHandler);
        
        sourceTextHandler = CreateWindowExW(0, L"EDIT", L"Enter language to be translated here",WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL ,0, 0, 300, 300,hWnd, NULL, GetModuleHandle(NULL), NULL);
        if(sourceTextHandler == NULL){
            std::cout << "Error with creating sourceTextHandler" << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(sourceTextHandler);
        textFieldHandlers.push_back(sourceTextHandler);


        translatedTextHandler = CreateWindowExW(0, L"EDIT", L"Text will be translated here :)",WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 0, 0, 300, 300,hWnd, NULL, GetModuleHandle(NULL), NULL);
        //translatedTextHandler = CreateWindowExW(WS_EX_TOPMOST,L"STATIC",L"Initial Text",WS_VISIBLE | WS_CHILD,100, 100, 300, 300,hWnd,NULL,NULL,NULL);
        if(translatedTextHandler == NULL){
            std::cout << "Error with creating translatedTextHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(translatedTextHandler);
        textFieldHandlers.push_back(translatedTextHandler);

        clearButtonHandler = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"Clear\n", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, buttonWidth, buttonHeight,hWnd,(HMENU)CLEAR_ID,NULL,NULL);
        if(clearButtonHandler == NULL){
            std::cout << "Error with creating clearButtonHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(clearButtonHandler);
        buttonHandlers.push_back(clearButtonHandler);

        settingsButtonHandler = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY,L"Button",L"Settings\n",WS_CHILD | WS_VISIBLE | WS_BORDER,0,0,buttonWidth,buttonHeight,hWnd,(HMENU)SETTINGS_ID,NULL,NULL);
        if(settingsButtonHandler == NULL){
            std::cout << "Error with creating settingsButtonHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(settingsButtonHandler);
        buttonHandlers.push_back(settingsButtonHandler);

        sourceLangListHandler = CreateWindowExA(0,"COMBOBOX",NULL,WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,100, 50, 100, 300,hWnd,(HMENU)SOURCELANG_ID,NULL,NULL);
        //Retrieve items data for adding to sourceLangHandler
        std::string getLangUrl = "https://translation.googleapis.com/language/translate/v2/languages";
        if(API_KEY != ""){
            std::string getLangParams = "key=" + std::string(API_KEY) +  "&target=en&model=nmt";
            request(curl,getLangUrl.c_str(),getLangParams.c_str(),API_REQUEST_TYPE::GET_LANG); //Populates global unordered_map instead
        }
        else{
            MessageBoxA(hWnd,LPCSTR("No API key registered! Set in settings"),LPCSTR("NO API KEY"),MB_OK);
        }
        //This is still blocking. Need to optimise to asynchronous eventually
        int sourceIdx =0 ;
        for(const std::pair<const std::string,std::string> &a : availableLanguages){
            SendMessageA(sourceLangListHandler,CB_ADDSTRING,0,(LPARAM)a.first.c_str());
            if(a.second == sourceLang)
            {
                //Setting the first default value
                SendMessageA(sourceLangListHandler,CB_SETCURSEL,(WPARAM)sourceIdx,NULL);
            }
            sourceIdx++;
        }
   
 
 
       targetLangListHandler = CreateWindowExA(0,"COMBOBOX",NULL,WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,100, 400, 100, 300,hWnd,(HMENU)TARGETLANG_ID,NULL,NULL);
        //This is still blocking. Need to optimise to asynchronous eventually
        int targetIdx =0;
        for(const std::pair<const std::string,std::string> &a : availableLanguages){
            SendMessageA(targetLangListHandler,CB_ADDSTRING,0,(LPARAM)a.first.c_str());
            if(a.second == targetLang)
            {
                //Setting the first default value
                SendMessageA(targetLangListHandler,CB_SETCURSEL,(WPARAM)targetIdx,NULL);
            }
            targetIdx++;
        }

        //Iterate through list of items and add into list
        flexLayout(textFieldHandlers,100,100,50,FLEX_DIR::COL);
        flexLayout(buttonHandlers,0,0,30,FLEX_DIR::ROW);
        break;
    }
    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED)
            {
                int id = LOWORD(wParam); 
                switch(id){
                    case TRANSLATE_ID:
                    {
                        //std::cout << "Button Pressed" << "\n";
                        //Store string up till now into temp buffer, then add to buffer, then set is back
                        LPWSTR buffer = (LPWSTR)std::calloc(MAX_TEXT_LEN ,sizeof(LPWSTR)); //Allocate larger buffer
                        GetWindowTextW(translatedTextHandler,buffer,MAX_TEXT_LEN);
                        /*
                        //BUFFER CHECK
                        std::cout << "Stored buffer: " << "\n";
                        for(int i =0; i < MAX_TEXT_LEN;++i){
                            if(buffer[i] == NULL)
                                break;
                            std::cout << buffer[i] << "\t";
                        }
                        std::cout << "\n";
                        */
                        
                        //Use libcurl to retrieve results
                        //https://cloud.google.com/translate/docs/reference/rest/v2/translate
                        std::string url = "https://translation.googleapis.com/language/translate/v2";
                        //Retrieve params to form params string
                        std::string key (API_KEY);
                        /*
                        Retrieve input in textfield 
                        We need UTF16 -> UTF8 for libcurl to process
                        Libcurl result -> UTF8 for display
                        */
                        LPWSTR buf = (LPWSTR)calloc(MAX_BUFFER_CHAR_ENV,sizeof(WCHAR));
                        GetWindowTextW(sourceTextHandler,buf,MAX_BUFFER_CHAR_ENV);
                        ULONG utf8BufSize = WideCharToMultiByte(CP_UTF8,0,buf,-1,NULL,0,NULL,NULL);
                        LPSTR utf8Buf = (LPSTR)calloc(utf8BufSize,sizeof(CHAR));
                        WideCharToMultiByte(CP_UTF8,0,buf,-1,utf8Buf,utf8BufSize,NULL,NULL);
                        char *q = curl_easy_escape(curl, utf8Buf, 0);
                        //std::string q (buf);
                        std::string target(targetLang);
                        std::string source(sourceLang);
                        std::string model("nmt");
                        std::string format("text");
                        std::string params = "key=" + key + "&q=" + q + "&target=" + target + "&format=" + format + "&source=" + source + "&model=" + model; 
                        std::string response = request(curl,url.c_str(),params.c_str(),API_REQUEST_TYPE::TRANSLATE_REQ);
                       
                        int reqSize = MultiByteToWideChar(CP_UTF8,0,response.c_str(),-1,NULL,0);
                        LPWSTR convertedResp = (LPWSTR)std::calloc(reqSize,sizeof(WCHAR));
                        MultiByteToWideChar(CP_UTF8, 0,response.c_str(), -1,convertedResp,reqSize);
                        SetWindowTextW(translatedTextHandler,convertedResp);
                        //std::wstring temp (buffer[MAX_TEXT_LEN-2] != 0 ? LPWSTR(L"") : buffer); //-2 because null terminated string, so last index always == 0
                        //temp += LPWSTR(L"\nButton Pressed");
                        //SetWindowTextW(translatedTextHandler,temp.c_str());
                        break;
                    }
                    case CLEAR_ID:
                    {
                        std::cout << "Clear Button Pressed" << "\n";
                        SetWindowTextW(sourceTextHandler,NULL);
                        break;
                    }
                    case SETTINGS_ID:
                    {
                        //Check if settings window is ready to be opened
                        if(!settingsWindowHandler){
                            CreateSettingsWindow();
                            std::cout << "Creating Settings window again" << "\n";
                        }
                        //To open new window, call window handler to ShowWindow
                         //Show and Update paint on window
                        ShowWindow(settingsWindowHandler, SW_SHOW);
                        UpdateWindow(settingsWindowHandler);
    
                        std::cout << "Settings Button Pressed" << "\n";
                        break;

                    }
            }
            }
     
        
        else if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            //We are using 2 combo boxes. Need identification.
            int id = LOWORD(wParam);
            std::cout << id << "\n";
            switch(id){
                case SOURCELANG_ID:
                {
                    std::cout << "New Language Selected" << "\n";
                    // Get selected item index
                        int sel = SendMessage(sourceLangListHandler, CB_GETCURSEL, 0, 0); //Get index of current item
                        if (sel != CB_ERR) {
                            char* buf= (char*) std::calloc(MAX_BUFFER_CHAR_ENV,sizeof(char));
                            SendMessageA(sourceLangListHandler, CB_GETLBTEXT, sel, (LPARAM) buf); //Get text of current item
                            std::cout << "Language Code: " << availableLanguages[buf] << "\n";
                            if(availableLanguages.find(buf) != availableLanguages.end()){sourceLang = availableLanguages[buf];}
                            else{MessageBoxA(hWnd,"Language Unavailable","No Language Code was found for selected language. Not gonna even try",MB_OK);}
                            
                        }
                break;
                }

                case TARGETLANG_ID:
                {
                        int sel = SendMessage(targetLangListHandler, CB_GETCURSEL, 0, 0); //Get index of current item
                        if (sel != CB_ERR) {
                            char* buf= (char*) std::calloc(MAX_BUFFER_CHAR_ENV,sizeof(char));
                            SendMessageA(targetLangListHandler, CB_GETLBTEXT, sel, (LPARAM) buf); //Get text of current item
                            std::cout << "Language Code: " << availableLanguages[buf] << "\n";
                            if(availableLanguages.find(buf) != availableLanguages.end()){targetLang = availableLanguages[buf];}
                            else{MessageBoxA(hWnd,"Language Unavailable","No Language Code was found for selected language. Not gonna even try",MB_OK);}
                            
                        }
                break;   
                }
            }
            break;
        }
   

    case WM_HOTKEY:
    {
        HWND previousWindow;
        if(wParam == OPEN_PROG_HOTKEY_ID){
            if(OPACITY == LOW)
                    { previousWindow = GetForegroundWindow();  // Save this before changing focus
                        OPACITY = HIGH;
                        SetLayeredWindowAttributes(hWnd, 0, OPACITY, LWA_ALPHA);
                        SetFocus(sourceTextHandler);
                        SetForegroundWindow(sourceTextHandler);
                    }
                    else
                    {
                        OPACITY=LOW;
                        SetLayeredWindowAttributes(hWnd, 0, OPACITY, LWA_ALPHA);
                        SetFocus(previousWindow);
                        SetForegroundWindow(previousWindow);
                    }
        }
        break;
    }
    case WM_DESTROY:
        std::cout <<"Exiting Application" << "\n";
        UnregisterHotKey(hWnd,OPEN_PROG_HOTKEY_ID);
        PostQuitMessage(0);
        return (0);
   }
   return DefWindowProcW(hWnd, uMsg, wParam, lParam); 
};
void populateEnvironmentVars(){
   //Parse .env file that is available locally to populate my environment variables
    std::wifstream myEnvFile(".env");
    std::wstring currLine;
    
  
    while (getline (myEnvFile, currLine))
    {
        //From text, we tokenise to split 
        int tokenIdx = 0;
        for(int i =0; i < sizeof(currLine);i +=1){
            if (currLine[i] == '='){
                tokenIdx = i;
                break;
            }
        } 
        std::wstring envFileName = currLine.substr(0,tokenIdx);
        std::wstring envFileValue = currLine.substr(tokenIdx+1);
        BOOL res = SetEnvironmentVariableW(envFileName.c_str(),envFileValue.c_str());
        if(res == 0){
                std::cout << "Error in setting env variable: " << GetLastError() << "\n";
        
            }
        //std::wcout <<"Environment Variable:" <<  envFileName << "|Value:" << envFileValue << "|\n"; 

    }

    //Inherit values that are important for the rest of the program
    API_KEY = (LPSTR)std::calloc(MAX_BUFFER_CHAR_ENV,sizeof(LPSTR));
    BOOL ret = GetEnvironmentVariableA("TRANSLATE_API",API_KEY,MAX_BUFFER_CHAR_ENV);
    if(ret == 0){
        std::cout << "Failed getting value from environment variable. Keeping default." << GetLastError() << "\n";
        std::string a = "DEFAULT_EMPTY";
        std::strcpy(API_KEY,a.c_str());
    }
    // Close the file
    myEnvFile.close(); 
    
};
LRESULT settingsWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,LPARAM lParam){
    switch(uMsg){
        case WM_CREATE:{
   
            HWND APITextTitle = CreateWindowExW(0,L"STATIC",L"Translation API key",WS_CHILD | WS_VISIBLE, 0,0,100,20,hWnd,NULL,GetModuleHandle(NULL),NULL);
            LPSTR currEnvAPIKey = (LPSTR)std::calloc(MAX_TEXT_LEN,sizeof(LPSTR)); //Change buffer allocated length later
            GetEnvironmentVariableA(API_KEY,currEnvAPIKey,MAX_TEXT_LEN);
            APITextHandler = CreateWindowExA(0, "EDIT", currEnvAPIKey,WS_CHILD | WS_VISIBLE, 0, 0, 100, 20,hWnd, NULL, GetModuleHandle(NULL), NULL);
            std::vector<HWND> div {APITextTitle,APITextHandler};
            flexLayout(div,0,0,20,FLEX_DIR::ROW);
            
            std::cout << "Text Handler in settings created" << "\n";
            int buttonWidth = 100;
            int buttonHeight = 20;
            HWND OKButton  =  CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"OK\n", WS_CHILD | WS_VISIBLE | WS_BORDER,0,0,buttonWidth,buttonHeight,hWnd,(HMENU)SETTINGS_OK_ID,NULL,NULL);
            HWND applyButton = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"Apply\n", WS_CHILD | WS_VISIBLE | WS_BORDER,250,250,buttonWidth,buttonHeight,hWnd,(HMENU)SETTINGS_APPLY_ID,NULL,NULL);
            std::vector<HWND> buttonDiv {OKButton,applyButton};
            flexLayout(buttonDiv,100,270,20,FLEX_DIR::ROW);
            break;
        }
        case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED)
            {
                int id = LOWORD(wParam); 
                switch(id){
                    case SETTINGS_APPLY_ID:
                    {
                        //std::cout << "Button Pressed" << "\n";
                        //Store string up till now into temp buffer, then add to buffer, then set is back
                        LPSTR buffer = (LPSTR)std::calloc(MAX_TEXT_LEN ,sizeof(LPSTR)); //Allocate larger buffer
                        GetWindowTextA(APITextHandler,buffer,MAX_TEXT_LEN);
                        //Create environment variable
                        BOOL ret = SetEnvironmentVariableA(API_KEY,buffer);
                        if(ret == 0){
                            std::cout << "Setting Environment Var Failed!" << "\n";
                        }
                        /*
                        for(int i =0; i < MAX_TEXT_LEN;++i){
                            std::cout << buffer[i] << "\t";
                        }
                        std::cout << "\n";
                        */
                        break;
                    }
                    case SETTINGS_OK_ID:
                    {
                       DestroyWindow(hWnd);
                    }
            }
            }
        break;
        case WM_DESTROY:{
            std::cout <<"Exiting Settings" << "\n";
            settingsWindowHandler = NULL;
            return (0);
        }
   }
   return DefWindowProcW(hWnd, uMsg, wParam, lParam); 
}

int main (){
    //Read from .env file for auto-environment vars population
    populateEnvironmentVars();
    if (globalHandleRet != CURLE_OK) {
        std::cerr << "curl_global_init() failed\n";
    }
    DWORD extendedWindowStyle = WS_EX_TOPMOST |  WS_EX_LAYERED  ;

    //Test API connection to set application online/offline status
    if(!curl) {
  
        std::cout << "Curl Handle Initialisation failed! Offline mode engaged. No online translation will be conducted" <<"\n";
        APP_STATUS = APP_MODE::OFFLINE;
    }

    //Define Windows Class to be registered 
    WNDCLASSEXW mainClass = { };
    mainClass.cbSize = sizeof(mainClass);
    mainClass.lpfnWndProc = WndProc;
    mainClass.hInstance = GetModuleHandle(NULL); 
    mainClass.lpszClassName = mainClassName;
    // translateOverlayClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    mainClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
   //Register class as ATOM for use with Window Handler later
    RegisterClassExW(&mainClass);
    LPCWSTR windowName = L"Main Window";
    DWORD windowStyle =  WS_OVERLAPPEDWINDOW;

    WNDCLASSEXW settingsClass = { };


    settingsClass.cbSize = sizeof(settingsClass);
    settingsClass.lpfnWndProc = settingsWndProc;
    settingsClass.hInstance = GetModuleHandle(NULL);
    settingsClass.lpszClassName = settingsClassName;
    settingsClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassExW(&settingsClass);

    int X = 0,Y = 0,nWidth= 600,nHeight=800;

    //Intantialise Window and Window Handler
    windowHandler = CreateWindowExW(extendedWindowStyle,mainClassName,windowName,windowStyle,X,Y,nWidth,nHeight,NULL,NULL,NULL,NULL);
    
    if(windowHandler == NULL){
        DWORD errorCode = GetLastError();
        std::cerr << "CreateWindowExW failed with error: " << errorCode << std::endl;
        return(0);
    }
    windowHandlers.push_back(windowHandler);
    CreateSettingsWindow();
    //settingsWindowHandler = CreateWindowExW(WS_EX_TOPMOST,settingsClassName,L"Settings Window",WS_OVERLAPPEDWINDOW,X,Y,nWidth,nHeight,NULL,NULL,NULL,NULL);
    std::cout << "Window and related classes initialised" << "\n";
    

    //Setting initial transparency  
    COLORREF crKey = RGB(255,255,255);
    BYTE bAlpha = OPACITY;
    SetLayeredWindowAttributes(windowHandler,crKey,bAlpha,LWA_ALPHA);
    //SetLayeredWindowAttributes(settingsWindowHandler,crKey,bAlpha,LWA_ALPHA);
    std::cout << "Transparency set for window" << "\n";

    //Register Keybinds
    RegisterHotKey(windowHandler, OPEN_PROG_HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'T');  // Ctrl + Shift + T


    //Show and Update paint on window
    int showWindowStyle = SW_SHOW;
    ShowWindow(windowHandler, showWindowStyle);
    UpdateWindow(windowHandler);
    
    //Get Messages from thread and react appropriately
    MSG msg; //MSG Struct gives packaged information from current working thread on message details
    BOOL getMessageRet;
    while ((getMessageRet = GetMessageW(&msg, NULL, 0, 0))) { //GetMessage is a hook and places message into empty message declared earlier
        if(getMessageRet == -1){
            std::cout << "Error in Window Handler. Exiting..." << "\n";
            break;
        }    
        else{
            //Translate virtual messages to character messages
            TranslateMessage(&msg);
            //Send message to WndProc
            DispatchMessageW(&msg);
            }
    }

    std::cout <<"Main Function Completed :)" << std::endl;
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    std::cout << "CURL cleanup executed successfully" << "\n";

    return 0;

}
