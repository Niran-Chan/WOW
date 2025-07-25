#include <Windows.h> //Include all the special windows types defined later on
#include <iostream>
#include <string>
#include <WinUser.h>
#include <vector>

#define UNICODE //Force MACROs to resolve to wide string equivalent instead of legacy ANSI 
#define TRANSLATE_ID   1001
#define CLEAR_ID   1002
#define OPEN_PROG_HOTKEY_ID 1


HWND windowHandler; //Main Window
HWND buttonHandler;
HWND translatedTextHandler;
HWND scrollBarHandler;
HWND clearButtonHandler;
HWND sourceTextHandler;

std::vector<HWND> windowHandlers; //Just to keep track of all windows currently present


const int MAX_TEXT_LEN = 4096;

enum OPACITY_LEVEL{
    LOW = 0,
    HIGH = 255
};

enum OPACITY_LEVEL OPACITY = LOW;

TRACKMOUSEEVENT generateTrackMouseEventObjects(HWND hWnd){
        TRACKMOUSEEVENT tme = {0};
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = hWnd;
        tme.dwHoverTime = 10;
        return tme;
}
LRESULT WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
    
    int margin = 100;
    
    int textFieldPadding = 100;
    int textFieldX = 100;
    int textFieldY = 100;
    int textFieldWidth = 300;
    int textFieldHeight = 300;




   switch(uMsg){
    case WM_CREATE:
        {//Create Button for Translate
        buttonHandler = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"Translate\n", WS_CHILD | WS_VISIBLE | WS_BORDER,20,20,200,25,hWnd,(HMENU)TRANSLATE_ID,NULL,NULL);
        if(buttonHandler == NULL){
            std::cout << "Error with creating buttonHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(buttonHandler);
        
        sourceTextHandler = CreateWindowExW(0, L"EDIT", L"Enter language to be translated here",WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL ,textFieldX, textFieldY, textFieldWidth, textFieldHeight,hWnd, NULL, GetModuleHandle(NULL), NULL);
        if(sourceTextHandler == NULL){
            std::cout << "Error with creating sourceTextHandler" << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(sourceTextHandler);
        
        //Generating space for next textfield
        margin = textFieldHeight + margin;
        textFieldX = 100;
        textFieldY = textFieldY + margin;
        textFieldWidth = 300;
        textFieldHeight = 300;

        translatedTextHandler = CreateWindowExW(0, L"EDIT", L"Text will be translated here :)",WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, textFieldX, textFieldY, textFieldWidth, textFieldHeight,hWnd, NULL, GetModuleHandle(NULL), NULL);
        //translatedTextHandler = CreateWindowExW(WS_EX_TOPMOST,L"STATIC",L"Initial Text",WS_VISIBLE | WS_CHILD,100, 100, 300, 300,hWnd,NULL,NULL,NULL);
        if(translatedTextHandler == NULL){
            std::cout << "Error with creating translatedTextHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(translatedTextHandler);

        RECT buttonRect;
         // Get the dimensions of the parent window's client area;
        if (!GetClientRect(buttonHandler, &buttonRect))
            {
                std::cout << "Error with getting  buttonHandler RECT dimensions " << "\n";
                std::cout << GetLastError() << "\n";
            }
        int padding = 100;
        clearButtonHandler = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"Clear\n", WS_CHILD | WS_VISIBLE | WS_BORDER,buttonRect.right + padding,buttonRect.bottom,buttonRect.right - buttonRect.left,buttonRect.bottom - buttonRect.top,hWnd,(HMENU)CLEAR_ID,NULL,NULL);
        if(clearButtonHandler == NULL){
            std::cout << "Error with creating clearButtonHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        windowHandlers.push_back(clearButtonHandler);
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

int main (){

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

    return 0;

}
