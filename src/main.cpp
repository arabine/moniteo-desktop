
#include "MainWindow.h"


#include <iostream>       // std::cerr
#include <exception>      // std::set_terminate
#include <cstdlib>        // std::abort
#include <csignal>

#include "TcpSocket.h"


#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <iostream>

using namespace boost::interprocess;

#ifdef USE_LINUX_OS
[[noreturn]] void signal_handler(int sig)
{
    named_mutex::remove("moniteo_mutex");
    std::fprintf(stderr, "Error: signal %d\n", sig);
    std::abort();
}

[[noreturn]] void terminate_handler()
{
    named_mutex::remove("moniteo_mutex");
    std::exception_ptr exptr = std::current_exception();
    // the only useful feature of std::exception_ptr is that it can be rethrown...
    try
    {
        std::rethrow_exception(exptr);
    }
    catch (std::exception &ex)
    {
        std::fprintf(stderr, "Terminated due to exception: %s\n", ex.what());
    }
    catch (...)
    {
        std::fprintf(stderr, "Terminated due to unknown exception\n");
    }


    std::abort();
}
#endif


class SingleProc
{
public:
    SingleProc()
    {
        if (!named_mtx.try_lock())
        {
            std::cout << "Application already started" << std::endl;
            exit(-1);
        }
    }

    ~SingleProc()
    {
        std::cout << "DELETED" << std::endl;
        named_mutex::remove("moniteo_mutex");
    }

private:
    named_mutex named_mtx{open_or_create, "moniteo_mutex"};
};

// Main code
int main(int, char**)
{
  //  named_mutex::remove("moniteo_mutex");
  //  SingleProc proc;


    tcp::TcpSocket::Initialize();

#ifdef USE_LINUX_OS
    std::set_terminate(terminate_handler);

    auto previous_handler = std::signal(SIGABRT, signal_handler);
    if (previous_handler == SIG_ERR)
    {
        std::cerr << "Setup failed\n";
        return EXIT_FAILURE;
    }
#endif

#ifdef USE_WINDOWS_OS
    HWND windowHandle = GetConsoleWindow();
    ShowWindow(windowHandle,SW_HIDE);
#endif

    MainWindow w;
    w.Initialize();

    w.Loop();

    return 0;
}
