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
#include <fstream>
#include <exception>
#include <libconfig.h++>

#include "daemonize.h"
//=====================================================================================================

#define APPL_VERSION "1.0.0 Build 2017-09-15"

#define DAEMON_NAME	"energy_dump"
#define PIDFILE		"/var/run/energy_dump.pid"
#define RUNDIR		"/tmp/"

using namespace std;
using namespace libconfig;

//=====================================================================================================
void usage(char * name)
{
	std::cout << "PEACEFAIR Dumper" << std::endl << std::endl;
	std::cout << "Usage: " << name << " [-v]" << std::endl << std::endl;
	std::cout << "Options: " << std::endl;
	std::cout << "   -t print time" << std::endl;
	std::cout << "   -r print raw data" << std::endl;
	std::cout << "   -h print this help and exit" << std::endl;
	std::cout << "   -c <file path> config file" << std::endl;
	std::cout << "   -o <output dir> directory to save output files to" << std::endl;
	std::cout << "   -b daemonize" << std::endl;
	std::cout << "   -v print version" << std::endl;
}

static struct dumpCfg {
	string	output_dir	= "/tmp";
	bool	dump_time = false;
	bool	dump_raw = false;
}_dumpCfg;

//=====================================================================================================
bool loadConfig(string fileName)
{
	Config cfg;

	try
	{
		cfg.readFile(fileName.c_str());
	} catch(const FileIOException &fioex) {
		return false;
	} catch(const ParseException &pex) {
		return false;
	}

	Setting &root = cfg.getRoot();
	if(root.exists("energy_dump"))
	{
		Setting & energy_dump = root["energy_dump"];

		if(energy_dump.exists("dump_time"))		energy_dump.lookupValue("dump_time", _dumpCfg.dump_time);
		if(energy_dump.exists("dump_raw_data"))	energy_dump.lookupValue("dump_raw_data", _dumpCfg.dump_raw);
		if(energy_dump.exists("output_dir"))	energy_dump.lookupValue("output_dir", _dumpCfg.output_dir);
	}

	return true;
}

//=====================================================================================================
static string getJsonValue(json_object * in, const std::string & param)
{
	json_object * obj_val;

	if(! json_object_object_get_ex(in, param.c_str(), &obj_val)) return "";
	return json_object_get_string(obj_val);
}

//=====================================================================================================
bool msgCallback(const char * data)
{
	if( data == NULL ) return false;

	string hex_val;
	char str_val[3];

	for(int pos = 0;pos < ENERGY_SERIAL_MESSAGE_LENGTH; pos++)
	{
		bzero(str_val, 3);
		sprintf(str_val, "%.2X", data[pos]);
		hex_val += string(str_val);
	}

// Prepare data:
	energyMed medObj;
	string type, command, value, unit, timestamp;

	json_object * json = medObj.parseMessage(data);
	if(json)
	{
		type		= getJsonValue(json, "type");
		command		= getJsonValue(json, "command");
		value		= getJsonValue(json, "value");
		unit		= getJsonValue(json, "unit");
		timestamp	= getJsonValue(json, "timestamp");

		json_object_put( json );
	}

// Write line to file:
	string file_path = string(_dumpCfg.output_dir.length() ? _dumpCfg.output_dir + "/" : "") + "energy." + sk::getDate("%Y%m%d") + ".csv";

	ifstream test_file(file_path.c_str()); // Test if file is newely opened!
	ofstream dump_file( file_path.c_str(), std::ios_base::out | std::ios_base::app );
	if( dump_file )
	{
		if(! test_file.good())
		{
			if( _dumpCfg.dump_time ) dump_file << "time,";
			if( _dumpCfg.dump_raw ) dump_file << "data,";
			dump_file << "type,command,value,unit,timestamp" << std::endl;
		}

		if( _dumpCfg.dump_time ) dump_file << sk::getDate() << ",";
		if( _dumpCfg.dump_raw ) dump_file << hex_val << ",";
		dump_file << type << "," << command << "," << value << "," << unit << "," << timestamp << std::endl;
	}

	return true;
}

//=====================================================================================================
int main(int argc, char* argv[])
{
	int option = 0;
	char conf_file[1024];
	bzero(conf_file, sizeof(conf_file));
	char out_dir[1024];
	bzero(out_dir, sizeof(out_dir));
	bool b_time = false;
	bool b_raw = false;
	bool b_daemonize = false;

// Parse Program arguments:
	while ((option = getopt(argc, argv, "-vbhtrc:o:")) != -1)
	{
		switch(option)
		{
			case 't':
				b_time = true;
				break;
			case 'r':
				b_raw = true;
				break;
			case 'v':
				printf("Version: %s\n", APPL_VERSION);
				return 0;
			case 'c':
				strncpy(conf_file, optarg, min(strlen(optarg),sizeof(conf_file)));
				break;
			case 'o':
				strncpy(out_dir, optarg, min(strlen(optarg),sizeof(out_dir)));
				break;
			case 'b':
				b_daemonize = true;
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
	loadConfig(strlen( conf_file ) ? conf_file : ENERGY_CONFIG_FILE_PATH);

// Overwrite config data:
	if(strlen( out_dir ))	_dumpCfg.output_dir = string(out_dir);
	if(b_time)				_dumpCfg.dump_time = b_time;
	if(b_raw)				_dumpCfg.dump_raw = b_raw;

// Setup logging:
	setlogmask( LOG_UPTO(LOG_INFO) );
	openlog( DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER );

// Deamonize:
	if(b_daemonize) daemonize(RUNDIR, PIDFILE);

	serialObj.initInst();
	serialObj.setDataCallback(msgCallback);
	serialObj.runInstLoop();

	if(b_daemonize) syslog (LOG_NOTICE, "terminated.");
	closelog();

	return EXIT_SUCCESS;
}
