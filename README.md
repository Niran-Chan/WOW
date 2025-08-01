# WOW (Way to Overlay Windows)
You can use this application to keep a persistent application always running on a side of your computer so that you can just copy-paste over as required, and its persistence goes on top of other EXEs as well. I originally created this to translate games, so if there are any issues where the window is simply not popping up, try playing your game in *borderless* instead of fullscreen.

## Installation
Since this will be a Windows-only project, feel free to download the compiled releases. If not, you can still build it from source.

## Build Instructions
The following dependencies are used:
1. vcpkg
2. cmake
3. libcurl

## Environment Variables
If you do not want to constantly keep updating the API key in the settings, you can save it into your .env file instead! The program auto-reads your environment variables and populates whichever that are necessary (Setting a registry key is still a WIP).
```
    TRANSLATE_API = <INSERT YOUR KEY HERE>
```
For now, this should work. However, any internal changes to the key in the **Settings** tab will **NOT CHANGE** the key-value pairs saved in your .env. Only relevant keys will be displayed in the settings tab.