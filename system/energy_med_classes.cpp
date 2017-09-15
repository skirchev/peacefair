/*###############################################################################################
#       ENERGY Mediator classes                                                                #
#       file: energy_med_classes.cpp                                                           #
#       Created by: Stefan Kirchev, 	stefan.kirchev@gmail.com                                #
#                                                                                               #
#       OS: Unix like                                                                           #
#       Purpose: Classes for managing ENERGY message translations                              #
#                                                                                               #
#       History: 2017-08-18      version:   1.0 - first release                                 #
###############################################################################################*/

#include "energy_med_classes.h"
#include "sk_tools.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>  
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>

#include <fstream>
#include <ctime>
#include <iconv.h>
#include <exception>

using namespace std;

#define __ENERGY_MED_LIB_VERSION__		"1.0.2 build 2017-09-11"

extern "C" energyMed* create_energy_med() { return new energyMed(); }
extern "C" void destroy_energy_com( energyMed* object ) { delete object; }

//=====================================================================================================
//=====================================================================================================
//=====================================================================================================
void energyMed::logging(const char * func, unsigned int line, const string & text, unsigned int level)
{
	if( ! this->_enable_logging ) return;
	if( this->_debug_level == 0 ) return;
	if( level > 0 && level > this->_debug_level ) return;

	std::cout << sk::getDate() << " (" << getpid() << "):" << __FILE__ << ":" << func << ":" << line << ":" << text << std::endl;
}

//=====================================================================================================
string energyMed::getLastError()
{
	return this->_error.code == 0 ? "" : sk::itoa(this->_error.code) + ":" + this->_error.info;
}

bool energyMed::error(int code, const string &info, const string & log_info)
{
	this->_error.code = code;
	this->_error.info = info;

	logging(__func__, __LINE__, "(" + sk::itoa(this->_error.code) + ") " + this->_error.info + (log_info.empty() ? "" : "; " + log_info), 1);
	return (_success = false);
}

bool energyMed::error(const char * func, unsigned int line, unsigned int code, const string &info, const string & log_info)
{
	this->_error.code = code;
	this->_error.info = info;

	logging(__func__, __LINE__, ":" + string(func) + ":" + sk::itoa(line) + ": (" + sk::itoa(this->_error.code) + ") " + this->_error.info + (log_info.empty() ? "" : "; " + log_info), 1);
	return (_success = false);
}

void energyMed::clearError()
{
	this->_error.code = 0;
	this->_error.info.clear();
	this->_success = true;
}

bool energyMed::success()
{
	clearError();
	return this->_success;
}

//=====================================================================================================
json_object * energyMed::parseMessage(const char * msg)
{
	if(! msg)
	{
		error( __func__, __LINE__, 10001, "Emptu input" );
		return NULL;
	}

	if( ENERGY_SERIAL_MESSAGE_LENGTH < 7 )
	{
		error( __func__, __LINE__, 10002, "Bad message length" );
		return NULL;
	}

	json_object * o_json = json_object_new_object();

	switch(msg[0])
	{
		case ENERGY_SERIAL_CMD_VOLTAGE:
			json_object_object_add(o_json, "type", json_object_new_string( "command" ));
			json_object_object_add(o_json, "command", json_object_new_string( "voltage" ));
			json_object_object_add(o_json, "address", json_object_new_string( (sk::itoa((int) msg[1]) + "." + sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3]) + "." + sk::itoa((int) msg[4])).c_str() ));
			break;
		case ENERGY_SERIAL_CMD_CURRENT:
			json_object_object_add(o_json, "type", json_object_new_string( "command" ));
			json_object_object_add(o_json, "command", json_object_new_string( "current" ));
			json_object_object_add(o_json, "address", json_object_new_string( (sk::itoa((int) msg[1]) + "." + sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3]) + "." + sk::itoa((int) msg[4])).c_str() ));
			break;
		case ENERGY_SERIAL_CMD_ACT_POWER:
			json_object_object_add(o_json, "type", json_object_new_string( "command" ));
			json_object_object_add(o_json, "command", json_object_new_string( "power" ));
			json_object_object_add(o_json, "address", json_object_new_string( (sk::itoa((int) msg[1]) + "." + sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3]) + "." + sk::itoa((int) msg[4])).c_str() ));
			break;
		case ENERGY_SERIAL_CMD_ENERGY:
			json_object_object_add(o_json, "type", json_object_new_string( "command" ));
			json_object_object_add(o_json, "command", json_object_new_string( "energy" ));
			json_object_object_add(o_json, "address", json_object_new_string( (sk::itoa((int) msg[1]) + "." + sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3]) + "." + sk::itoa((int) msg[4])).c_str() ));
			break;
		case ENERGY_SERIAL_CMD_ADDRESS:
			json_object_object_add(o_json, "type", json_object_new_string( "command" ));
			json_object_object_add(o_json, "command", json_object_new_string( "address" ));
			json_object_object_add(o_json, "address", json_object_new_string( (sk::itoa((int) msg[1]) + "." + sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3]) + "." + sk::itoa((int) msg[4])).c_str() ));
			break;
		case ENERGY_SERIAL_CMD_ALARM_THR:
			json_object_object_add(o_json, "type", json_object_new_string( "command" ));
			json_object_object_add(o_json, "command", json_object_new_string( "alarm" ));
			json_object_object_add(o_json, "address", json_object_new_string( (sk::itoa((int) msg[1]) + "." + sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3]) + "." + sk::itoa((int) msg[4])).c_str() ));
			json_object_object_add(o_json, "value", json_object_new_int( (int) msg[5] ));
			break;

		case ENERGY_SERIAL_ANSWER_VOLTAGE:
			json_object_object_add(o_json, "type", json_object_new_string( "answer" ));
			json_object_object_add(o_json, "command", json_object_new_string( "voltage" ));
			json_object_object_add(o_json, "value", json_object_new_double( atof((sk::itoa((int) ((msg[1] * 0xFF) + msg[2])) + "." + sk::itoa((int) msg[3])).c_str()) ));
			json_object_object_add(o_json, "unit", json_object_new_string( "V" ));
			break;
		case ENERGY_SERIAL_ANSWER_CURRENT:
			json_object_object_add(o_json, "type", json_object_new_string( "answer" ));
			json_object_object_add(o_json, "command", json_object_new_string( "current" ));
			json_object_object_add(o_json, "value", json_object_new_double( atof((sk::itoa((int) msg[2]) + "." + sk::itoa((int) msg[3])).c_str()) ));
			json_object_object_add(o_json, "unit", json_object_new_string( "A" ));
			break;
		case ENERGY_SERIAL_ANSWER_ACT_POWER:
			json_object_object_add(o_json, "type", json_object_new_string( "answer" ));
			json_object_object_add(o_json, "command", json_object_new_string( "power" ));
			json_object_object_add(o_json, "value", json_object_new_int( (int) ((msg[1] * 0xFF) + msg[2]) ));
			json_object_object_add(o_json, "unit", json_object_new_string( "W" ));
			break;
		case ENERGY_SERIAL_ANSWER_ENERGY:
			json_object_object_add(o_json, "type", json_object_new_string( "answer" ));
			json_object_object_add(o_json, "command", json_object_new_string( "energy" ));
			json_object_object_add(o_json, "value", json_object_new_int( (int) ((msg[1] * 0xFF * 0xFF) + (msg[2] * 0xFF) + msg[3]) ));
			json_object_object_add(o_json, "unit", json_object_new_string( "Wh" ));
			break;
		case ENERGY_SERIAL_ANSWER_ADDRESS:
			json_object_object_add(o_json, "type", json_object_new_string( "answer" ));
			json_object_object_add(o_json, "command", json_object_new_string( "address" ));
			json_object_object_add(o_json, "value", json_object_new_boolean( ! (msg[1] || msg[2] || msg[3] || msg[4] || msg[5]) ));
			break;
		case ENERGY_SERIAL_ANSWER_ALARM_THR:
			json_object_object_add(o_json, "type", json_object_new_string( "answer" ));
			json_object_object_add(o_json, "command", json_object_new_string( "alarm" ));
			json_object_object_add(o_json, "value", json_object_new_boolean( ! (msg[1] || msg[2] || msg[3] || msg[4] || msg[5]) ));
			break;

		default:
			error( __func__, __LINE__, 10003, "Unknown message type" );

			json_object_put( o_json );
			return NULL;
	}

// Add date:
	time_t rawtime;
	time (&rawtime);

	json_object_object_add(o_json, "timestamp", json_object_new_string( sk::getDate( rawtime ).c_str() ));

	return o_json;
}

//=====================================================================================================
char * energyMed::buildMessage(json_object * in)
{
	if( json_object_get_type(in) != json_type_object )
	{
		error( __func__, __LINE__, 11001, "Bad json object type" );
		return NULL;
	}

	json_object * obj_val;

	if(! json_object_object_get_ex(in, "type", &obj_val) || json_object_get_type(obj_val) != json_type_string) return NULL;
	string type = json_object_get_string(obj_val);

	if(! json_object_object_get_ex(in, "command", &obj_val) || json_object_get_type(obj_val) != json_type_string) return NULL;
	string command = json_object_get_string(obj_val);

	if(! json_object_object_get_ex(in, "address", &obj_val) || json_object_get_type(obj_val) != json_type_string) return NULL;
	string address = json_object_get_string(obj_val);

	vector <string> address_parts  = sk::explode('.', address);
	if(address_parts.size() != 4)
	{
		error( __func__, __LINE__, 11002, "Bad address: `" + address + "`");
		return NULL;
	}

	if(type.compare( "command" ))
	{
		error( __func__, __LINE__, 11004, "Bad message type: `" + type + "`");
		return NULL;
	}

	int cmd = 0;
	int value = 0;

	if(command.compare( "voltage" ) == 0) cmd = ENERGY_SERIAL_CMD_VOLTAGE;
	else if(command.compare( "current" ) == 0) cmd = ENERGY_SERIAL_CMD_VOLTAGE;
	else if(command.compare( "power" ) == 0) cmd = ENERGY_SERIAL_CMD_VOLTAGE;
	else if(command.compare( "energy" ) == 0) cmd = ENERGY_SERIAL_CMD_VOLTAGE;
	else if(command.compare( "address" ) == 0) cmd = ENERGY_SERIAL_CMD_VOLTAGE;
	else if(command.compare( "alarm" ) == 0)
	{
		cmd = ENERGY_SERIAL_CMD_VOLTAGE;
		if(! json_object_object_get_ex(in, "value", &obj_val) || json_object_get_type(obj_val) != json_type_int) return NULL;
		value = json_object_get_int( obj_val );
	}else{
			error( __func__, __LINE__, 11003, "Unknown command: `" + command + "`");
			return NULL;
	}

	if(ENERGY_SERIAL_MESSAGE_LENGTH < 7) return NULL;

	char * data = new char[ENERGY_SERIAL_MESSAGE_LENGTH];
	bzero(data, ENERGY_SERIAL_MESSAGE_LENGTH);

// Set data:
	data[0] = cmd;
	for(int pos = 0; pos < 4; pos ++) data[pos + 1] = atoi(address_parts[pos].c_str());
	data[5] = value;
	data[6] = (((int) data[0]) + ((int) data[1]) + ((int) data[2]) + ((int) data[3]) + ((int) data[4]) + ((int) data[5])) & 0xFF;

	return data;
}
