#include "CourseWindow.h"
#include "Gui.h"

#include "Util.h"

#include "Log.h"

CourseWindow::CourseWindow()
{
    mHttpThread = std::thread(&CourseWindow::RunHttp, this);
}

CourseWindow::~CourseWindow()
{
    HttpOrder req;
    req.cmd = "quit";
    mHttpQueue.Push(req);

    if (mHttpThread.joinable())
    {
        mHttpThread.join();
    }
}

void CourseWindow::RunHttp()
{
    bool quit = false;
    HttpOrder req;
    while(!quit)
    {
        if (mHttpQueue.TryPop(req))
        {
            if (req.cmd != "quit")
            {
                /*
                std::string response = mHttpClient.ExecuteAsync(req);

                std::cout << response << std::endl;
                JsonReader reader;
                JsonValue json;
                if (reader.ParseString(json, response))
                {
                    if (json.IsArray())
                    {
                        mTable.clear();
                        mCategories.clear();
                        for (const auto &e : json.GetArray())
                        {
                            Entry entry;

                            entry.dossard = Util::FromString<uint64_t>(e.FindValue("dossard").GetString());
                            entry.dbId = Util::FromString<uint64_t>(e.FindValue("id").GetString());
                            entry.tours = Util::FromString<uint64_t>(e.FindValue("tours").GetString());
                            entry.category = e.FindValue("F5").GetString();
                            entry.lastname = e.FindValue("F6").GetString();
                            entry.firstname = e.FindValue("F7").GetString();
                            entry.club = e.FindValue("F8").GetString();

                            // Protection en cas de catégorie invalide ou non renseignée
                            if (entry.category.size() > 0)
                            {
                                mTable[entry.dossard] = entry;
                                mCategories.insert(entry.category);
                            }
                        }

                        // Copie dans les tables internes de Manolab
                        std::vector<Value> line;
                        for (auto & c : mCategories)
                        {
                            line.push_back(c);
                        }
                        mEngine.SetTableEntry("categories", 0, line);

                        uint32_t index = 0;
                        for (const auto & e : mTable)
                        {
                            line.clear();

                            line.push_back(Value(e.second.dossard));
                            line.push_back(Value(e.second.category));
                            line.push_back(Value(e.second.tours));
                            mEngine.SetTableEntry("dossards", index, line);
                            index++;
                        }

                    }
                    else
                    {
                        TLogError("[HTTP] JSON format: not an array!");
                    }
                }
                else
                {
                    TLogError("[HTTP] Parse JSON reply error");
                }
                */
            }
            else
            {
                quit = true;
            }
        }
    }
}

bool CourseWindow::GetCourse(const std::string &host, const std::string &path, uint16_t port)
{
    bool success = false;
/*
    std::string response;
    HttpClient::Request req;

    req.action = HttpClient::Action::HTTP_GET;
    req.host = mServer;
    req.port = std::to_string(mPort);
    req.target = mPath;
    req.secured = false;

    mHttpQueue.Push(req);
*/
    return success;
}

void CourseWindow::Draw(const char *title, bool *p_open)
{
    ImGui::Begin(title, p_open);

    /* ======================  Réception de la course depuis le Cloud ====================== */

    if (ImGui::Button( "Récupérer", ImVec2(100, 40)))
    {
        GetCourse(mServer, mPath, mPort);
    }

    ImGui::Text("Participants : %d, Catégories : %d", static_cast<int>(mTable.size()), static_cast<int>(mCategories.size()));

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | 
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;

    if (ImGui::BeginTable("table1", 6, tableFlags))
    {
        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Dossard", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Catégorie", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tours", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Nom", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Prénom", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        for (const auto & e : mTable)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", std::to_string(e.second.dbId).c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", std::to_string(e.second.dossard).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", e.second.category.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", e.second.tours);

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", e.second.lastname.c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%s", e.second.firstname.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
