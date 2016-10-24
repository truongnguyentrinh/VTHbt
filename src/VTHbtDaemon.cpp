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
