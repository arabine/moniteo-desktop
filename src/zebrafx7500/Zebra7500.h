#ifndef ZEBRA7500_H
#define ZEBRA7500_H


#include "rfidapi.h"
#include <thread>

#include "EventLoop.h"
#include "Util.h"

class Zebra7500
{
public:
    struct Device{
        std::string name;
        std::string conn_channel;
        std::string id;
        std::string type;
        std::string conn_settings;
        std::string options;
    };
    Zebra7500();
    virtual ~Zebra7500();

    void SetConfiguration();

    bool Initialize();
    void Start();
     void Stop();
    void InventoryLoop();
    void SendToManolab(int64_t id);
    void CheckCapabilities();

private:
    RFID_HANDLE32 readerHandle;
    READER_CAPS readerCaps;
    std::thread mThread;
    UINT16 frequencyIndex = 0;

    Device dev;

    bool mInitialized = false;
};

#endif // ZEBRA7500_H
