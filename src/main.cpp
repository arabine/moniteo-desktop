
#include "MainWindow.h"


#include <iostream>       // std::cerr
#include <exception>      // std::set_terminate
#include <cstdlib>        // std::abort
#include <csignal>

#include <iostream>


// Main code
int main(int, char**)
{

#ifdef USE_WINDOWS_OS
    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle,SW_HIDE);
#endif

    MainWindow w;
    w.Initialize();

    w.Loop();

    return 0;
}
