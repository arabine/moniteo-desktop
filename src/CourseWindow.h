#ifndef COURSE_WINDOW_H
#define COURSE_WINDOW_H


#include <vector>
#include <map>
#include <mutex>
#include <set>

#ifdef USE_WINDOWS_OS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <httplib.h>

#include "ThreadQueue.h"

#include "HttpOrder.h"

class CourseWindow
{
public:
    CourseWindow();
    ~CourseWindow();
    void Draw(const char *title, bool *p_open);
    void SetServer(const std::string &server, const std::string &path, uint16_t port)
    {
        mServer = server;
        mPath = path;
        mPort = port;
    }
private:
   struct Entry
   {
       Entry()
        : dbId(0)
        , dossard(0)
        , birthYear(1900)
       {

       }
        int64_t dbId; // database table unique ID
        int64_t dossard;
        int64_t tours; // nombre max de tours à effectuer
        std::string category;
        std::string firstname;
        std::string lastname;
        std::string gender;
        std::string club;
        uint32_t birthYear;
   };

   ThreadQueue<HttpOrder> mHttpQueue;
   std::thread mHttpThread;

   std::string mServer;
   std::string mPath;
   uint16_t mPort;

   // clé: dossard
   // 
   std::map<int64_t, Entry> mTable;

   std::set<std::string> mCategories;

   bool GetCourse(const std::string &host, const std::string &path, uint16_t port);
   void RunHttp();
};

#endif // COURSE_WINDOW_H

