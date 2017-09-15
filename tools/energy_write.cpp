#include "energy_com_classes.h"
#include "sk_tools.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <getopt.h>
#include <math.h>

#include <iostream>
#include <algorithm>

#define APPL_VERSION "1.0.0 Build 2017-09-11"

using namespace std;
//=====================================================================================================
void usage(char * name)
{
	std::cout << "PEACEFAIR commander" << std::endl << std::endl;
	std::cout << "Usage: " << name << " [-hv] [-d[d[d]] -c <command> [-a <address>] [-v <value>]" << std::endl << std::endl;
	std::cout << "Options: " << std::endl;
	std::cout << "   -d increase debug level" << std::endl;
	std::cout << "   -c command (voltage, current, power, energy, address, alarm)" << std::endl;
	std::cout << "   -a device address (IPv4 type address)" << std::endl;
	std::cout << "   -v value (integer value of desired alarm threshold)" << std::endl;
	std::cout << "   -h print this help and exit" << std::endl;
	std::cout << "   -C <file path> config file" << endl;
	std::cout << "   -V print version" << std::endl;
}

//=====================================================================================================
int main(int argc, char* argv[])
{
	int debug_level = 0;
	int cmd = 0;

	int option = 0;
	char command[32];
	char address[32];
	int value = 0;
	char conf_file[128];

	bzero(command, sizeof(command));
	bzero(address, sizeof(address));
	bzero(conf_file, sizeof(conf_file));

// Parse Program arguments:
	while ((option = getopt(argc, argv, "-dVhc:a:v:C:")) != -1)
	{
		switch(option)
		{
			case 'd':
				++ debug_level;
				break;
			case 'c':
				strncpy(command, optarg, std::min(strlen(optarg), sizeof(command) - 1));
				break;
			case 'a':
				strncpy(address, optarg, std::min(strlen(optarg), sizeof(address) - 1));
				break;
			case 'v':
				value = atoi(optarg);
				break;
			case 'C':
				strncpy(conf_file, optarg, min(strlen(optarg),sizeof(conf_file)));
				break;
			case 'V':
				printf("Version: %s\n", APPL_VERSION);
				return 0;
			case 'h':
			default:
				usage(argv[0]);
				return 0;
		}
	}

	if(debug_level > 0)
	{
		cout << "Command: " << command << endl;
		cout << "Address: " << address << endl;
		cout << "Value:   " << value << endl;
		cout << endl;
	}

	if(strcmp( command, "voltage" ) == 0) cmd = ENERGY_SERIAL_CMD_VOLTAGE;
	else if(strcmp( command, "current" ) == 0) cmd = ENERGY_SERIAL_CMD_CURRENT;
	else if(strcmp( command, "power" ) == 0) cmd = ENERGY_SERIAL_CMD_ACT_POWER;
	else if(strcmp( command, "energy" ) == 0) cmd = ENERGY_SERIAL_CMD_ENERGY;
	else if(strcmp( command, "address" ) == 0) cmd = ENERGY_SERIAL_CMD_ADDRESS;
	else if(strcmp( command, "alarm" ) == 0) cmd = ENERGY_SERIAL_CMD_ALARM_THR;
	else{
		fprintf (stdout, "Unknown command: `%s`\n", command);
		return 1;
	}

	vector <string> address_parts  = sk::explode('.', address);
	if(address_parts.size() != 4)
	{
		fprintf (stdout, "Bad address: `%s`\n", address);
		return 1;
	}

	if(value < 0 || value > 0xFF)
	{
		fprintf (stdout, "Bad value: %d\n", value);
		return 1;
	}

	if( ENERGY_SERIAL_MESSAGE_LENGTH < 7 ) return 1;

	char data[ENERGY_SERIAL_MESSAGE_LENGTH];
	bzero(data, ENERGY_SERIAL_MESSAGE_LENGTH);

// Set data:
	data[0] = cmd;
	for(int pos = 0; pos < 4; pos ++) data[pos + 1] = atoi(address_parts[pos].c_str());
	data[5] = value;
	data[6] = (((int) data[0]) + ((int) data[1]) + ((int) data[2]) + ((int) data[3]) + ((int) data[4]) + ((int) data[5])) & 0xFF;

// Write command:
	energySerial serialObj;

// Load custom config:
	if(strlen( conf_file )) serialObj.loadConfig( conf_file );

// Init instance:
	serialObj.initInst();

	if( ! serialObj.instWrite( data )) fprintf(stdout, "Failed to send command: %s\n", serialObj.getLastError().c_str());
	else fprintf(stdout, "Command sent `%s`\n", command);

	return 0 ;
}
