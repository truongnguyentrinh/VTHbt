/*
Generic UDP socket utility
used by HBT to send msg to WDT and receive data from application
*/
#include <stdint.h>
#include <VT-udp-utility.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> //tutorial point on bind() - Unix, Linux System Call

//init UDP socket on UDPHbt only
Socket_Errors_en UDP_connection::Initialize(void)
{
  //create a socket
  my_UDP_connection.socket = socket(AF_UNIX , SOCK_DGRAM, 0);
  if (my_UDP_connection.socket < 0)
    return SOCKET_ERROR_CREAT_FAIL;

  return SOCKET_SUCCESS;
}

//Connect to host at port
//UDP is connectionless, so this step need to assign hostname, destination port ID to the UDP structure, and bind socket to name of host
Socket_Errors_en UDP_connection::connect(char* hostname, int portID)
{
  int status = 0;
  //bind host name to socket
  my_UDP_connection.udp_addr.sun_family = AF_UNIX;
  my_UDP_connection.udp_addr.sun_port = portID;
  strncpy(my_UDP_connection.udp_addr.sun_path, hostname, sizeof(my_UDP_connection.udp_addr.sun_path) - 1);

  //binding socket to address
  status = bind(my_UDP_connection.socket, (struct sockaddr *) &my_UDP_connection.udp_addr, sizeof(struct sockaddr_un));
  //if bind() fail
  if (status == -1)
  {
    //close socket and return failure
    close(UDP_connection_lc.socket)
    return SOCKET_ERROR_BIND_FAIL;
  }
  
  //attemp to connect to socket
  status = connect(my_UDP_connection.socket, (struct sockaddr *) &my_UDP_connection.udp_addr, sizeof(struct sockaddr_un));
  if (status == -1)
  {
    //close socket and return failure
    close(UDP_connection_lc.socket)
    return SOCKET_ERROR_CONNECT_FAIL;
  }
  return SOCKET_SUCCESS;
}

//Send a stream to host connected at port above
Socket_Errors_en UDP_connection::send(char* buffer, int len)
{
  if(sendto(my_UDP_connection.socket, buffer, len, 0, (struct sockaddr *) &(my_UDP_connection.udp_addr), sizeof(my_UDP_connection.udp_addr)) < 0)
    return SOCKET_ERROR_SEND_FAIL;
  return SOCKET_SUCCESS;
}

//listen to data on port and receive, record length of packet
//we do not need to send reply to sender, so src and src lengh are both 0
//buffer size for data is 1024
Socket_Errors_en UDP_connection::receive(char * buffer, int* len)
{


  *len = recvfrom(my_UDP_connection.socket, buffer, BUFF_LEN, 0, 0, 0);
  //if length received is not 0, return success, else return fail
  if(*len >0)
    return SOCKET_SUCCESS;
  return SOCKET_ERROR_RECEIVE_FAIL;
}
