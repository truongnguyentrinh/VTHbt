#ifndef VT_APP_LIB_H
#define VT_APP_LIB_H
#include <string>
using namespace std;
  class VTAppFuncs
  {
  private:
    char message_buffer[20];
  public:
    //register an application with hbtmonitor
    bool registerWithHbt(string progname, int timeout, int maxretires);
    //deregister an application with hbtmonitor
    bool deregisterWithHbt(string progname);
    //send ping to hbtmonitor to keep the app alive
    bool pingHbt(char* progname);
  };

#endif
