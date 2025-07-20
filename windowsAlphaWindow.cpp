#include <Windows.h> //Include all the special windows types defined later on
#include <iostream>
#include <string>
#include <WinUser.h>
#include <vector>
#include <fstream>

#define UNICODE //Force MACROs to resolve to wide string equivalent instead of legacy ANSI 
#define TRANSLATE_ID   1001
#define CLEAR_ID   1002
#define SETTINGS_ID 1003
#define OPEN_PROG_HOTKEY_ID 1

HWND windowHandler; //Main Window
HWND translateButtonHandler;
HWND translatedTextHandler;
HWND scrollBarHandler;
HWND clearButtonHandler;
HWND sourceTextHandler;
HWND settingsButtonHandler;

std::vector<HWND> windowHandlers; //Just to keep track of all windows currently present

//Constants
const int MAX_TEXT_LEN = 4096;
const int MAX_BUFFER_CHAR_ENV = 32767;

//Variables dependent on environment variables
LPWSTR TRANSLATE_API;

enum OPACITY_LEVEL{
    LOW = 0,
    HIGH = 255
};
enum FLEX_DIR{
    ROW = 0,
    COL = 1
};

enum OPACITY_LEVEL OPACITY = LOW;
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

        flexLayout(textFieldHandlers,100,100,100,FLEX_DIR::COL);
        flexLayout(buttonHandlers,0,0,30,FLEX_DIR::ROW);
        break;
    }
    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED)
            {
                int id = LOWORD(wParam); 
                switch(id){
                    case 1001:
                    {
                        //std::cout << "Button Pressed" << "\n";
                        //Store string up till now into temp buffer, then add to buffer, then set is back
                        LPWSTR buffer = (LPWSTR)std::calloc(MAX_TEXT_LEN ,sizeof(WCHAR)); //Allocate larger buffer
                        GetWindowTextW(translatedTextHandler,buffer,MAX_TEXT_LEN);
                        /*
                        for(int i =0; i < MAX_TEXT_LEN;++i){
                            std::cout << buffer[i] << "\t";
                        }
                        std::cout << "\n";
                        */
                        
                        std::wstring temp (buffer[MAX_TEXT_LEN-2] != 0 ? LPWSTR(L"") : buffer); //-2 because null terminated string, so last index always == 0
                        temp += LPWSTR(L"\nButton Pressed");
                        SetWindowTextW(translatedTextHandler,temp.c_str());
                        break;
                    }
                    case 1002:
                    {
                        std::cout << "Clear Button Pressed" << "\n";
                        SetWindowTextW(sourceTextHandler,NULL);
                        break;
                    }
            }
            }
        break;
    

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
    TRANSLATE_API = (LPWSTR)std::calloc(MAX_BUFFER_CHAR_ENV,sizeof(LPWSTR));
    BOOL ret = GetEnvironmentVariableW(L"TRANSLATE_API",TRANSLATE_API,MAX_BUFFER_CHAR_ENV);
    if(ret == 0){
        std::cout << "Failed getting value from environment variable. Keeping default." << GetLastError() << "\n";
        std::wstring a = L"DEFAULT_EMPTY";
        wcscpy(TRANSLATE_API,a.c_str());
    }
    // Close the file
    myEnvFile.close(); 
    
};
int main (){
    //Read from .env file for auto-environment vars population
    populateEnvironmentVars();
    DWORD extendedWindowStyle = WS_EX_TOPMOST |  WS_EX_LAYERED  ;
    
    //Define Windows Class to be registered 
    WNDCLASSEXW translateOverlayClass = { };
    LPCWSTR className = L"translateOverlayClass";
    
    translateOverlayClass.cbSize = sizeof(translateOverlayClass);
    translateOverlayClass.lpfnWndProc = WndProc;
    translateOverlayClass.hInstance = GetModuleHandle(NULL); 
    translateOverlayClass.lpszClassName = className;
   // translateOverlayClass.hCursor = LoadCursor(NULL, IDC_ARROW);
   translateOverlayClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    
   //Register class as ATOM for use with Window Handler later
    RegisterClassExW(&translateOverlayClass);
    LPCWSTR windowName = L"Translate Overlay";
    DWORD windowStyle =  WS_OVERLAPPEDWINDOW;

    int X = 0,Y = 0,nWidth= 600,nHeight=800;

    //Intantialise Window and Window Handler
    windowHandler = CreateWindowExW(extendedWindowStyle,className,windowName,windowStyle,X,Y,nWidth,nHeight,NULL,NULL,NULL,NULL);
    if(windowHandler == NULL){
        DWORD errorCode = GetLastError();
        std::cerr << "CreateWindowExW failed with error: " << errorCode << std::endl;
        return(0);
    }
    windowHandlers.push_back(windowHandler);

    std::cout << "Window and related classes initialised" << "\n";
    
    //Setting initial transparency  
    COLORREF crKey = RGB(255,255,255);
    BYTE bAlpha = OPACITY;
    SetLayeredWindowAttributes(windowHandler,crKey,bAlpha,LWA_ALPHA);
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
    free(TRANSLATE_API);

    return 0;

}
