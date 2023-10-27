#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Gui.h"
#include "ConsoleWindow.h"
#include "TableWindow.h"
#include "CourseWindow.h"
#include "Pool.h"
#include "Zebra7500.h"
class MainWindow : public IDeviceEvent
{
public:
    MainWindow();
    ~MainWindow();

    void Initialize();
    void Loop();

    // From IDeviceEvent
    virtual void TagEvent(int64_t id, uint64_t timestamp) override;
    virtual void Message(const std::string &message) override;

private:

    Gui gui;

    Zebra7500 m_zebra;

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
