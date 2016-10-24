#ifndef VT_UDP_UL_H
#define VT_UDP_UL_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> //tutorial point on bind() - Unix, Linux System Call
using namespace std;
//defines
#define WDT_UDP_ADDR 0x10101010
#define HBT_UDP_ADDR 0x01010101
#define BUFF_LEN 20   //size of UDP buffer

//enum for error code
typedef enum Socket_Errors
{
  SOCKET_SUCCESS,
  SOCKET_ERROR_CREAT_FAIL,
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
  struct UDP_Connection_t* nextUDPConnection;
  struct sockaddr_un udp_addr;
  uint16_t udp_des_prt;
  char* hostname;
  int socket;
}UDP_struct_t;


#pragma pack(1)
//data packet used for all msg types
typedef struct dataPacket
{
  VHM_cmd_enum msg_cmd;
  char packetbuffer[BUFF_LEN];
  int timeout;
  int retries;
}dataPacket_struct;
typedef union dataPacketFiedls
{
  dataPacket_struct packetStruct;
  char bufferArray[sizeof(dataPacket_struct)];
}dataPacketFiedls_u;
#pragma pack(0)

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
