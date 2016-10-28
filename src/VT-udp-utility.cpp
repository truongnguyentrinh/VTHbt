/*
  VT-udp-utility.cpp
  Author: TruongNguyen Trinh
  Description: 
    Generic UDP socket utility
    used by HBT to send msg to WDT and receive data from application
  References:
    https://www.abc.se/~m6695/udp.html
    https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c
    https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c
*/
#include <stdint.h>
#include <VT-udp-utility.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <errno.h>
#define NET_IP "127.0.0.1"
//init UDP socket on UDPHbt only
Socket_Errors_en UDP_connection::Initialize(void)
{
  //create a socket
  my_UDP_connection.socket = socket(AF_INET , SOCK_DGRAM, 0);
  if (my_UDP_connection.socket < 0)
    return SOCKET_ERROR_CREAT_FAIL;

  return SOCKET_SUCCESS;
}

//Connect to host at port
Socket_Errors_en UDP_connection::connect(char* hostname, int portID)
{
  int status = 0;
  //reset udp_addr
  memset((char*)&my_UDP_connection.udp_addr, 0, sizeof(my_UDP_connection.udp_addr));
  //setup inet address
  my_UDP_connection.udp_addr.sin_family = AF_INET;
  my_UDP_connection.udp_addr.sin_addr.s_addr = inet_addr(NET_IP);
  my_UDP_connection.udp_addr.sin_port = htons(portID);

  //binding socket to address
  status = bind(my_UDP_connection.socket, (struct sockaddr *) &my_UDP_connection.udp_addr, sizeof(struct sockaddr_in));
  //if bind() fail
  if (status == -1)
  {
    //close socket and return failure
    printf("bind failed with stat %d errno %s\r\n", status, strerror(errno));
//    close(my_UDP_connection.socket);
    return SOCKET_ERROR_BIND_FAIL;
  }

  memcpy(my_UDP_connection.hostname, hostname, sizeof(my_UDP_connection.hostname));

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
  struct sockaddr si_recv;
  int si_recv_len = sizeof(si_recv);

  *len = recvfrom(my_UDP_connection.socket, buffer, BUFF_LEN, 0, &si_recv, 0);
  //if length received is not 0, return success, else return fail
  if(*len >0)
    return SOCKET_SUCCESS;
  return SOCKET_ERROR_RECEIVE_FAIL;
}
