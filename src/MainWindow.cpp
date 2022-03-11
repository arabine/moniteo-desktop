#include "MainWindow.h"
#include <filesystem>
#include <Util.h>
#include "ImGuiFileDialog.h"
#include "imgui_internal.h"
#include "Settings.h"
#include "Log.h"
#include "JsonWriter.h"
#include <boost/asio.hpp>
#include "Value.h"

#ifdef USE_WINDOWS_OS
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

static const char gDefaultAddress[] = "moniteo.io";
static const char gDefaulPort[] = "80";
static const char gDefaultReceivePath[] = "/api/v1/data/uplink";
static const char gDefaultSendPath[] = "/api/v1/data/downlink";



MainWindow::MainWindow()
    : taskList(mEngine)
    , courseWindow(mEngine)
{
    Log::EnableLog(false);

    memcpy(mBufAddress, gDefaultAddress, sizeof(gDefaultAddress));
    memcpy(mBufReceivePath, gDefaultReceivePath, sizeof(gDefaultReceivePath));
    memcpy(mBufSendPath, gDefaultSendPath, sizeof(gDefaultSendPath));
    memcpy(mBufPort, gDefaulPort, sizeof(gDefaulPort));

    octets[0] = 192;
    octets[1] = 168;
    octets[2] = 1;
    octets[3] = 100;
}

MainWindow::~MainWindow()
{
    mEngine.Quit();
}

#ifdef USE_WINDOWS_OS
bool ExecuteWindowsPing(const std::string &host)
{
    HANDLE hIcmpFile;
   unsigned long ipaddr = INADDR_NONE;
   DWORD dwRetVal = 0;
   char SendData[32] = "Data Buffer";
   LPVOID ReplyBuffer = NULL;
   DWORD ReplySize = 0;

   ipaddr = inet_addr(host.c_str());
   if (ipaddr == INADDR_NONE) {
       printf("usage: %s IP address\n", host.c_str());
       return false;
   }

   hIcmpFile = IcmpCreateFile();
   if (hIcmpFile == INVALID_HANDLE_VALUE) {
       printf("\tUnable to open handle.\n");
       printf("IcmpCreatefile returned error: %ld\n", GetLastError() );
       return false;
   }

   ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
   ReplyBuffer = (VOID*) malloc(ReplySize);
   if (ReplyBuffer == NULL) {
       printf("\tUnable to allocate memory\n");
       return false;
   }


   dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
       NULL, ReplyBuffer, ReplySize, 1000);
   if (dwRetVal != 0) {
       PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
       struct in_addr ReplyAddr;
       ReplyAddr.S_un.S_addr = pEchoReply->Address;
       printf("\tSent icmp message to %s\n", host.c_str());
       if (dwRetVal > 1) {
           printf("\tReceived %ld icmp message responses\n", dwRetVal);
           printf("\tInformation from the first response:\n");
       }
       else {
           printf("\tReceived %ld icmp message response\n", dwRetVal);
           printf("\tInformation from this response:\n");
       }
       printf("\t  Received from %s\n", inet_ntoa( ReplyAddr ) );
       printf("\t  Status = %ld\n",
           pEchoReply->Status);
       printf("\t  Roundtrip time = %ld milliseconds\n",
           pEchoReply->RoundTripTime);
   }
   else {
       printf("\tCall to IcmpSendEcho failed.\n");
       printf("\tIcmpSendEcho returned error: %ld\n", GetLastError() );
       return false;
   }
   return true;
}
#endif

bool MainWindow::ExecutePing(const std::string &host)
{
#ifdef USE_WINDOWS_OS
    return ExecuteWindowsPing(host);
#else
    int x = system("ping -c1 -s1 192.168.1.240  > /dev/null 2>&1");
    if (x==0){
        std::cout<<"success"<<std::endl;
        return true;
    }else{
        std::cout<<"failed"<<std::endl;
        return false;
    }
#endif
}

void MainWindow::SetupMainMenuBar()
{
    bool showAboutPopup = false;
    bool showParameters = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Paramètres"))
            {
                showParameters = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showAboutPopup)
    {
        ImGui::OpenPopup("AboutPopup");
    }

    if (showParameters)
    {
        ImGui::OpenPopup("Options");
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    //ImVec2 parent_pos = ImGui::GetWindowPos();
    //ImVec2 parent_size = ImGui::GetWindowSize();
    //ImVec2 center(parent_pos.x + parent_size.x * 0.5f, parent_pos.y + parent_size.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("AboutPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Moniteo");
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "Platform");
        ImGui::Text("%s", SDL_GetPlatform());
        ImGui::Text("CPU cores: %d", SDL_GetCPUCount());
        ImGui::Text("RAM: %.2f GB", SDL_GetSystemRAM() / 1024.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::SameLine(300);
        if (ImGui::Button("Close", ImVec2(120, 40)))
        {
           ImGui::CloseCurrentPopup();
        }
       ImGui::EndPopup();
    }
}

void MainWindow::Initialize()
{
    // GUI Init
    gui.Initialize();
  //  gui.ApplyTheme();

    editor.Initialize();
    imgWindow.Initialize();
    taskList.Initialize();

    mEngine.SetWorkspace(Util::ExecutablePath());

    std::function< void(int, const std::vector<Value>&) > cb = std::bind( &MainWindow::EngineEvents, this, std::placeholders::_1 , std::placeholders::_2 );
    mEngine.RegisterEventEmitter(cb);

    mSettings.ReadSettings(mEngine);
    mSettings.WriteSettings(mEngine);

    taskList.ScanWorkspace();

    LoadParams();
}


void MainWindow::ShowOptionsWindow()
{
    static int pingState = 0;

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 0.0f));
    if (ImGui::BeginPopupModal("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::PushItemWidth(-1.0f);

        ImGui::Text("Adresse du serveur");
        ImGui::SameLine();
        ImGui::InputText("##addr",  mBufAddress, sizeof(mBufAddress));

        ImGui::Text("Chemin de récupération");
        ImGui::SameLine();
        ImGui::InputText("##rec_path",  mBufReceivePath, sizeof(mBufReceivePath));

        ImGui::Text("Chemin d'envoi des données");
        ImGui::SameLine();
        ImGui::InputText("##send_path",  mBufSendPath, sizeof(mBufSendPath));

        ImGui::PushItemWidth(100);
        ImGui::Text("Port");
        ImGui::SameLine();
        ImGui::InputText("##port",  mBufPort, sizeof(mBufPort), ImGuiInputTextFlags_CharsDecimal);

        float width = 50;
        ImGui::BeginGroup();
        ImGui::PushID("Zebra7500");
        ImGui::TextUnformatted("Adresse IP Zebra7500");
        ImGui::SameLine();
        for (int i = 0; i < 4; i++) {
            ImGui::PushItemWidth(width);
            ImGui::PushID(i);
            bool invalid_octet = false;
            if (octets[i] > 255) {
                // Make values over 255 red, and when focus is lost reset it to 255.
                octets[i] = 255;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            if (octets[i] < 0) {
                // Make values below 0 yellow, and when focus is lost reset it to 0.
                octets[i] = 0;
                invalid_octet = true;
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            ImGui::InputInt("##v", &octets[i], 0, 0, ImGuiInputTextFlags_CharsDecimal);
            if (invalid_octet) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::PopItemWidth();
        }
        ImGui::PopID();
        ImGui::EndGroup();

        // Example action button and way to build the IP string
        ImGui::SameLine();
        if (ImGui::Button("Test")) {

            mPool.enqueue_work([&] {
                std::stringstream ip;
                ip << octets[0] << "." << octets[1] << "." << octets[2] << "." << octets[3];
                if (pingState == 1)
                {
                    return;
                }
                pingState = 1;
                if (ExecutePing(ip.str()))
                {
                    pingState = 2;
                }
                else
                {
                    pingState = 3;
                }
            });

        }
        ImGui::SameLine();
        if (pingState == 1)
        {
            ImGui::TextUnformatted("Ping en cours...");
        }
        else if (pingState == 2)
        {
            ImGui::TextUnformatted("Ping succès!");
        }
        else if (pingState == 3)
        {
            ImGui::TextUnformatted("Ping erreur :(");
        }
        else
        {
            ImGui::TextUnformatted("");
        }

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            mServerAddr = std::string(mBufAddress);
            mServerRecUrl = std::string(mBufReceivePath);
            mServerSndUrl = std::string(mBufSendPath);
            mServerPort = Util::FromString<int>(mBufPort);
            SaveParams();

            // On sauve la config du plugin
            std::stringstream ip;
            ip << octets[0] << "." << octets[1] << "." << octets[2] << "." << octets[3];
            mEngine.SetDeviceValue("nfc_reader", "conn_channel", Value(ip.str()));
            mEngine.SaveConfigFile();

            // On applique la configuration
            courseWindow.SetServer(mServerAddr, mServerRecUrl, mServerPort);
            tableWindow.SetServer(mServerAddr, mServerSndUrl, mServerPort);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool MainWindow::ShowQuitConfirm()
{
    bool quitRequest = false;
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
   // ImGui::SetNextWindowSize(ImVec2(200, 150));
    if (ImGui::BeginPopupModal("QuitConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Voulez-vous vraiment quitter le logiciel ?");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            quitRequest = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return quitRequest;
}

/*
auto start = std::chrono::system_clock::now();
std::default_random_engine rng(std::random_device{}());
std::uniform_real_distribution<double> dist_players(810, 900);  //(min, max)

std::uniform_real_distribution<double> dist_delay(1000, 10000);  //(min, max)

typedef std::chrono::duration<double, std::milli> duration;
*/
void MainWindow::Loop()
{
    // Main loop
    bool done = false;

    // do some work
    // record end time

   // double next = dist_delay(rng);

    while (!done)
    {
        bool aboutToClose = gui.PollEvent();

        gui.StartFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        SetupMainMenuBar();

        console.Draw("Console", nullptr);
        imgWindow.Draw("ImageWindow", nullptr);
        editor.Draw("Code Editor", nullptr);
        if (mShowTaskList)
        {
            taskList.Draw("Test list", nullptr);
        }
        tableWindow.Draw("Tableau des passages", nullptr, mEngine);
        courseWindow.Draw("Infos Course", nullptr);
        ShowOptionsWindow();

        if (aboutToClose)
        {
             ImGui::OpenPopup("QuitConfirm");
        }
        if (ShowQuitConfirm())
        {
            done = true;
        }

        gui.EndFrame();

        /*
        // -------------------- Simulation
        auto end = std::chrono::system_clock::now();
        duration diff = end - start;

        if (diff.count() >= next)
        {
            next = dist_delay(rng);

            // Joueur au hasard
            int p = dist_players(rng);
            std::vector<Value> args;
            args.push_back(std::string("{\"tag\": " + std::to_string(p) + ", \"time\": " + std::to_string(Util::CurrentTimeStamp64()) + "}"));
            tableWindow.ParseAction(args);
        }

        */

        // -------------------- Simulation END
    }







    gui.Destroy();
    mEngine.Stop();
    mEngine.Quit();
}

void MainWindow::SaveParams()
{
    JsonObject obj;

    obj.AddValue("server_address", mServerAddr);
    obj.AddValue("receive_url", mServerRecUrl);
    obj.AddValue("send_url", mServerSndUrl);
    obj.AddValue("server_port", mServerPort);

    JsonWriter::SaveToFile(obj, "moniteo.json");
}

void MainWindow::LoadParams()
{
    JsonValue json;

    if (JsonReader::ParseFile(json, "moniteo.json"))
    {
        mServerAddr = json.FindValue("server_address").GetString();
        mServerRecUrl = json.FindValue("receive_url").GetString();
        mServerSndUrl = json.FindValue("send_url").GetString();
        mServerPort = json.FindValue("server_port").GetInteger();

        std::strncpy(mBufAddress, mServerAddr.c_str(), sizeof (mBufAddress));
        std::strncpy(mBufReceivePath, mServerRecUrl.c_str(), sizeof (mBufReceivePath));
        std::strncpy(mBufSendPath, mServerSndUrl.c_str(), sizeof (mBufSendPath));

        std::string portStr = Util::ToString<int>(mServerPort);
        std::strncpy(mBufPort, portStr.c_str(), sizeof (mBufPort));

        // On applique la configuration
        courseWindow.SetServer(mServerAddr, mServerRecUrl, mServerPort);
        tableWindow.SetServer(mServerAddr, mServerSndUrl, mServerPort);
    }
}

/**
 * @brief MainWindow::EngineEvents
 *
 *  ATTENTION, POTENTIELLEMENT MULTI THREADÉ | PRÉVOIR DES MUTEX
 *
 * @param signal
 * @param args
 */
void MainWindow::EngineEvents(int signal, const std::vector<Value> &args)
{
    switch (signal)
    {

    case ProcessEngine::SIG_DELAY_1S:
        if (args.size() > 0)
        {
//            console.AddMessage("[TEST] Delay: " + std::to_string(args[0].GetInteger()));
            //            QMetaObject::invokeMethod(this, "sigDelay", Qt::QueuedConnection,
            //                                      Q_ARG(int, args[0].GetInteger()));

        }
        break;
    case ProcessEngine::SIG_TEST_NUMBER:
        if (args.size() > 1)
        {
//            console.AddMessage("[TEST] Test number: " + std::to_string(args[0].GetInteger())
//                               + " / " + std::to_string(args[1].GetInteger()));
            //            QMetaObject::invokeMethod(this, "sigTest", Qt::QueuedConnection,
            //                                      Q_ARG(int, args[0].GetInteger()),
            //                                      Q_ARG(int, args[1].GetInteger())
            //                                      );
        }
        break;
    case ProcessEngine::SIG_STEP_NUMBER:
        if (args.size() >= 2)
        {
            std::string enabled = args[1].GetBool() ? "true" : "false";
//            console.AddMessage("[TEST] Test step: " + args[0].GetString() + " " + enabled);
            //            QMetaObject::invokeMethod(this, "sigStep", Qt::QueuedConnection,
            //                                      Q_ARG(QString, .c_str()),
            //                                      Q_ARG(bool, args[1].GetBool()));
        }
        break;
    case ProcessEngine::SIG_TEST_FINISHED:
//        console.AddMessage("[TEST] Finished");
        //        QMetaObject::invokeMethod(this, "sigFinished", Qt::QueuedConnection);

        break;

    case ProcessEngine::SIG_MESSAGE:
        if (args.size() > 0)
        {
            console.AddMessage(args[0].GetString());
//            std::cout << args[0].GetString() << std::endl;
            //            QMetaObject::invokeMethod(this, "sigMessage", Qt::QueuedConnection,
            //                                      Q_ARG(QString, args[0].GetString().c_str()));
        }
        break;

    case ProcessEngine::SIG_LOADED:
    {
        //        QMetaObject::invokeMethod(this, "sigScriptLoaded", Qt::QueuedConnection);
        console.AddMessage("Script loaded");

        std::string zebraIP = mEngine.GetDeviceValue("nfc_reader", "").GetString();

        std::vector<std::string> ipOctets = Util::Split(zebraIP, ".");

        if (ipOctets.size() == 4)
        {
            octets[0] = Util::FromString<int>(ipOctets[0]);
            octets[1] = Util::FromString<int>(ipOctets[1]);
            octets[2] = Util::FromString<int>(ipOctets[2]);
            octets[3] = Util::FromString<int>(ipOctets[3]);
        }


        taskList.RefreshList();
        break;
    }
    case ProcessEngine::SIG_INPUT_TEXT:
        if (args.size() >= 2)
        {
            //            QMetaObject::invokeMethod(this, "sigInputText", Qt::QueuedConnection,
            //                                      Q_ARG(QString, args[0].GetString().c_str()),
            //                                      Q_ARG(bool, args[1].GetBool()));
        }
        break;

    case ProcessEngine::SIG_AUTO_TEST_FINISHED:
        console.AddMessage("Script finished");
        //        BuildComChannelModel(); // refresh model
        //        // THEN, update UI
        //        QMetaObject::invokeMethod(this, "sigAutoTestFinished", Qt::QueuedConnection);
        break;

    case ProcessEngine::SIG_SHOW_IMAGE:
        //        QMetaObject::invokeMethod(this, "sigShowImage", Qt::QueuedConnection,
        //                                  Q_ARG(bool, args[0].GetBool()));
        break;

    case ProcessEngine::SIG_TEST_SKIPPED:
        //        QMetaObject::invokeMethod(this, "sigTestSkipped", Qt::QueuedConnection);
        break;

    case ProcessEngine::SIG_TEST_ENDED:
//        console.AddMessage("Test ended");
        //        QMetaObject::invokeMethod(this, "sigTestEnded", Qt::QueuedConnection);
        break;

    case ProcessEngine::SIG_TEST_ERROR:
//        console.AddMessage("[TEST] Error");
        //        QMetaObject::invokeMethod(this, "sigTestError", Qt::QueuedConnection);
        break;
    case ProcessEngine::SIG_TABLE_ACTION:
        tableWindow.ParseAction(args);
        break;
    default:
        TLogWarning("Un-managed signal: " + std::to_string(signal));
        break;
    }

    //QMetaObject::invokeMethod(this, "sigRunningChanged", Qt::QueuedConnection);
}
