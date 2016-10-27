#include <stdint.h>
#include <VT_AppLib.h>
#include <VT-udp-utility.h>
#include <string>
#include <iostream>
using namespace std;

static UDP_connection appConnection;

//register an application with hbtmonitor
bool VTAppFuncs::registerWithHbt(string progname, int timeout, int maxretries)
{
  char bht_name[] = "HBT_MONITOR";
  char message[20];
  int message_size;
  
  message_size = sprintf(message, "%d,%s,%d,%d\r\n", MsgType_RegisterClient, progname.c_str(), timeout, maxretries);

  //create a connection to UDP port 5001
  appConnection.Initialize();
  appConnection.connect(bht_name, 5001);
  //send message to register application with hbt monitor
  return (appConnection.send(message, message_size) == SOCKET_SUCCESS);

}
//deregister an application with hbtmonitor
bool VTAppFuncs::deregisterWithHbt(string progname)
{
  char message[20];
  int message_size;

  message_size = sprintf(message, "%d,%s\r\n",MsgType_DeregisterClient, progname.c_str());
  return (appConnection.send(message, message_size) == SOCKET_SUCCESS);
}
//send ping to hbtmonitor to keep the app alive
bool VTAppFuncs::pingHbt(char* progname)
{

  char message[20];
  int message_size;

  message_size = sprintf(message, "%d,%s\r\n",MsgType_Heartbeat, progname);
  return (appConnection.send(message, message_size) == SOCKET_SUCCESS);
}
