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
  int message_size;
  
  //construct registration packet: command, ap name,time out, retries
  message_size = sprintf(message_buffer, "%d,%s,%d,%d\r\n", MsgType_RegisterClient, progname.c_str(), timeout, maxretries);

  //create a connection to UDP port 5001
  appConnection.Initialize();
  appConnection.connect(bht_name, 5001);
  //send message to register application with hbt monitor
  return (appConnection.send(message_buffer, message_size) == SOCKET_SUCCESS);

}
//deregister an application with hbtmonitor
bool VTAppFuncs::deregisterWithHbt(string progname)
{
  int message_size;
  //construct packet to deregister an app: command, app name
  message_size = sprintf(message_buffer, "%d,%s\r\n",MsgType_DeregisterClient, progname.c_str());
  return (appConnection.send(message_buffer, message_size) == SOCKET_SUCCESS);
}
//send ping to hbtmonitor to keep the app alive
bool VTAppFuncs::pingHbt(char* progname)
{
  int message_size;
  //construct a packet to send app heartbeats: command, app name
  message_size = sprintf(message_buffer, "%d,%s\r\n",MsgType_Heartbeat, progname);
  return (appConnection.send(message_buffer, message_size) == SOCKET_SUCCESS);
}
