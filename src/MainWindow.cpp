#include "MainWindow.h"
#include <filesystem>
#include <Util.h>
#include "ImGuiFileDialog.h"
#include "imgui_internal.h"
#include "Settings.h"
#include "Log.h"
#include "JsonWriter.h"

static const char gDefaultAddress[] = "moniteo.io";
static const char gDefaulPort[] = "80";
static const char gDefaultReceivePath[] = "/api/v1/data/uplink";
static const char gDefaultSendPath[] = "/api/v1/data/downlink";


MainWindow::MainWindow()
    : taskList(mEngine)
{
    Log::EnableLog(false);

    memcpy(mBufAddress, gDefaultAddress, sizeof(gDefaultAddress));
    memcpy(mBufReceivePath, gDefaultReceivePath, sizeof(gDefaultReceivePath));
    memcpy(mBufSendPath, gDefaultSendPath, sizeof(gDefaultSendPath));
    memcpy(mBufPort, gDefaulPort, sizeof(gDefaulPort));
}

MainWindow::~MainWindow()
{
    mEngine.Quit();
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
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400));
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

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            mServerAddr = std::string(mBufAddress);
            mServerRecUrl = std::string(mBufReceivePath);
            mServerSndUrl = std::string(mBufSendPath);
            mServerPort = Util::FromString<int>(mBufPort);
            SaveParams();
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
    ImGui::SetNextWindowSize(ImVec2(200, 150));
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
        mServerPort = json.FindValue("server_address").GetInteger();

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
        //        QMetaObject::invokeMethod(this, "sigScriptLoaded", Qt::QueuedConnection);
        console.AddMessage("Script loaded");
        taskList.RefreshList();
        break;

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
