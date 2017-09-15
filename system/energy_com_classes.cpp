/*###############################################################################################
#       ENERGY Communicator classes                                                            #
#       file: energy_com_classes.cpp                                                           #
#       Created by: Stefan Kirchev, 	stefan.kirchev@gmail.com                                #
#                                                                                               #
#       OS: Unix like                                                                           #
#       Purpose: Classes for managing ENERGY communication                                     #
#                                                                                               #
#       History: 2017-08-18      version:   1.0 - first release                                 #
###############################################################################################*/

#include "energy_com_classes.h"
#include "sk_tools.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>  
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <libconfig.h++>

#include <fstream>
#include <ctime>
#include <iconv.h>
#include <exception>

#include <wiringPi.h>
#include <wiringSerial.h>

using namespace std;
using namespace libconfig;

#define __ENERGY_COM_LIB_VERSION__		"1.0.0 build 2017-08-18"
#define __ENERGY_SERIAL_LIB_VERSION__	"1.0.0 build 2017-09-11"
#define __ENERGY_MQTT_LIB_VERSION__		"1.0.0 build 2017-08-18"

extern "C" energyCom* create_energy_com() { return new energyCom(); }
extern "C" void destroy_energy_com( energyCom* object ) { delete object; }

extern "C" energySerial* create_energy_serial() { return new energySerial(); }
extern "C" void destroy_energy_serial( energySerial* object ) { delete object; }

extern "C" energyMqtt* create_energy_mqtt() { return new energyMqtt(); }
extern "C" void destroy_energy_mqtt( energyMqtt* object ) { delete object; }

//=====================================================================================================
//=====================================================================================================
//=====================================================================================================
void energyCom::logging(const char * func, unsigned int line, const string & text, unsigned int level)
{
	if( ! this->_enable_logging ) return;
	if( this->_debug_level == 0 ) return;
	if( level > 0 && level > this->_debug_level ) return;

	ofstream log_file( this->_log_file.c_str(), std::ios_base::out | std::ios_base::app );
	if( log_file ) log_file << sk::getDate() << " (" << getpid() << "/" << this << "):" << __FILE__ << ":" << func << ":" << line << ":" << text << std::endl;
}

//=====================================================================================================
string energyCom::getVersion() { return __ENERGY_COM_LIB_VERSION__; }

//=====================================================================================================
string energyCom::getLastError()
{
	return this->_error.code == 0 ? "" : sk::itoa(this->_error.code) + ":" + this->_error.info;
}

bool energyCom::error(int code, const string &info, const string & log_info)
{
	this->_error.code = code;
	this->_error.info = info;

	logging(__func__, __LINE__, "(" + sk::itoa(this->_error.code) + ") " + this->_error.info + (log_info.empty() ? "" : "; " + log_info), 1);
	return (_success = false);
}

bool energyCom::error(const char * func, unsigned int line, unsigned int code, const string &info, const string & log_info)
{
	this->_error.code = code;
	this->_error.info = info;

	logging(__func__, __LINE__, ":" + string(func) + ":" + sk::itoa(line) + ": (" + sk::itoa(this->_error.code) + ") " + this->_error.info + (log_info.empty() ? "" : "; " + log_info), 1);
	return (_success = false);
}

void energyCom::clearError()
{
	this->_error.code = 0;
	this->_error.info.clear();
	this->_success = true;
}

bool energyCom::success()
{
	clearError();
	return this->_success;
}

//=====================================================================================================
//=====================================================================================================
//=====================================================================================================
// Serial
//=====================================================================================================

//=====================================================================================================
energySerial::energySerial() : energyCom()
{
	loadConfig( this->_conf_file );
}

//=====================================================================================================
energySerial::energySerial(const string & device, unsigned int baud_rate) : energyCom()
{
	loadConfig( this->_conf_file );

	this->_serialCon.device = device;
	this->_serialCon.baud_rate = baud_rate;
}

energySerial::~energySerial() {}

//=====================================================================================================
string energySerial::getVersion() { return __ENERGY_SERIAL_LIB_VERSION__; }

//=====================================================================================================
bool energySerial::loadConfig(string fileName)
{
	Config cfg;

	try
	{
		cfg.readFile(fileName.c_str());
	} catch(const FileIOException &fioex) {
		logging( __func__, __LINE__, "I/O error while reading file " + fileName + ".");

		return false;
	} catch(const ParseException &pex) {
		logging( __func__, __LINE__, "Parse error at " + string(pex.getFile()) + ":" + sk::itoa(pex.getLine()) + " - " + pex.getError());

		return false;
	}

	Setting &root = cfg.getRoot();
	if(root.exists("energy_serial"))
	{
		Setting & energy_serial = root["energy_serial"];

		if(energy_serial.exists("enable_logging"))	energy_serial.lookupValue("enable_logging", _enable_logging);
		if(energy_serial.exists("log_file"))		energy_serial.lookupValue("log_file", _log_file);
		if(energy_serial.exists("debug_level"))		energy_serial.lookupValue("debug_level", _debug_level);

		if(energy_serial.exists("device"))			energy_serial.lookupValue("device", _serialCon.device);
		if(energy_serial.exists("baud_rate"))		energy_serial.lookupValue("baud_rate", _serialCon.baud_rate);
	}

	logging( __func__, __LINE__, "Loaded config file: " + fileName);
	return true;
}

//=====================================================================================================
bool energySerial::initInst()
{
	if( (this->_fd = serialOpen(this->_serialCon.device.c_str(), this->_serialCon.baud_rate)) < 0 ) return error( __func__, __LINE__, 55555, "Unable to open serial device", strerror(errno));

	if( wiringPiSetup () == -1 )
	{
		serialClose(this->_fd);
		this->_fd = -1;

		return error( __func__, __LINE__, 55555, "Unable to start wiringPi", strerror(errno));
	}

	_initialized = true;
	logging( __func__, __LINE__, "Initialized " + this->_serialCon.device + " with baud rate of " + sk::itoa(this->_serialCon.baud_rate));

	return success();
}

//=====================================================================================================
bool energySerial::clearInst()
{
	if( ! _initialized ) return error( __func__, __LINE__, 55555, "Not initialized yet");
	if( this->_fd < 0 ) return error( __func__, __LINE__, 55555, "Bad file descriptor");

	serialClose( this->_fd );

	_initialized = false;
	return success();
}

//=====================================================================================================
bool energySerial::runInstLoop()
{
	if( ! _initialized ) return error( __func__, __LINE__, 55555, "Not initialized yet");
	if( this->_fd < 0 ) return error( __func__, __LINE__, 55555, "Bad file descriptor");
	logging( __func__, __LINE__, " Listening ...");

	char data[ENERGY_SERIAL_MESSAGE_LENGTH];
	bzero(data, ENERGY_SERIAL_MESSAGE_LENGTH);
	int pos = 0;

	while( ! this->_exit_flag )
	{
		while( serialDataAvail(this->_fd) && (! this->_exit_flag) )
		{
			char value = serialGetchar(this->_fd);
			switch(pos)
			{
				case 0:
					if( value == ENERGY_SERIAL_ANSWER_VOLTAGE || value == ENERGY_SERIAL_ANSWER_CURRENT ||
						value == ENERGY_SERIAL_ANSWER_ACT_POWER || value == ENERGY_SERIAL_ANSWER_ENERGY ||
						value == ENERGY_SERIAL_ANSWER_ADDRESS || value == ENERGY_SERIAL_ANSWER_ALARM_THR )
					{
						data[pos] = value;
						pos ++;
					}else{
						char str_val[3];
						bzero(str_val, 3);
						sprintf(str_val, "%.2X", value);

						logging( __func__, __LINE__, "Bad value found at zero position: " + string(str_val), 2);
					}
					break;

				case ENERGY_SERIAL_MESSAGE_LENGTH - 1:
					data[pos] = value;
					onMessage(data);

					pos = 0;
					bzero(data, ENERGY_SERIAL_MESSAGE_LENGTH);
					break;

				default:
					data[pos] = value;
					pos ++;
			}
		}

		delay (3);
	}

	return success();
}

//=====================================================================================================
char energySerial::calculateCumulativeSum(const char * data)
{
	int sum = 0;
	for(int pos = 0;pos < ENERGY_SERIAL_MESSAGE_LENGTH - 1; pos++) sum += data[pos];

	return ((char) (sum & 0xFF));
}

//=====================================================================================================
void energySerial::onMessage(const char * data)
{

// Process data:
	if( data == NULL ) return;
	if( _enable_logging )
	{
		string value;
		char str_val[4];

		for(int pos = 0;pos < ENERGY_SERIAL_MESSAGE_LENGTH; pos++)
		{
			bzero(str_val, 4);
			sprintf(str_val, "x%.2X", data[pos]);
			value += (value.empty() ? "" : " ") + string(str_val);
		}

		logging( __func__, __LINE__, "Received message: `" + value + "`", 2);
	}

	if( _on_data_callback == NULL ) return;

// Validate data:
	char crc = calculateCumulativeSum(data);
	if( data[ENERGY_SERIAL_MESSAGE_LENGTH - 1] != crc )
	{
		char str_val[3];
		bzero(str_val, 3);
		sprintf(str_val, "%.2X", data[ENERGY_SERIAL_MESSAGE_LENGTH - 1]);

		char str_crc[3];
		bzero(str_crc, 3);
		sprintf(str_crc, "%.2X", crc);

		logging( __func__, __LINE__, "Bad CRC: `" + string(str_val) + "` vs `" + string(str_crc) + "`");
	}

// Callback:
	bool result = _on_data_callback(data);
	logging( __func__, __LINE__, "Exec callback: " + string(result ? "TRUE" : "FALSE"), 2);
}

//=====================================================================================================
bool energySerial::instWrite(const char * data)
{
	if( data == NULL ) return error( __func__, __LINE__, 55555, "Empty data");

	vector <char> v_data;
	for(size_t i = 0; i < ENERGY_SERIAL_MESSAGE_LENGTH; i++) v_data.push_back( data[i] );

	return instWrite( v_data );
}

//=====================================================================================================
bool energySerial::instWrite(vector <char> data)
{
	if( ! _initialized ) return error( __func__, __LINE__, 55555, "Not initialized yet");
	if( this->_fd < 0 ) return error( __func__, __LINE__, 55555, "Bad file descriptor");

	for(char val: data) serialPutchar( this->_fd, val);

	if( _enable_logging )
	{

		string text;
		char str_val[4];

		for(char val: data)
		{
			bzero(str_val, 4);
			sprintf(str_val, "x%.2X", val);
			text += (text.empty() ? "" : " ") + string(str_val);
		}
		
		logging( __func__, __LINE__, "Sent " + sk::itoa( data.size() ) + " chars" + string(_debug_level > 3 ? ": `" + text + "`" : ""));
	}

	return success();
}

//=====================================================================================================
//=====================================================================================================
//=====================================================================================================
// MQTT
//=====================================================================================================
energyMqtt::energyMqtt() : energyCom()
{
	_mqttMsg.instObj	= this;

	loadConfig( this->_conf_file );

	mosquitto_lib_init(); // First initialization
	_mqttMsg.m = mosquitto_new(NULL, true, (void *) &_mqttMsg);

	logging( __func__, __LINE__, "Created;", 2);
}

//=====================================================================================================
energyMqtt::~energyMqtt()
{
	if(isConnected()) instDisconnect();
	if(isInitialized()) mosquitto_destroy(_mqttMsg.m);

	mosquitto_lib_cleanup();
}

//=====================================================================================================
string energyMqtt::getVersion() { return __ENERGY_MQTT_LIB_VERSION__; }

//=====================================================================================================
bool energyMqtt::loadConfig(string fileName)
{
	Config cfg;

	try
	{
		cfg.readFile(fileName.c_str());
	} catch(const FileIOException &fioex) {
		logging( __func__, __LINE__, "I/O error while reading file " + fileName + ".");

		return false;
	} catch(const ParseException &pex) {
		logging( __func__, __LINE__, "Parse error at " + string(pex.getFile()) + ":" + sk::itoa(pex.getLine()) + " - " + pex.getError());

		return false;
	}

	Setting &root = cfg.getRoot();
	if(root.exists("energy_mqtt"))
	{
		Setting &energy_mqtt = root["energy_mqtt"];

		if(energy_mqtt.exists("enable_logging"))		energy_mqtt.lookupValue("enable_logging", _enable_logging);
		if(energy_mqtt.exists("log_file"))				energy_mqtt.lookupValue("log_file", _log_file);
		if(energy_mqtt.exists("debug_level"))			energy_mqtt.lookupValue("debug_level", _debug_level);

		if(energy_mqtt.exists("host"))					energy_mqtt.lookupValue("host", _mqttCon.host);
		if(energy_mqtt.exists("port"))					energy_mqtt.lookupValue("port", _mqttCon.port);
		if(energy_mqtt.exists("username"))				energy_mqtt.lookupValue("username", _mqttCon.username);
		if(energy_mqtt.exists("password"))				energy_mqtt.lookupValue("password", _mqttCon.password);
		if(energy_mqtt.exists("keepalive_time"))		energy_mqtt.lookupValue("keepalive_time", _mqttCon.keepalive_time);
		if(energy_mqtt.exists("timeout"))				energy_mqtt.lookupValue("timeout", _mqttCon.timeout);
		if(energy_mqtt.exists("max_packets"))			energy_mqtt.lookupValue("max_packets", _mqttCon.max_packets);

		if(energy_mqtt.exists("message_profile"))		energy_mqtt.lookupValue("message_profile", _mqttCon.message_profile);
		if(energy_mqtt.exists("control_profile"))		energy_mqtt.lookupValue("control_profile", _mqttCon.control_profile);

		if(energy_mqtt.exists("use_ssl"))				energy_mqtt.lookupValue("use_ssl", _mqttTls.useSsl);
		if(energy_mqtt.exists("tls_version"))			energy_mqtt.lookupValue("tls_version", _mqttTls.version);
		if(energy_mqtt.exists("tls_ca_file"))			energy_mqtt.lookupValue("tls_ca_file", _mqttTls.ca_file);
		if(energy_mqtt.exists("tls_ca_path"))			energy_mqtt.lookupValue("tls_ca_path", _mqttTls.ca_path);
		if(energy_mqtt.exists("tls_cert_file"))		energy_mqtt.lookupValue("tls_cert_file", _mqttTls.cert_file);
		if(energy_mqtt.exists("tls_key_file"))			energy_mqtt.lookupValue("tls_key_file", _mqttTls.key_file);
		if(energy_mqtt.exists("tls_insecure"))			energy_mqtt.lookupValue("tls_insecure", _mqttTls.insecure);
	}

	logging( __func__, __LINE__, "Loaded config file: " + fileName);
	return true;
}

//=====================================================================================================
bool energyMqtt::initInst()
{
	if (_mqttMsg.m == NULL) return error( __func__, __LINE__, 10001, "init() failure");

    if(isInitialized())
	{
		_initialized = false;

		int result = mosquitto_reinitialise( _mqttMsg.m, NULL, true, (void *) &_mqttMsg);

		if(result != MOSQ_ERR_SUCCESS) return error( __func__, __LINE__, 10002, mosquitto_strerror(result));
		logging( __func__, __LINE__, "Reinitialized;", 2);
	}

// Set callbacks:
	mosquitto_connect_callback_set( _mqttMsg.m, onConnect);
	logging( __func__, __LINE__, "Set callback `onConnect`", 3);

    mosquitto_disconnect_callback_set( _mqttMsg.m, onDisconnect);
	logging( __func__, __LINE__, "Set callback `onDisconnect`", 3);

	mosquitto_publish_callback_set( _mqttMsg.m, onPublish);
	logging( __func__, __LINE__, "Set callback `onPublish`", 3);

	mosquitto_subscribe_callback_set( _mqttMsg.m, onSubscribe);
	logging( __func__, __LINE__, "Set callback `onSubscribe`", 3);

    mosquitto_message_callback_set( _mqttMsg.m, onMessage);
	logging( __func__, __LINE__, "Set callback `onMessage`", 3);

// Set security:
	if( _mqttTls.useSsl && ! setTls() ) return error( __func__, __LINE__, 10005, "TLS failure");
	if( ! _mqttCon.username.empty() && ! setUserPass() ) return error( __func__, __LINE__, 10006, "User/Password set failure");

// Set reconnect trategy:
	if( mosquitto_reconnect_delay_set( _mqttMsg.m, 1, 60, true) != MOSQ_ERR_SUCCESS ) error( __func__, __LINE__, 10005, "Reconnection delay set failure");

	_initialized = true;
	return success();
}

//=====================================================================================================
bool energyMqtt::setTls()
{
	try{
		int result = mosquitto_tls_opts_set(_mqttMsg.m, 1, _mqttTls.version.c_str(), NULL);
		logging( __func__, __LINE__, "Set TLS version (" + _mqttTls.version + "): " + string(result == MOSQ_ERR_SUCCESS ? "OK" : mosquitto_strerror(result)) + ";", 3);

		result = mosquitto_tls_set(_mqttMsg.m, (_mqttTls.ca_file.empty() ? NULL : _mqttTls.ca_file.c_str()), (_mqttTls.ca_path.empty() ? NULL : _mqttTls.ca_path.c_str()), _mqttTls.cert_file.c_str(), _mqttTls.key_file.c_str(), NULL);
		if(result != MOSQ_ERR_SUCCESS) return error( __func__, __LINE__, 10003, mosquitto_strerror(result));
		logging( __func__, __LINE__, "Set TLS: OK;", 3);

		result = mosquitto_tls_insecure_set(_mqttMsg.m, _mqttTls.insecure);
		if(result != MOSQ_ERR_SUCCESS) return error( __func__, __LINE__, 10004, mosquitto_strerror(result));
		logging( __func__, __LINE__, "Set TLS insecure (" + string(_mqttTls.insecure ? "TRUE" : "FALSE") + "): OK;", 3);
	}catch(exception &e){
		return error( __func__, __LINE__, 10005, "TLS failure");
	}

	return success();
}

//=====================================================================================================
bool energyMqtt::setUserPass()
{
	try{
		int result = mosquitto_username_pw_set( _mqttMsg.m, _mqttCon.username.c_str(), _mqttCon.password.c_str());
		logging( __func__, __LINE__, "Set username `" + _mqttCon.username + "`: " + string(result == MOSQ_ERR_SUCCESS ? "OK" : mosquitto_strerror(result)) + ";", 3);
	}catch(exception &e){
		return error( __func__, __LINE__, 10005, "User/Password set failure");
	}

	return success();
}

//=====================================================================================================
bool energyMqtt::instConnect()
{
	if(! isInitialized()) return error( __func__, __LINE__, 10001, "Not initialized");

    int result = mosquitto_connect(_mqttMsg.m, _mqttCon.host.c_str(), _mqttCon.port, _mqttCon.keepalive_time);

	logging( __func__, __LINE__, "Host: " + _mqttCon.host + "; Port: " + sk::itoa(_mqttCon.port) + "; keepAliveSeconds: " + sk::itoa(_mqttCon.keepalive_time) +
		"; Result: (" + sk::itoa(result) + ") " + mosquitto_strerror(result), 3);
	return result == MOSQ_ERR_SUCCESS;
}

//=====================================================================================================
void energyMqtt::onConnect(struct mosquitto *m, void *udata, int result)
{
	if(udata == NULL) return;
	mqttMessage * mqttMsg = (mqttMessage *) udata;

	mqttMsg->instObj->logging( __func__, __LINE__, "Result: (" + sk::itoa(result) + ") " + mosquitto_strerror(result), 2);

    if ( result != MOSQ_ERR_SUCCESS ) mqttMsg->instObj->error( __func__, __LINE__, 10001, "Connection refused");
	else{
		mqttMsg->instObj->_connected = true;

	   if ( mqttMsg->instObj->_subscribed && ! mqttMsg->instObj->instSubscribe( m, NULL, mqttMsg->instObj->_mqttCon.control_profile.c_str(), 0))
		   mqttMsg->instObj->error( __func__, __LINE__, 55555, "Subscribtion error");
	}
}

//=====================================================================================================
bool energyMqtt::instDisconnect()
{
    int result = mosquitto_disconnect(_mqttMsg.m);

	logging( __func__, __LINE__, "Result: (" + sk::itoa(result) + ") " + mosquitto_strerror(result), 3);
	return (result == MOSQ_ERR_SUCCESS);
}

//=====================================================================================================
void energyMqtt::onDisconnect(struct mosquitto *m, void *udata, int result)
{
	if(udata == NULL) return;
	mqttMessage * mqttMsg = (mqttMessage *) udata;

    mqttMsg->instObj->_connected = false;

	switch(result)
	{
		case MOSQ_ERR_SUCCESS:
			if(mqttMsg->instObj->_enable_logging) mqttMsg->instObj->logging( __func__, __LINE__, "No error", 3); break;
		default:
			if( mqttMsg->instObj->_mqttTls.useSsl ) mqttMsg->instObj->setTls();
			if( ! mqttMsg->instObj->_mqttCon.username.empty() ) mqttMsg->instObj->setUserPass();

			if(mqttMsg->instObj->_enable_logging) mqttMsg->instObj->logging( __func__, __LINE__, "Unexpected result (" + sk::itoa(result) + "): " + mosquitto_strerror(result), 2);
	}
}

//=====================================================================================================
bool energyMqtt::instPublish(const char * data, int qos)
{
	if(data == NULL) return error( __func__, __LINE__, 12001, "Empty data");

	if(! isInitialized() && ! initInst()) return error( __func__, __LINE__, 12002, "Instance uninitialized");
	if(! isConnected() && ! instConnect()) return error( __func__, __LINE__, 12003, "Not connected");

// Ensure the connection is up:
	int result = MOSQ_ERR_SUCCESS;
	unsigned int stage = 0;
	do{
		switch( stage )
		{
			case 0:
				if( ! isConnected() ) result = mosquitto_loop( _mqttMsg.m, _mqttCon.timeout, _mqttCon.max_packets );
				break;
			case 1:
				result = mosquitto_publish( _mqttMsg.m, NULL, _mqttCon.message_profile.c_str(), strlen(data), data, qos, false );
				mosquitto_loop( _mqttMsg.m, _mqttCon.timeout, _mqttCon.max_packets );
				break;
			case 2:
				result = mosquitto_loop( _mqttMsg.m, _mqttCon.timeout, _mqttCon.max_packets );
				break;
			default:
				return error( __func__, __LINE__, 11111, "Something went wrong");
		}

	// Analyze result:
		switch( result )
		{
			case MOSQ_ERR_SUCCESS:
				if( ! isConnected() && stage != 1 ) break;
				stage ++;
				break;
			case MOSQ_ERR_NO_CONN: // Move to next case
			case MOSQ_ERR_CONN_LOST:
				error( __func__, __LINE__, 11111, mosquitto_strerror(result));
				instConnect();
				break;
			case MOSQ_ERR_PROTOCOL:
				if( _mqttTls.useSsl && ! setTls() ) return error( __func__, __LINE__, 10005, "TLS failure");
				if( ! _mqttCon.username.empty() && ! setUserPass() ) return error( __func__, __LINE__, 10006, "User/Password set failure");
				break;
			default:
				return error( __func__, __LINE__, 11111, mosquitto_strerror(result));
		}
	}while( result != MOSQ_ERR_SUCCESS || stage < 3 );

	logging( __func__, __LINE__, "To: " + _mqttCon.message_profile + "; Timeout: " + sk::itoa(_mqttCon.timeout) + "; Length: " + sk::itoa(strlen(data)) +
		"; QOS: " + sk::itoa(qos) + "; Result: (" + sk::itoa(result) + ") " + mosquitto_strerror(result), (result == MOSQ_ERR_SUCCESS ? 2 : 1));

	return success();
}

//=====================================================================================================
void energyMqtt::onPublish(struct mosquitto *m, void *udata, int m_id)
{
 	if(udata == NULL) return;
	mqttMessage * mqttMsg = (mqttMessage *) udata;

	if(mqttMsg->instObj->_enable_logging) mqttMsg->instObj->logging( __func__, __LINE__, "successful", 2);
}

//=====================================================================================================
bool energyMqtt::instSubscribe(struct mosquitto *m, int *mid, const char *sub, int qos)
{
	int result = mosquitto_subscribe(m, mid, sub, qos);
	if(_enable_logging) logging( __func__, __LINE__, "Profile: `" + string(sub == NULL ? "" : sub) + "`; Result: (" + sk::itoa(result) + ") " + mosquitto_strerror(result), 2);

	if(result == MOSQ_ERR_SUCCESS) return success();
	return error( __func__, __LINE__, 13003, mosquitto_strerror(result));
}

//=====================================================================================================
void energyMqtt::onSubscribe(struct mosquitto *m, void *udata, int mid, int qos_count, const int *granted_qos)
{
 	if(udata == NULL) return;
	mqttMessage * mqttMsg = (mqttMessage *) udata;
    mqttMsg->instObj->_subscribed = true;

	if(mqttMsg->instObj->_enable_logging) mqttMsg->instObj->logging( __func__, __LINE__, "successful", 2);
}

//=====================================================================================================
bool energyMqtt::runInstLoop()
{
	if(! isInitialized() && ! initInst()) return error( __func__, __LINE__, 11001, "Failed to initialize instance");
	if(! isConnected() && ! instConnect()) return error( __func__, __LINE__, 11002, "Failed to connect");

    if ( (! _subscribed) && (! instSubscribe( _mqttMsg.m, NULL, _mqttCon.control_profile.c_str(), 0)) ) return error( __func__, __LINE__, 11005, "Subscribtion error");

	logging( __func__, __LINE__, "Timeout: " + sk::itoa(_mqttCon.timeout) + "; MaxPackets: " + sk::itoa(_mqttCon.max_packets));
	int result = mosquitto_loop_forever( _mqttMsg.m, _mqttCon.timeout, _mqttCon.max_packets);

	if(result == MOSQ_ERR_SUCCESS) return success();
	return error( __func__, __LINE__, 11006, mosquitto_strerror(result));
}

//=====================================================================================================
void energyMqtt::onMessage(struct mosquitto *m, void *udata, const struct mosquitto_message *msg)
{
	if(msg == NULL) return;
	if(udata == NULL) return;
	mqttMessage * mqttMsg = (mqttMessage *) udata;

	if( mqttMsg->instObj->_enable_logging ) mqttMsg->instObj->logging( __func__, __LINE__, "Got message @ " + string(msg->topic) + ": (" + sk::itoa(msg->payloadlen) +
		", QoS " + sk::itoa(msg->qos) + ", " + (msg->retain ? "R" : "!r") + ") '" + string((char*) msg->payload) + "'", 2);

	mqttMsg->instObj->processCtrlData((const char*) msg->payload);
}

//=====================================================================================================
void energyMqtt::processCtrlData(const char * data)
{
	if(data == NULL) return;
	logging( __func__, __LINE__, "dataLength: " + sk::itoa(strlen(data)), 2);

	if(_on_ctrl_data_callback == NULL) return;

	bool result = _on_ctrl_data_callback(data);
	logging( __func__, __LINE__, "Result: " + string(result ? "TRUE" : "FALSE"), 2);
}

