#ifndef TABLE_WINDOW_H
#define TABLE_WINDOW_H

#include <vector>
#include <map>
#include <mutex>
#include "Util.h"
#include "DurationTimer.h"
//#include "Pool.h"

#include "HttpOrder.h"
#include "ThreadQueue.h"
#include "IAppEvent.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <httplib.h>

class TableWindow
{
public:
    TableWindow(IAppEvent &app);
    ~TableWindow();
    void Draw(const char *title, bool *p_open);

  //  void ParseAction(const std::vector<Value> &args);

   void TagEvent(const Tag &t);

    void SetServer(const std::string &server, const std::string &path, uint16_t port)
    {
        mServer = server;
        mPath = path;
        mPort = port;
    }

private:
    struct Entry {
        int64_t tag;
        std::vector<int64_t> laps;

        std::string ToString(int64_t start) const
        {
            std::string times;
            for (const auto & l : laps)
            {
                double diff = static_cast<double>(l - start);
                diff /= 1000;
                times += std::to_string(diff) + ";";
            }
            return times;
        }

        void FromString(const std::string& times, int64_t start)
        {
            std::vector<std::string> array = Util::Split(times, ";");

            for (auto& s : array)
            {
                if (s.size() > 0)
                {
                    double t = Util::FromString<double>(s);
                    t *= 1000;
                    laps.push_back(static_cast<int64_t>(t + start));
                }
            }
        }
    };

    IAppEvent &m_app;
    
    std::map<int64_t, Entry> mTable;
    std::mutex mMutex;
    int64_t mWindow = 10000;
    int64_t mStartTime = 0;
    char buf2[10];
    std::string mServer;
    std::string mPath;
    uint16_t mPort;

    
    ThreadQueue<HttpOrder> mHttpQueue;
    std::thread mHttpThread;

    DurationTimer timer;

    std::vector<std::string> mCatLabels;
    // clé: nom catégorie
    // valeur: si autorisée ou non
    std::map<std::string, bool> mCategories;

    // clé: numéro de dossard
    // Valeur: catégorie
    std::map<int64_t, std::string> mDossards;

    bool mSendToCloud = false;
    bool mSending = false;

    // clé: numéro de dossard
    // Valeur: nombre de tours max
    std::map<int64_t, uint32_t> mToursMax;

    void RefreshWindowParameter();
    void SendToServer(const std::string &body);
    std::string ToJson(const std::map<int64_t, Entry> &table, int64_t startTime);
    void Autosave(const std::map<int64_t, Entry>& table, int64_t startTime);
    bool ShowEraseConfirm();
    void RunHttp();
};

#endif // TABLEWINDOW_H
