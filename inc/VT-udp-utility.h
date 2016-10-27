#ifndef VT_UDP_UL_H
#define VT_UDP_UL_H
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h> //tutorial point on bind() - Unix, Linux System Call
using namespace std;
//defines
#define BUFF_LEN 12   //size of UDP buffer#
#define WDT_MONITOR "WDT_MONITOR"
#define HBT_MONITOR "HBT_MONITOR"
//enum for error code
typedef enum Socket_Errors
{
  SOCKET_SUCCESS,
  SOCKET_ERROR_CREAT_FAIL,
  SOCKET_ERROR_BIND_FAIL,
  SOCKET_ERROR_CONNECT_FAIL,
  SOCKET_ERROR_SEND_FAIL,
  SOCKET_ERROR_RECEIVE_FAIL,
}Socket_Errors_en;

typedef enum VHM_cmd
{
	MsgType_RegisterClient,
	MsgType_DeregisterClient,
	MsgType_Heartbeat,
  MsgMaxSize = 0xFF,
}VHM_cmd_enum;

//data structure
typedef struct UDP_struct
{
  struct sockaddr_in udp_addr;
  uint16_t udp_des_prt;
  int socket;
  char hostname[BUFF_LEN];
}UDP_struct_t;

class UDP_connection
{
  private:
    UDP_struct_t my_UDP_connection;
  public:
    //constructor
    //UDP_connection();
    //destructor
    //~UDP_connection();

    //init UDP socket, return 0 if success
    Socket_Errors_en Initialize();

    //Connect to host at port, return 0 if sucess
    Socket_Errors_en connect(char* hostname, int portID);

    //Send a stream to host connected at port above, return 0 if success
    Socket_Errors_en send(char* buffer, int len);

    //listen to data on port and receive, record length of packet also, return 0 if success
    Socket_Errors_en receive(char * buffer, int* len);

};
#endif /*VT_UDP_UL_H */
