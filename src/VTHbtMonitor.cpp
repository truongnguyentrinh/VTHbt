/*
Class to create heartbeat monitor object
After init, hbt would run a 1 second timer clock.
everytime 1 second timer hit, a handler is called
inside handler, loop through a queue of timeout values for application and deduct 1
application queue should have hbt monitor itself on loc 0 for 30 sec. This
would give application 30 second to register during start up.
Once at least 1 application is registered, replace loc 0 with actual application queue
*/
#include <stdint.h>
#include <VTHbtMonitor.h>
#include <queue>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define HBT_WDT_PRT 5000
#define APP_HBT_PRT 5001

//data struct to hold information to a registed application
typedef struct registeredApp
{
  //time period to check for signal from application
  uint16_t timeout;
  uint16_t maxRetries;
  uint16_t currentTO;
  uint16_t currentRT;
  //application ID: a slot on application array to pair appication name with its data
  //uint16_t applicationID;
  //application name
  //string applicationName;
}registeredApp_t;

//message queue to hold messages sent to hbt by applications
typedef struct MessageQueue
{
  queue<string> msg_queue;
  pthread_mutex_t mu_queue;
  pthread_cond_t cond;
}MessageQueue_t;

map<char*, registeredApp_t> appList;
map<char*, registeredApp_t>::iterator appp_iterator;
UDP_connection VTHbt_WDT_connection;
static int received_len = 0;
static MessageQueue_t dataPacketQueue;
pthread_t tid[3];

//udp connection from hbt monitor to wdt monitor
//each application would have their own static connection object.
static UDP_connection Hbt_Wdt_udp;
static UDP_connection App_Hbt_udp;

//initialize the heartbeat monitor
void VTHbtMonitor::initialize(void)
{
  received_len = 0;
  //initilize and connect hbt_monitor to wdt_monitor
  Hbt_Wdt_udp.Initialize();
  Hbt_Wdt_udp.connect((char*) WDT_MONITOR, HBT_WDT_PRT);
  App_Hbt_udp.Initialize();
  App_Hbt_udp.connect((char*) HBT_MONITOR, APP_HBT_PRT);
}

//sends heart beat to port 5000 to watchdog monitor
static void heartBeat_wdt()
{
  dataPacketFiedls_u dataPacket;
  dataPacket.packetStruct.msg_cmd = MsgType_Heartbeat;
  Hbt_Wdt_udp.send(dataPacket.bufferArray, sizeof(dataPacket));
}

static int check_time_out()
{
  char sysCmd[20];
  //go through list of timers, reduce by 1,
  // if 0, deduct retry counter,
  //       if (0) set flag to stop wdt signal or reset_system
  //       else kill and restart app
  for(appp_iterator = appList.begin(); appp_iterator != appList.end(); appp_iterator++)
  {
    if(((appp_iterator->second.currentTO)--) == 0)
    {
      if(((appp_iterator->second.currentRT)--) == 0)
      {
           //kill the system, max retries reached
           //alternately, setup a flag to not ping wdt_monitor anymore
           strcpy(sysCmd, "sudo reboot\n");
           system(sysCmd);
           memset(sysCmd, 0, strlen(sysCmd));
      }
      else
      {
        appp_iterator->second.currentTO = appp_iterator->second.timeout;
        //send cmd to kill the app
        strcpy(sysCmd, "pkill -9 ");
        strcat(sysCmd, appp_iterator->first);
        strcat(sysCmd, "\n");
        system(sysCmd);
        //reset cmd string
        memset(sysCmd, 0, strlen(sysCmd));
        //send command to restart the app, assuming the app name is in the same directory
        //and the app name is the same with the exe name
        strcpy(sysCmd, appp_iterator->first);
        system(sysCmd);
        //reset cmd string
        memset(sysCmd, 0, strlen(sysCmd));
      }
    }
  }
  return 0;
}
//handling a second event. Whenever a second event happens, increment the second counter
static void sec_handler()
{
  printf("handling timer\r\n");
  //check for time out on application list
  if(!check_time_out());
    heartBeat_wdt(); // sends heart beat to wdt_monitor
}

//handling incoming string over port 5001
static void string_handler(string message)
{
  dataPacketFiedls_u *msg_fields;
  registeredApp_t temp;

  //pointing the char array to the address of the message string
  message.copy(msg_fields->bufferArray, sizeof(msg_fields->bufferArray) - 1, 0);

  //switch on commands:
  switch(msg_fields->packetStruct.msg_cmd)
  {
    case MsgType_RegisterClient:
      //get the fields out of the message and register the client application
      temp.timeout = msg_fields->packetStruct.timeout;
      temp.maxRetries = msg_fields->packetStruct.retries ;
      temp.currentRT = temp.maxRetries;
      temp.currentTO = temp.timeout;
      //insert app onto the map for monitoring
      appList.insert (pair<char*, registeredApp_t>(msg_fields->packetStruct.packetbuffer, temp));
      break;
    case MsgType_DeregisterClient:
      //get client ID and deregister it
      appp_iterator = appList.find(msg_fields->packetStruct.packetbuffer);
      appList.erase(appp_iterator);
      break;
    case MsgType_Heartbeat:
      //reload counter for time out and retries
      appp_iterator = appList.find(msg_fields->packetStruct.packetbuffer);
      appp_iterator->second.currentTO = appp_iterator->second.timeout;
      break;
    default:
      break;
  }
}



//function to wait for and service message on queue
static void *heartbeat_thread(void* arg)
{
  string s;
  MessageQueue_t* mq = &dataPacketQueue;
  while(1)
  {
    
    pthread_mutex_lock(&mq->mu_queue);
    if(!mq->msg_queue.empty())
    {
      printf("got msg in thread heartbeat\r\n");
       s = mq->msg_queue.front();
       mq->msg_queue.pop();
       string_handler(s);
    }
    else
    {
      printf("waiting in thread heartbeat\r\n");
      pthread_cond_wait(&mq->cond, &mq->mu_queue);
    }
    pthread_mutex_unlock(&mq->mu_queue);
  }
}

//thread to receive data packets from applications
static void *receiver_thread(void* arg)
{
  //local queue pointer pointing to the shared static queue structure declared for this file
  MessageQueue_t* incomingPk = &dataPacketQueue;
  //local data packet type to parse in data packet from messages
  string udp_data_packet_dt;
 
  //loop and try to get a message from buffer
  while(1)
  {
     printf("in thread receiver\r\n");
    //receive with known length, put into buffer
    App_Hbt_udp.receive((char*) &udp_data_packet_dt, &received_len);

    //mutex lock the shared queue
    pthread_mutex_lock(&incomingPk->mu_queue);
    //push new packet on to queue, this woula allow the hbt thread to service the packets
    incomingPk->msg_queue.push(udp_data_packet_dt);
    //mutex unlock
    pthread_mutex_unlock(&incomingPk->mu_queue);
    //signal conditional variable for the other thread to run
    pthread_cond_signal(&incomingPk->cond);
  }
}

//thread to keep track of 1 second intervals
static void *timer_thread(void* arg)
{
  struct timeval sec_t;
 
  while(1)
  {
    printf("in thread timer\r\n");
    sec_t.tv_sec = 1;
    sec_t.tv_usec = 0;
    select(0, NULL, NULL, NULL, &sec_t);
    sec_handler();
  }
}

void VTHbtMonitor::run(void)
{
  int i = 0;
  int err;
  //spawn 2 threads:
  //      - one to periodically sendbeats to WDT daemon
  //      - one to loop waiting for incoming packet

  //create first thread for receiver
  err = pthread_create(&(tid[0]), NULL, receiver_thread, (void*) NULL);
  //check for thread error
  if (err != 0)
     printf("\ncan't create thread :[%s]", strerror(err));
  else
     printf("\n receiver thread created successfully\n");

  //create second thread for heartbeat
  err = pthread_create(&(tid[1]), NULL, heartbeat_thread, (void*) NULL);
  //check for thread error
  if (err != 0)
     printf("\ncan't create thread :[%s]", strerror(err));
  else
     printf("\n hbt thread created successfully\n");

  //create thrid thread for 1 second timer
  err = pthread_create(&(tid[2]), NULL, timer_thread, (void*) NULL);
  //check for thread error
  if (err != 0)
      printf("\ncan't create thread :[%s]", strerror(err));
  else
      printf("\n 1sec timer thread created successfully\n");

  while(1)
	  sleep(1);

}
