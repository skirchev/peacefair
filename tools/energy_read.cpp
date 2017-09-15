#include "energy_com_classes.h"
#include "energy_med_classes.h"
#include "sk_tools.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <getopt.h>
#include <math.h>

#include <iostream>
#include <algorithm>

#define APPL_VERSION "1.0.0 Build 2017-09-15"

using namespace std;

//=====================================================================================================
void usage(char * name)
{
	std::cout << "PEACEFAIR reader" << std::endl << std::endl;
	std::cout << "Usage: " << name << " [-v]" << std::endl << std::endl;
	std::cout << "Options: " << std::endl;
	std::cout << "   -t print time" << std::endl;
	std::cout << "   -h print this help and exit" << std::endl;
	std::cout << "   -c <file path> config file" << endl;
	std::cout << "   -v print version" << std::endl;
}

static bool b_time = false;

//=====================================================================================================
bool msgCallback(const char * data)
{
	if( data == NULL ) return false;

	string value;
	char str_val[4];

	for(int pos = 0;pos < ENERGY_SERIAL_MESSAGE_LENGTH; pos++)
	{
		bzero(str_val, 4);
		sprintf(str_val, "x%.2X", data[pos]);
		value += (value.empty() ? "" : " ") + string(str_val);
	}

// Printing data:
	if(b_time) std::cout << sk::getDate() << ":";
	std::cout << "Message: " << value << "; ";

	energyMed medObj;
	json_object * json = medObj.parseMessage(data);
	if(json)
	{
		std::cout << json_object_to_json_string( json );
		json_object_put( json );
	}

	std::cout << endl;

	return true;
}

//=====================================================================================================
int main(int argc, char* argv[])
{
	int option = 0;
	char conf_file[128];
	bzero(conf_file, sizeof(conf_file));

// Parse Program arguments:
	while ((option = getopt(argc, argv, "-vhtc:")) != -1)
	{
		switch(option)
		{
			case 't':
				b_time = true;
				break;
			case 'v':
				printf("Version: %s\n", APPL_VERSION);
				return 0;
			case 'c':
				strncpy(conf_file, optarg, min(strlen(optarg),sizeof(conf_file)));
				break;
			case 'h':
			default:
				usage(argv[0]);
				return 0;
		}
	}

	energySerial serialObj;

// Load custom config:
	if(strlen( conf_file )) serialObj.loadConfig( conf_file );

	serialObj.initInst();
	serialObj.setDataCallback(msgCallback);
	serialObj.runInstLoop();

	return 0 ;
}
