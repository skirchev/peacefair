#include "energy_environment.h"
#include "energy_com_classes.h"
#include "energy_med_classes.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <getopt.h>

#include "sk_tools.h"

//=====================================================================================================
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <condition_variable>
#include <mutex>

#include "daemonize.h"
//=====================================================================================================

static bool debug = false;
static bool b_daemonize = false;

static std::mutex mqtt_lock;
static std::mutex serial_lock;

#include <unistd.h>

using namespace std;
static energyMqtt mqttObj;
static energyMqtt mqttObjPub;
static energySerial serialObj;

#define APPL_VERSION	"1.0.0 build 2017-09-14"

#define DAEMON_NAME	"energy_com"
#define PIDFILE		"/var/run/energy_com.pid"
#define RUNDIR		"/tmp/"

volatile bool b_serial_listener = false;

//=====================================================================================================
bool serialListener()
{
	b_serial_listener = true;
	if( debug && ! b_daemonize ) cout << __func__ << endl;

	serialObj.runInstLoop();

	b_serial_listener = false;
	return true;
}

//=====================================================================================================
static bool ctrlCallback(const char * data)
{
	if( debug && ! b_daemonize ) cout << __func__ << endl;
	if( data == NULL || strlen(data) == 0 ) return false;

	energyMed medObj;
	json_object * json = medObj.parseMessage(data);
	if(! json) return false;

	char * msg = medObj.buildMessage(json);
	if(! msg) return false;

	serial_lock.lock();
	bool result = serialObj.instWrite( msg );
	serial_lock.unlock();

	delete msg; // Release resources
	return result;
}

//=====================================================================================================
static bool msgCallback(const char * data)
{
	if( debug && ! b_daemonize ) cout << __func__ << endl;
	if( data == NULL ) return false;

	energyMed medObj;
	json_object * json = medObj.parseMessage(data);
	if(! json) return false;

	string str = json_object_to_json_string( json );
	json_object_put( json );

// Publishing data:
	try{
		mqtt_lock.lock();
		mqttObjPub.instPublish( str.c_str(), 1 );
		mqtt_lock.unlock();
	}catch(exception &e){
		mqtt_lock.unlock();
		syslog(LOG_INFO, "Failed to publish stats: %s", e.what());
	}

	return true;
}

//=====================================================================================================
//=====================================================================================================
//=====================================================================================================
void usage(char* name)
{
	cout << "Usage: " << name << " [-hvbdc]" << endl << endl;
	cout << "Options:" << std::endl;
	cout << "     -v print version" << endl;
	cout << "     -h print this help and exit" << endl;
	cout << "     -d enable debugging" << endl;
	cout << "     -c <file path> config file" << endl;
	cout << "     -b daemonize" << endl;
}

//=====================================================================================================
int main(int argc, char* argv[])
{
	char option;
	char conf_file[1024];
	bzero(conf_file, sizeof(conf_file));

// Parse Program arguments
	while (EOF != (option = getopt(argc, argv, "-vhbdc:")) && (option != 255))
	{
		switch (option)
		{
			case 'b':
				b_daemonize = true;
				break;
			case 'd':
				debug = true;
				break;
			case 'c':
				strncpy(conf_file, optarg, min(strlen(optarg),sizeof(conf_file)));
				break;
			case 'v':
				printf("Version: %s\n", APPL_VERSION);
				exit(0);
			case 'h':
			default:
				usage(argv[0]);
				exit(0);
		}
	}

// Load custom config:
	if(strlen( conf_file ))
	{
		mqttObj.loadConfig( conf_file );
		mqttObjPub.loadConfig( conf_file );
		serialObj.loadConfig( conf_file );
	}

// Setup logging:
	setlogmask( LOG_UPTO(LOG_INFO) );
	openlog( DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER );

// Initialize serial:
	serialObj.initInst();
	serialObj.setDataCallback(msgCallback);

// Start Serial listener:
	thread thr_serial(serialListener);
	thr_serial.detach();

// Deamonize:
	if(b_daemonize) daemonize(RUNDIR, PIDFILE);

	mqttObj.setCtrlDataCallback(ctrlCallback);
	mqttObj.runInstLoop();

	if(b_daemonize) syslog (LOG_NOTICE, "terminated.");
	closelog();

	return EXIT_SUCCESS;
}

