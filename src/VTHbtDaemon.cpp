/*
  VTHbtDaemon.cpp
  Author: TruongNguyen Trinh
  Description: 
    Main code to run the heartbeat monitor code to get msgs from app and send heartbeats to watchdog monitor
  References:
    https://www.abc.se/~m6695/udp.html
    https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c
    https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c
*/

#include <stdint.h>
#include <VT-udp-utility.h>
#include <VTHbtMonitor.h>



int main(int argc, char* argv[])
{
		// process any arguments as applicable
		// daemonize program here
		VTHbtMonitor monitor;
		monitor.initialize();
		// run forever heartbeat duties
		monitor.run();
		return 1;
}
