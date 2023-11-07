#include "MainWindow.h"
#include <filesystem>
#include <Util.h>


#include "ImGuiFileDialog.h"
#include "imgui_internal.h"

#include "json.hpp"
#include "Log.h"

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
    : m_zebra(*this)
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
    bool showImportDialog = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Paramètres"))
            {
                showParameters = true;
            }

            if (ImGui::MenuItem("Importer"))
            {
                showImportDialog = true;
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

    if (showImportDialog)
    {
        // open Dialog Simple
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".csv", ".", 1, nullptr, ImGuiFileDialogFlags_Modal);

    }

    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        // action if OK
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            // action

            m_db.ImportFile(filePathName);
        }

        // close
        ImGuiFileDialog::Instance()->Close();
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

    Zebra7500::Device d;
    d.name = "nfc_reader";
    d.type = "Zebra7500";
    d.conn_channel = "192.168.1.1";
    d.conn_settings = "";
    d.id = "";
    d.options = "";
    m_zebra.SetConfiguration(d);
    m_zebra.Initialize();
    m_zebra.Connect();

    LoadParams();
}

void MainWindow::TagEvent(int64_t id, uint64_t timestamp)
{
    // FIXME
}

void MainWindow::Message(const std::string &message) 
{
    console.AddMessage(message);
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

            // FIXME: save parameters
//            mEngine.SetDeviceValue("nfc_reader", "conn_channel", Value(ip.str()));
 //           mEngine.SaveConfigFile();

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

        tableWindow.Draw("Tableau des passages", nullptr);
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
}

void MainWindow::SaveParams()
{
    nlohmann::json j = nlohmann::json{
        {"server_address", mServerAddr}, 
        {"receive_url", mServerRecUrl}, 
        {"send_url", mServerSndUrl},
        {"server_address", mServerPort}
    };

    Util::StringToFile("moniteo.json", j.dump());
}

void MainWindow::LoadParams()
{
    /*
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
    }*/
}

