#include <VT_AppLib.h>
#include <string>
#include <stdio.h>
#include <unistd.h>

int main()
{
    VTAppFuncs vt_app_connect;
    string name = "testApp";
    if(!vt_app_connect.registerWithHbt("testApp", 5, 1))
        printf("failed to reg %s\r\n",name.c_str());
    else
    {
      while(1)
      {
        printf("sending hbt to monitor %s\r\n", name.c_str());
        vt_app_connect.pingHbt((char*) &name[0]);
        sleep(4);
      }
    }
    return 0;
}
