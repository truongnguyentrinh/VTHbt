#include <stdint.h>
#include <VT_AppLib.h>
#include <VT-udp-utility.h>
#include <string>

static UDP_connection appConnection;
static dataPacketFiedls_u message;

//register an application with hbtmonitor
bool VTAppFuncs::registerWithHbt(string progname, int timeout, int maxretries)
{
  char bht_name[] = "HBT_MONITOR";
  //copy string to message name buffer
  strncpy(message.packetStruct.packetbuffer, (char*) &progname, sizeof(message.packetStruct.packetbuffer) - 1);
  //assign fields to structure
  message.packetStruct.timeout = timeout;
  message.packetStruct.retries = maxretries;
  message.packetStruct.msg_cmd = MsgType_RegisterClient;

  //create a connection to UDP port 5001
  appConnection.Initialize();
  appConnection.connect(bht_name, 5001);
  //send message to register application with hbt monitor
  return (appConnection.send(message.bufferArray, sizeof(message)) == SOCKET_SUCCESS);

}
//deregister an application with hbtmonitor
bool VTAppFuncs::deregisterWithHbt(string progname)
{
  strncpy(message.packetStruct.packetbuffer, (char*) &progname, sizeof(message.packetStruct.packetbuffer) - 1);
  //assign fields to structure
  message.packetStruct.timeout = 0;
  message.packetStruct.retries = 0;
  message.packetStruct.msg_cmd = MsgType_DeregisterClient;
  return (appConnection.send(message.bufferArray, sizeof(message)) == SOCKET_SUCCESS);
}
//send ping to hbtmonitor to keep the app alive
bool VTAppFuncs::pingHbt(char* progname)
{
  strncpy(message.packetStruct.packetbuffer, progname, sizeof(message.packetStruct.packetbuffer) - 1);
  //assign fields to structure
  message.packetStruct.timeout = 0;
  message.packetStruct.retries = 0;
  message.packetStruct.msg_cmd = MsgType_Heartbeat;
  return (appConnection.send(message.bufferArray, sizeof(message)) == SOCKET_SUCCESS);
}
