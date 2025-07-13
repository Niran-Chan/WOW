#include <Windows.h> //Include all the special windows types defined later on
#include <iostream>
#include <string>
#include <WinUser.h>

#define UNICODE //Force MACROs to resolve to wide string equivalent instead of legacy ANSI 
#define TRANSLATE_ID   1001
#define CLEAR_ID   1002

HWND buttonHandler;
HWND translatedTextHandler;
HWND scrollBarHandler;
HWND clearButtonHandler;
//HDC hdc;
const int MAX_TEXT_LEN = 4096;
int cnt = 0;

LRESULT WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
   //std::cout << "WndProc Called: " << uMsg << std::endl;
    SCROLLINFO si; 
    int yChar = 1; //Scrolling unit
   switch(uMsg){
    case WM_CREATE:
        {//Create Button for Translate
        buttonHandler = CreateWindowExW(BS_PUSHBUTTON | BS_NOTIFY, L"Button",L"Translate\n", WS_CHILD | WS_VISIBLE | WS_BORDER,20,20,200,25,hWnd,(HMENU)TRANSLATE_ID,NULL,NULL);
        if(buttonHandler == NULL){
            std::cout << "Error with creating buttonHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
        translatedTextHandler = CreateWindowExW(0, L"EDIT", L"Text will be translated here :)",WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,100, 100, 300, 300,hWnd, NULL, GetModuleHandle(NULL), NULL);
        //translatedTextHandler = CreateWindowExW(WS_EX_TOPMOST,L"STATIC",L"Initial Text",WS_VISIBLE | WS_CHILD,100, 100, 300, 300,hWnd,NULL,NULL,NULL);
        if(translatedTextHandler == NULL){
            std::cout << "Error with creating translatedTextHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }
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
        
   /*
        RECT rect;
      
        // Get the dimensions of the parent window's client area;
        if (!GetClientRect(translatedTextHandler, &rect))
            {
                std::cout << "Error with getting translatedTextHandler RECT dimensions " << "\n";
                std::cout << GetLastError() << "\n";
            }
        int scrollWidth = 20;
        int scrollHeight = rect.bottom - rect.top;
        int scrollX = rect.right - scrollWidth;
        int scrollY = rect.top;
        scrollBarHandler= CreateWindowExW(0,L"SCROLLBAR", NULL,WS_VISIBLE | WS_CHILD | SBS_VERT,scrollX,scrollY,scrollWidth,scrollHeight,translatedTextHandler,NULL,NULL,NULL);
        
        if(scrollBarHandler == NULL){
            std::cout << "Error with creating scrollBarHandler " << "\n";
            std::cout << GetLastError() << "\n";
        }

        //Set parameters in ScrollInfo for use later
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        si.nMin = 1;
        si.nMax = 20;
        */
        /*
        DeviceUnits = (DesignUnits/unitsPerEm) * (PointSize/72) * DeviceResolution 
        
        Reference: https://learn.microsoft.com/en-us/windows/win32/gdi/device-vs--design-units
        
        Pointsize is the font size: A font's size is specified in units called points. A point is .013837 of an inch. Following the point system devised by Pierre Simon Fournier, it is common practice to approximate a point as 1/72 inch. 
        Reference: https://learn.microsoft.com/en-us/windows/win32/gdi/font-elements

        */
       //hdc = GetDC(scrollBarHandler); //We need to get the font attributes used in the static class object
       //LPTEXTMETRIC textMetrics;
       //GetTextMetrics(hdc,textMetrics);
       //UINT textMetrics->tmHeight  
       //UINT deviceUnits;
       // si.nPage = deviceUnits;
       //si.nPage = 20;
        break;
    }
    case WM_COMMAND:
        if(HIWORD(wParam) == BN_CLICKED)
            {
                int id = LOWORD(wParam); 
                switch(id){
                    case 1001:
                    {
                        std::cout << "Button Pressed" << "\n";
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
                        SetWindowTextW(translatedTextHandler,NULL);
                        break;
                    }
            }
            }
        break;
    //Handle Scrolling

    case WM_DESTROY:
        std::cout <<"Exiting Application" << "\n";
        PostQuitMessage(0);
        return (0);
   }
   return DefWindowProcW(hWnd, uMsg, wParam, lParam); 
};

int main (){
    
    //DLL Loading
    /*
    HMODULE a= LoadLibrary(TEXT("firstDLL.dll"));
    FARPROC b= GetProcAddress(a,TEXT("myMessage"));
    b("Inside DLL!");
    if(a == NULL){
        printf("Get Process for DLL failed!\n");
    }
    else{
        printf("Address found at : %x \n",a);
    }
*/

    DWORD extendedWindowStyle = WS_EX_TOPMOST;
    
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

    int X = 0,Y = 0,nWidth= 1000,nHeight=1000;

    //Intantialise Window and Window Handler
    HWND windowHandler = CreateWindowExW(extendedWindowStyle,className,windowName,windowStyle,X,Y,nWidth,nHeight,NULL,NULL,NULL,NULL);
    
    if(windowHandler == NULL){
        DWORD errorCode = GetLastError();
        std::cerr << "CreateWindowExW failed with error: " << errorCode << std::endl;
        return(0);
    }
    std::cout << "Window and related classes initialised" << "\n";
    
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
