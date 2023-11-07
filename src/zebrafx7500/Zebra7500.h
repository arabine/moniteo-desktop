#ifndef ZEBRA7500_H
#define ZEBRA7500_H


#include "rfidapi.h"
#include <thread>

#include "EventLoop.h"
#include "Util.h"

class IDeviceEvent
{
public:
    virtual void TagEvent(int64_t id, uint64_t timestamp) = 0;
    virtual void Message(const std::string &message) = 0;
};

class Zebra7500
{
public:
    const int OrderCOnnect = 23;
    const int OrderStart = 60;
    const int OrderQuit = 102;

    struct Device{
        std::string name;
        std::string conn_channel;
        std::string id;
        std::string type;
        std::string conn_settings;
        std::string options;

        Device() = default;
    };
    Zebra7500(IDeviceEvent &ev);
    virtual ~Zebra7500();

    void SetConfiguration(const Device &dev);

    bool Initialize();
    void Start();
     void Stop();
    void Connect();
    void InventoryLoop();

    void CheckCapabilities();

private:
    IDeviceEvent &m_ev;
    RFID_HANDLE32 readerHandle;
    READER_CAPS readerCaps;
    std::thread mThread;
    UINT16 frequencyIndex = 0;

    Device m_dev;

    bool mInitialized = false;
};

#endif // ZEBRA7500_H
