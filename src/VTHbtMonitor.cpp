/*
  VTHbtMonitor.cpp
  Author: TruongNguyen Trinh
  Description: 
    Class to create heartbeat monitor object
    After init, hbt would run a 1 second timer clock.
    everytime 1 second timer hit, a handler is called
    inside handler, loop through a queue of timeout values for application and deduct 1
    application queue should have hbt monitor itself on loc 0 for 30 sec. This
    would give application 30 second to register during start up.
    Once at least 1 application is registered, replace loc 0 with actual application queue
  References:
    https://www.abc.se/~m6695/udp.html
    https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c
    https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c
*/
#include <stdint.h>
#include <VTHbtMonitor.h>
#include <queue>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <string>

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
}registeredApp_t;

//message queue to hold messages sent to hbt by applications
typedef struct MessageQueue
{
  queue<string> msg_queue;
  pthread_mutex_t mu_queue;
  pthread_cond_t cond;
}MessageQueue_t;

//map to store pair of app name and registered app structure
map<char*, registeredApp_t> appList;
//iterator to access map elements
map<char*, registeredApp_t>::iterator app_iterator;
static int received_len = 0;
static MessageQueue_t dataPacketQueue;
pthread_t tid[3];

//udp socket to wdt monitor
//each application would have their own static connection object.
static UDP_connection Hbt_Wdt_udp;
//open udp socket to apps
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
  char message[20];
  int message_size;
  string progname = "HBT_MONITOR";
  //constructing message to send heartbeat to watchdog monitor
  message_size = sprintf(message, "%d,%s\r\n", MsgType_Heartbeat, progname.c_str());
  //send message over udp
  Hbt_Wdt_udp.send(message, message_size);
}

static int check_time_out()
{
  char sysCmd[20];
  //go through list of timers, reduce by 1,
  // if 0, deduct retry counter,
  //       if (0) set flag to stop wdt signal or reset_system
  //       else kill and restart app
  for(app_iterator = appList.begin(); app_iterator != appList.end(); app_iterator++)
  {
    if(((app_iterator->second.currentTO)--) == 0)
    {
      if(((app_iterator->second.currentRT)--) == 0)
      {
           //kill the system, max retries reached
           //alternately, setup a flag to not ping wdt_monitor anymore
           strcpy(sysCmd, "sudo reboot\n");
           system(sysCmd);
           memset(sysCmd, 0, strlen(sysCmd));
      }
      else
      {
        app_iterator->second.currentTO = app_iterator->second.timeout;
        //send cmd to kill the app
        strcpy(sysCmd, "pkill -9 ");
        strcat(sysCmd, app_iterator->first);
        strcat(sysCmd, "\n");
        system(sysCmd);
        //reset cmd string
        memset(sysCmd, 0, strlen(sysCmd));
        //send command to restart the app, assuming the app name is in the same directory
        //and the app name is the same with the exe name
        strcpy(sysCmd, app_iterator->first);
        system(sysCmd);
        //reset cmd string
        memset(sysCmd, 0, strlen(sysCmd));
      }
      printf("current timeout %d\r\n", app_iterator->second.currentTO);
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

//handling incoming string on port 5001
static void string_handler(string message)
{
  registeredApp_t temp;
  char* cptr;
  char progname[20];
  int cmd;

  //create a copy of message string. this allow strtok to run properly
  char* copy = strdup(message.c_str());

  cptr = strtok(copy, ",\r\n");
  if (cptr!=NULL)
    cmd = atoi(cptr);
  else cmd = MsgType_Heartbeat;

  //switch on commands:
  switch(cmd)
  {
    //received command to register a new application
    case MsgType_RegisterClient:
      //get the name of the application
      cptr = strtok(NULL,",\r\n"); 
      if(cptr!=NULL)
        strcat(progname, cptr);
      //get the imeout value of application
      cptr = strtok(NULL,",\r\n"); 
      if(cptr!=NULL)
        temp.timeout = atoi(cptr);
      else temp.timeout = 100;
      //get max tries of application
      cptr = strtok(NULL,",\r\n"); 
      if(cptr!=NULL)      
        temp.maxRetries = atoi(cptr);
      else temp.maxRetries = 100;

      //bring the time out and maxtries into current counter to be monitored by the 1 second handler
      temp.currentRT = temp.maxRetries;
      temp.currentTO = temp.timeout;
      //insert app onto the map for monitoring
      appList.insert (pair<char*, registeredApp_t>(progname, temp));
      break;
    //received command is to deregister an application
    case MsgType_DeregisterClient:
      cptr = strtok(NULL,",\r\n"); 
      if(cptr!=NULL)
        strcat(progname, cptr);
      //get client ID and deregister it
      app_iterator = appList.find(progname);
      if(app_iterator != appList.end())
        appList.erase(app_iterator);
      break;
    //received heart beat from the application, should reload the counter from the known timeout values
    case MsgType_Heartbeat:
      cptr = strtok(NULL,",\r\n"); 
      if(cptr!=NULL)
        strcat(progname, cptr);
      //reload counter for time out
      app_iterator = appList.find(progname);
      app_iterator->second.currentTO = app_iterator->second.timeout;
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
    //try to get a mutex on the message queue
    pthread_mutex_lock(&mq->mu_queue);
    //check if there is any message needed service on queue
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
      //let thread wait for the conditional signal when there is a message on queue
      pthread_cond_wait(&mq->cond, &mq->mu_queue);
    }
    //unlock queue so other thread can use it
    pthread_mutex_unlock(&mq->mu_queue);
  }
}

//thread to receive data packets from applications
static void *receiver_thread(void* arg)
{
  //local queue pointer pointing to the shared static queue structure declared for this file
  MessageQueue_t* incomingPk = &dataPacketQueue;
  //local data packet type to parse in data packet from messages
  char udp_data_packet_dt[20];
  string msg;
 
  //loop and try to get a message from buffer
  while(1)
  {
     printf("in thread receiver\r\n");
    //receive with known length, put into buffer
    App_Hbt_udp.receive(udp_data_packet_dt, &received_len);
    msg = udp_data_packet_dt;
    //mutex lock the shared queue
    pthread_mutex_lock(&incomingPk->mu_queue);
    //push new packet on to queue, this woula allow the hbt thread to service the packets
    incomingPk->msg_queue.push(msg);
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
    //populate fields of timer
    sec_t.tv_sec = 1;
    sec_t.tv_usec = 0;
    //wait for the 1 second timer clock before call to handle 1 second event
    select(0, NULL, NULL, NULL, &sec_t);
    sec_handler();
  }
}

void VTHbtMonitor::run(void)
{
  int i = 0;
  int err;
  //spawn 3 threads:
  //      - one to check if there is packet needed to be processed
  //      - one to loop waiting for incoming packet
  //      - one to run a 1 second timer and handle the 1 second event properly

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

  //sleep forever main thread
  while(1)
	  sleep(1);

}
