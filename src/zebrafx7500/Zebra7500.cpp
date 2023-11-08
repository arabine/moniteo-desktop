
#ifdef WIN32
#include <windows.h>
#include <commctrl.h>
#endif
 
#include "Zebra7500.h"
//#include "Zebra7500Util.h"
#include <iostream>

#include "Util.h"
#include "Log.h"
#include "ThreadQueue.h"


//MANOLAB_PLUGIN(Zebra7500, "ZebraFX7500 RFID UHF Reader", "0.1.1")


static ThreadQueue<int> mLoopQueue;

Zebra7500::Zebra7500(IAppEvent &app, IDeviceEvent &ev)
    : m_app(app), m_ev(ev)
{

}

Zebra7500::~Zebra7500()
{
    Stop();
}


void Zebra7500::SetConfiguration(const Device &dev)
{
    m_dev = dev;
}

void Zebra7500::Start()
{
    mLoopQueue.Push(Zebra7500::OrderStart);
}

void Zebra7500::Connect()
{
    mLoopQueue.Push(Zebra7500::OrderCOnnect);
}


static wchar_t hostName[260];
bool g_bUseWin32EventHandling = false; // in lunux, just the callback mechanism is supported.



#define MAX_EVENTS 12
static RFID_EVENT_TYPE gRfidEventTypes[MAX_EVENTS] =
{
    GPI_EVENT, TAG_READ_EVENT, BUFFER_FULL_EVENT, BUFFER_FULL_WARNING_EVENT,
    ANTENNA_EVENT, DISCONNECTION_EVENT,
    INVENTORY_START_EVENT, INVENTORY_STOP_EVENT, ACCESS_START_EVENT, ACCESS_STOP_EVENT, NXP_EAS_ALARM_EVENT, READER_EXCEPTION_EVENT
};

static void ZebraRfidEventCallback(RFID_HANDLE32 readerHandle, RFID_EVENT_TYPE eventType)
{
    (void) readerHandle;
    mLoopQueue.Push(eventType);
}

void HandleResult(RFID_HANDLE32 readerHandle, RFID_STATUS rfidStatus)
{
	if(rfidStatus != RFID_API_SUCCESS)
	{
		ERROR_INFO errorInfo;
		RFID_GetLastErrorInfo(readerHandle, &errorInfo);
		wprintf(L"Error: %ls\n\t%ls\n\t%ls", RFID_GetErrorDescription(rfidStatus), errorInfo.statusDesc, errorInfo.vendorMessage);
	}

}

static READER_CAPS readerCaps;
static ANTENNA_INFO g_antennaInfo;
RFID_STATUS  ConnectReader(RFID_HANDLE32 *readerHandle, wchar_t *hostName, int readerPort)
{
	RFID_STATUS rfidStatus = RFID_API_SUCCESS;

	CONNECTION_INFO connectionInfo;
	connectionInfo.version = RFID_API3_5_1;
	rfidStatus = RFID_Connect(readerHandle, hostName, readerPort, 0,&connectionInfo);
	if(RFID_API_SUCCESS == rfidStatus)
	{
		RFID_SetTraceLevel(*readerHandle, TRACE_LEVEL_OFF);
		rfidStatus = RFID_GetReaderCaps(*readerHandle, &readerCaps); 
		g_antennaInfo.length = readerCaps.numAntennas;
		g_antennaInfo.pAntennaList = new	UINT16[g_antennaInfo.length];
		for(UINT16 nIndex = 0; nIndex < g_antennaInfo.length; nIndex++)
			g_antennaInfo.pAntennaList[nIndex] = nIndex+1;
		g_antennaInfo.pAntennaOpList = NULL;
	}
	HandleResult(*readerHandle, rfidStatus);
	return rfidStatus;
}

void Zebra7500::CheckCapabilities()
{
    RFID_STATUS rfidStatus = RFID_API_SUCCESS;

    UINT16 antennaID = 0;
    UINT16 receiveSensitivityIndex;
    UINT16 transmitPowerIndex;
    UINT16 transmitFrequencyIndex;


    RFID_GetReaderCaps(readerHandle, &readerCaps);
//    wprintf(L"\nEnter ReceiveSensitivityIndex  value range 0-%d   ", readerCaps.receiveSensitivtyTable.numValues-1);

    std::string log = "ReceiveSensitivityIndex  value range 0-" + std::to_string(readerCaps.receiveSensitivtyTable.numValues-1);
    m_app.Message(log);

    rfidStatus = RFID_GetAntennaConfig(readerHandle, antennaID, &receiveSensitivityIndex,
        &transmitPowerIndex, &transmitFrequencyIndex);
    if(rfidStatus == RFID_API_SUCCESS)
    {
        log = "Config: ReceiveSensitivityIndex=" + std::to_string(receiveSensitivityIndex) +
                ", TransmitPowerIndex=" + std::to_string(transmitPowerIndex)+
                ", TransmitFrequencyIndex=" + std::to_string(transmitFrequencyIndex);

        m_app.Message(log);
//        wprintf(L"\nReceiveSensitivityIndex = %d", receiveSensitivityIndex);
//        wprintf(L"\nTransmitPowerIndex = %d", transmitPowerIndex);
//        wprintf(L"\nTransmitFrequencyIndex = %d", transmitFrequencyIndex);
    }

//    scanf("%hu", &receiveSensitivityIndex);

    if(readerCaps.hoppingEnabled)
    {
        frequencyIndex = readerCaps.freqHopInfo.numTables;
    }
    else
    {
        frequencyIndex = readerCaps.fixedFreqInfo.numFreq;
    }
}


bool Zebra7500::Initialize()
{
    if (!mInitialized)
    {
        mThread = std::thread(&Zebra7500::InventoryLoop, this);
        mInitialized = true;
    }

    return mInitialized;
}


/*

RFID_STATUS	ConfigureAntenna(RFID_HANDLE32 readerHandle)
{
    RFID_STATUS rfidStatus = RFID_API_SUCCESS;

    UINT16 antennaID;
    UINT16 receiveSensitivityIndex;
    UINT16 transmitPowerIndex;
    UINT16 transmitFrequencyIndex;

    int option = 0;

    wprintf(L"\nEnter AntennaID   ");
    scanf("%hu", &antennaID);

    wprintf(L"\n----Command Menu----");
    wprintf(L"\n1. SetConfigureAntenna");
    wprintf(L"\n2. GetConfigureAntenna");

    while(1 != scanf("%d", &option))
    {
        wprintf(L"\nEnter a valid input:");
        clean_stdin();
    }
    switch(option)
    {
    case 1:
        {
            UINT16 frequencyIndex = 0;
            READER_CAPS readerCaps;
            RFID_GetReaderCaps(readerHandle, &readerCaps);
            wprintf(L"\nEnter ReceiveSensitivityIndex  value range 0-%d   ", readerCaps.receiveSensitivtyTable.numValues-1);
            scanf("%hu", &receiveSensitivityIndex);

            if(readerCaps.hoppingEnabled)
            {
                frequencyIndex = readerCaps.freqHopInfo.numTables;
            }
            else
            {
                frequencyIndex = readerCaps.fixedFreqInfo.numFreq;
            }
            wprintf(L"\nEnter TransmitFrequencyIndex value 1-%d   ", frequencyIndex);
            scanf("%hu", &transmitFrequencyIndex);

            wprintf(L"\nEnter TransmitPowerIndex  value 0-%d   ", readerCaps.transmitPowerLevelTable.numValues-1);
            scanf("%hu", &transmitPowerIndex);

            rfidStatus = RFID_SetAntennaConfig(readerHandle, antennaID,
                receiveSensitivityIndex, transmitPowerIndex, transmitFrequencyIndex);
            if(rfidStatus == RFID_API_SUCCESS)
                wprintf(L"\n Antenna Configuration successfully set...\n");
        }
        break;
    case 2:
        rfidStatus = RFID_GetAntennaConfig(readerHandle, antennaID, &receiveSensitivityIndex,
            &transmitPowerIndex, &transmitFrequencyIndex);
        if(rfidStatus == RFID_API_SUCCESS)
        {
            wprintf(L"\nReceiveSensitivityIndex = %d", receiveSensitivityIndex);
            wprintf(L"\nTransmitPowerIndex = %d", transmitPowerIndex);
            wprintf(L"\nTransmitFrequencyIndex = %d", transmitFrequencyIndex);
        }
        break;
    }
    HandleResult(readerHandle, rfidStatus);
    return rfidStatus;
}




*/

void Zebra7500::InventoryLoop()
{
    int order = 0;
    bool loop = true;
//    bool start = false;

    TAG_DATA* pTagData = NULL;


    do
    {
        if (mLoopQueue.TryPop(order))
        {
            //
            if (order == 42)
            {
                loop = false;
            }

            else if (order == Zebra7500::OrderCOnnect)
            {

                std::string channel = m_dev.conn_channel;
                wcscpy(hostName, Util::ToWString(channel).c_str());

                RFID_STATUS rfidStatus = ConnectReader(&readerHandle, hostName, 5084);
                if(RFID_API_SUCCESS == rfidStatus)
                {
                    TAG_STORAGE_SETTINGS tagStorageSettings;

                    m_ev.Message("Connected to RFID reader success!");

                    RFID_GetTagStorageSettings(readerHandle,&tagStorageSettings);
                    tagStorageSettings.discardTagsOnInventoryStop = TRUE;
                    RFID_SetTagStorageSettings(readerHandle,&tagStorageSettings);

                    CheckCapabilities();

                    HandleResult(readerHandle, RFID_RegisterEventNotificationCallback(readerHandle, gRfidEventTypes,  MAX_EVENTS, (RfidEventCallbackFunction) ZebraRfidEventCallback, NULL, NULL));


                    pTagData = RFID_AllocateTag(readerHandle);
                    if(NULL == pTagData)
                    {
                        // Handle memory allocation failure
                        // Optimally, Tag Allocation can be done once and pointer reused till disconnection.
                        m_ev.Message("RFID_AllocateTag Failed.");
                        return;
                    }
                }
                else
                {
                    m_ev.Message("Error: failed to Connect to RFID reader");
                }

            }

            else if (order == Zebra7500::OrderStart)
            {
//                start = true;

        
                RFID_STATUS rfidStatus = RFID_API_SUCCESS;

                rfidStatus = RFID_PerformInventory(readerHandle, NULL, NULL, NULL, NULL);
                if(RFID_API_SUCCESS != rfidStatus)
                {
                    HandleResult(readerHandle, rfidStatus);
                    // FIXME: handle error
                }

            }
            else if (order == TAG_READ_EVENT)
            {
                // on ne fait rien, cela permet juste de débloquer la boucle
            }
        }

        if (RFID_API_SUCCESS == RFID_GetReadTag(readerHandle, pTagData))
        {
            // On récupère le Tag id, il doit être de 12 octets
            UINT32 epcLength =  pTagData->tagIDLength;
            char tagidBuffer[12]; // en ascii
            uint64_t tid = 0;
            if (epcLength == 12)
            {
                for (uint32_t i = 0; i < epcLength; i++)
                {
                    char b = pTagData->pTagID[i];
                    if ((b < 0x30) || (b > 0x39))
                    {
                        b = 0x20;
                    }

                    tagidBuffer[i] = b;
                }
                std::istringstream iss(tagidBuffer);
                iss >> tid;
            }

           // SendToManolab(tid);
            m_ev.TagEvent(tid, Util::CurrentTimeStamp64());
        }
    }
    while(loop);

    if(pTagData)
    {
        RFID_DeallocateTag(readerHandle, pTagData);
    }
}


void Zebra7500::Stop()
{
    if (mInitialized)
    {
        mLoopQueue.Push(42);
        if (mThread.joinable())
        {
            mThread.join();
        }

        mInitialized = false;
      //  KillEventThread();
        RFID_Disconnect(readerHandle);
    }
}
