#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Gui.h"
#include "ConsoleWindow.h"
#include "CodeEditor.h"
#include "ProcessEngine.h"
#include "ImageWindow.h"
#include "TaskListWindow.h"
#include "Settings.h"
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
    ProcessEngine mEngine;
    Gui gui;
    ImageWindow imgWindow;
    ConsoleWindow console;
    CodeEditor editor;
    TaskListWindow taskList;
    Settings mSettings;
    TableWindow tableWindow;
    ImGuiFileDialog fileDialog;
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
    void EngineEvents(int signal, const std::vector<Value> &args);
    void ShowOptionsWindow();
    bool ShowQuitConfirm();
    bool ExecutePing(const std::string &host);
};

#endif // MAINWINDOW_H
