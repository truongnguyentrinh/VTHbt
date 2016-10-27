#include <VT_AppLib.h>
#include <string>
#include <stdio.h>
#include <unistd.h>

int main()
{
    VTAppFuncs vt_app_connect;
    if(!vt_app_connect.registerWithHbt("testApp", 5, 1))
        printf("failed to reg\r\n");
    while(1)
    {
        printf("sending hbt to monitor\r\n");
        vt_app_connect.pingHbt((char*) "testApp");
        sleep(4);
    }
    return 0;
}
