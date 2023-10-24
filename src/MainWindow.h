#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Gui.h"
#include "ConsoleWindow.h"
#include "TableWindow.h"
#include "CourseWindow.h"
#include "Pool.h"

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    void Initialize();
    void Loop();

private:

    Gui gui;

    ConsoleWindow console;
    TableWindow tableWindow;

    CourseWindow courseWindow;

    char mBufAddress[200];
    char mBufReceivePath[200];
    char mBufSendPath[200];
    char mBufPort[10];

    int octets[4];

    thread_pool mPool;

    std::string mServerAddr;
    std::string mServerRecUrl;
    std::string mServerSndUrl;
    int mServerPort;
    bool mShowTaskList = false;

    void SaveParams();
    void LoadParams();

    void SetupMainMenuBar();
   
    void ShowOptionsWindow();
    bool ShowQuitConfirm();
    bool ExecutePing(const std::string &host);
};

#endif // MAINWINDOW_H
