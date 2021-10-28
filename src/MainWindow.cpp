#include "MainWindow.h"
#include <filesystem>
#include <Util.h>
#include "ImGuiFileDialog.h"
#include "imgui_internal.h"
#include "Settings.h"
#include "Log.h"
#include "JsonWriter.h"
#include "asio.hpp"
#include "Value.h"

static const char gDefaultAddress[] = "moniteo.io";
static const char gDefaulPort[] = "80";
static const char gDefaultReceivePath[] = "/api/v1/data/uplink";
static const char gDefaultSendPath[] = "/api/v1/data/downlink";


#include <istream>
#include <iostream>
#include <ostream>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

namespace chrono = asio::chrono;
using asio::steady_timer;

class pinger
{
public:
  pinger(asio::io_context& io_context, const char* destination)
    : resolver_(io_context), socket_(io_context, asio::ip::icmp::v4()),
      timer_(io_context), sequence_number_(0), num_replies_(0)
  {
    destination_ = *resolver_.resolve(asio::ip::icmp::v4(), destination, "").begin();

    start_send();
    start_receive();
  }

private:
  void start_send()
  {
    std::string body("\"Hello!\" from Asio ping.");

    // Create an ICMP header for an echo request.
    icmp_header echo_request;
    echo_request.type(icmp_header::echo_request);
    echo_request.code(0);
    echo_request.identifier(get_identifier());
    echo_request.sequence_number(++sequence_number_);
    compute_checksum(echo_request, body.begin(), body.end());

    // Encode the request packet.
    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << echo_request << body;

    // Send the request.
    time_sent_ = steady_timer::clock_type::now();
    socket_.send_to(request_buffer.data(), destination_);

    // Wait up to five seconds for a reply.
    num_replies_ = 0;
    timer_.expires_at(time_sent_ + chrono::seconds(5));
    timer_.async_wait(std::bind(&pinger::handle_timeout, this));
  }

  void handle_timeout()
  {
    if (num_replies_ == 0)
      std::cout << "Request timed out" << std::endl;

    // Requests must be sent no less than one second apart.
    timer_.expires_at(time_sent_ + chrono::seconds(1));
    timer_.async_wait(std::bind(&pinger::start_send, this));
  }

  void start_receive()
  {
    // Discard any data already in the buffer.
    reply_buffer_.consume(reply_buffer_.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    socket_.async_receive(reply_buffer_.prepare(65536),
        std::bind(&pinger::handle_receive, this, std::placeholders::_2));
  }

  void handle_receive(std::size_t length)
  {
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    reply_buffer_.commit(length);

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    if (is && icmp_hdr.type() == icmp_header::echo_reply
          && icmp_hdr.identifier() == get_identifier()
          && icmp_hdr.sequence_number() == sequence_number_)
    {
      // If this is the first reply, interrupt the five second timeout.
      if (num_replies_++ == 0)
        timer_.cancel();

      // Print out some information about the reply packet.
      chrono::steady_clock::time_point now = chrono::steady_clock::now();
      chrono::steady_clock::duration elapsed = now - time_sent_;
      std::cout << length - ipv4_hdr.header_length()
        << " bytes from " << ipv4_hdr.source_address()
        << ": icmp_seq=" << icmp_hdr.sequence_number()
        << ", ttl=" << ipv4_hdr.time_to_live()
        << ", time="
        << chrono::duration_cast<chrono::milliseconds>(elapsed).count()
        << std::endl;
    }

    start_receive();
  }

  static unsigned short get_identifier()
  {
#if defined(ASIO_WINDOWS)
    return static_cast<unsigned short>(::GetCurrentProcessId());
#else
    return static_cast<unsigned short>(::getpid());
#endif
  }

  asio::ip::icmp::resolver resolver_;
  asio::ip::icmp::endpoint destination_;
  asio::ip::icmp::socket socket_;
  steady_timer timer_;
  unsigned short sequence_number_;
  chrono::steady_clock::time_point time_sent_;
  asio::streambuf reply_buffer_;
  std::size_t num_replies_;
};


MainWindow::MainWindow()
    : taskList(mEngine)
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

bool MainWindow::ExecutePing(const std::string &host)
{
    int x = system("ping -c1 -s1 192.168.1.240  > /dev/null 2>&1");
    if (x==0){
        std::cout<<"success"<<std::endl;
        return true;
    }else{
        std::cout<<"failed"<<std::endl;
        return false;
    }

    /*
    try
      {

        asio::io_context io_context;
        pinger p(io_context, host.c_str());
        io_context.run();
      }
      catch (std::exception& e)
      {
        std::cerr << "Exception: " << e.what() << std::endl;
      }
      */
}

void MainWindow::SetupMainMenuBar()
{
    bool showAboutPopup = false;
    bool showParameters = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Choisir le répertoire de travail"))
            {
                fileDialog.OpenDialog("ChooseDirDlgKey", "Choisir un dossier", nullptr, ".");
            }

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

    // display
    if (fileDialog.Display("ChooseDirDlgKey"))
    {
        // action if OK
        if (fileDialog.IsOk())
        {
        //   std::string filePathName = fileDialog.GetFilePathName();
            std::string ws = fileDialog.GetCurrentPath();
            mEngine.SetWorkspace(ws);
            mSettings.WriteSettings(mEngine);
            console.AddMessage("[INFO] Workspace changed to: " + ws);
            taskList.ScanWorkspace();

          // action
        }

        // close
        fileDialog.Close();
    }
}

void MainWindow::Initialize()
{
    // GUI Init
    gui.Initialize();
    gui.ApplyTheme();

    editor.Initialize();
    imgWindow.Initialize();
    taskList.Initialize();

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
            std::stringstream ip;
            ip << octets[0] << "." << octets[1] << "." << octets[2] << "." << octets[3];

            mPool.enqueue_work([&] {
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


void MainWindow::Loop()
{
    // Main loop
    bool done = false;

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
        courseWindow.Draw("Infos Course", nullptr, mEngine);
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
            console.AddMessage("[TEST] Delay: " + std::to_string(args[0].GetInteger()));
            //            QMetaObject::invokeMethod(this, "sigDelay", Qt::QueuedConnection,
            //                                      Q_ARG(int, args[0].GetInteger()));

        }
        break;
    case ProcessEngine::SIG_TEST_NUMBER:
        if (args.size() > 1)
        {
            console.AddMessage("[TEST] Test number: " + std::to_string(args[0].GetInteger())
                               + " / " + std::to_string(args[1].GetInteger()));
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
            console.AddMessage("[TEST] Test step: " + args[0].GetString() + " " + enabled);
            //            QMetaObject::invokeMethod(this, "sigStep", Qt::QueuedConnection,
            //                                      Q_ARG(QString, .c_str()),
            //                                      Q_ARG(bool, args[1].GetBool()));
        }
        break;
    case ProcessEngine::SIG_TEST_FINISHED:
        console.AddMessage("[TEST] Finished");
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
        console.AddMessage("Test ended");
        //        QMetaObject::invokeMethod(this, "sigTestEnded", Qt::QueuedConnection);
        break;

    case ProcessEngine::SIG_TEST_ERROR:
        console.AddMessage("[TEST] Error");
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
