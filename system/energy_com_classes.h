#include "energy_environment.h"
#include "mosquitto.h"

#include <iostream>
#include <string>
#include <regex>
#include <map>

using namespace std;

class energyCom
{
	protected:
		struct energyError{
			unsigned int code = 0;
			string info = "";
		}_error;

		bool _success = true;

		bool _enable_logging = false;
		string _conf_file = ENERGY_CONFIG_FILE_PATH;
		string _log_file;
		unsigned int _debug_level = 0;

	protected:
		bool isNull(int i) { return i == ENERGY_NULL_VALUE; };
		bool isNull(const string & s) { return s.empty(); };

		virtual string version() { return ""; }
		void logging(const char * func, unsigned int line, const string & text, unsigned int level = 0);

	public:
		virtual string getVersion();
		string getLastError();
		bool error(int code, const string &info, const string & log_info = "");
		bool error(const char * func, unsigned int line, unsigned int code, const string &info, const string & log_info = "");
		void clearError();
		bool getLastExecSuccess() { return this->_success; }
		bool success();

	public:
		energyCom() {};
		energyCom(string conf_file) { this->_conf_file = conf_file; }
		virtual ~energyCom() {};
};

//=====================================================================================================
class energySerial: public energyCom
{
	private:
		struct serialCon {
			string			device			= ENERGY_SERIAL_DEVICE;
			unsigned int	baud_rate		= ENERGY_SERIAL_BAUD_RATE;
		}_serialCon;

		int _fd = -1;

		bool _initialized = false;
		bool _connected = false;
		bool _exit_flag = false;
		bool (* _on_data_callback)(const char * data) = NULL;

	public:
		virtual string getVersion();
		bool loadConfig(string fileName);

		bool initInst();
		bool clearInst();
		bool runInstLoop();
		bool instWrite(const char * data = NULL);
		bool instWrite(vector <char> data);

		void setDataCallback(bool (* callback)(const char * data)) { this->_on_data_callback = callback; }
		void onMessage(const char * data);
		char calculateCumulativeSum(const char * data);
	public:
		energySerial();
		energySerial(const string & conf_file);
		energySerial(const string & dev, unsigned int baud_rate);
		virtual ~energySerial();
};

//=====================================================================================================
class energyMqtt: public energyCom
{
	private:
		struct mqttMessage {
			struct mosquitto * m	= NULL;
			energyMqtt * instObj	= NULL;
		}_mqttMsg;

		struct mqttTls {
			bool useSsl			= false;
			string version		= "tlsv1";
			string ca_file		= "";
			string ca_path		= "";
			string cert_file	= "";
			string key_file		= "";
			bool insecure		= false;
		}_mqttTls;

		struct mqttCon {
			string			host			= ENERGY_MQTT_BROKER_HOSTNAME;
			unsigned int	port			= ENERGY_MQTT_BROKER_PORT;
			string			username		= "";
			string			password		= "";
			unsigned int	keepalive_time	= ENERGY_MQTT_KEEPALIVE_TIME;
			unsigned int	timeout			= ENERGY_MQTT_CONN_TIMEOUT;
			unsigned int	max_packets		= ENERGY_MQTT_CONN_MAX_PACKETS;

			string			message_profile	= ENERGY_MQTT_MSG_PROFILE_NAME;
			string			control_profile	= ENERGY_MQTT_CTRL_PROFILE_NAME;
		}_mqttCon;

		bool _initialized	= false;
		bool _connected		= false;
		bool _subscribed	= false;

		bool (* _on_ctrl_data_callback)(const char *) = NULL;

	public:
		bool setTls();
		bool setUserPass();

	public:
		virtual string getVersion();
		bool loadConfig(string fileName);

		string hostMqtt() { return this->_mqttCon.host; }
		int portMqtt() { return this->_mqttCon.port; }
		int keepAliveSeconds() { return this->_mqttCon.keepalive_time; }

		void hostMqtt(string host) { this->_mqttCon.host = host; }
		void portMqtt(unsigned int port) { this->_mqttCon.port = port; }
		void keepAliveSeconds(unsigned int seconds) { this->_mqttCon.keepalive_time = seconds; }

		bool isInitialized() { return _initialized; }
		bool isConnected() { return _connected; }
	public:
		bool initInst();
		bool instConnect();
		bool instDisconnect();
		bool runInstLoop();
		bool instSubscribe(struct mosquitto *m, int *mid, const char *sub, int qos = 0);
		bool instPublish(const char * data, int qos = 0);

		static void onConnect(struct mosquitto *m, void *udata, int result);
		static void onDisconnect(struct mosquitto *m, void *udata, int result);
		static void onPublish(struct mosquitto *m, void *udata, int m_id);
		static void onSubscribe(struct mosquitto *m, void *udata, int mid, int qos_count, const int *granted_qos);
		static void onMessage(struct mosquitto *m, void *udata, const struct mosquitto_message *msg);

		void processCtrlData(const char * data);
		void setCtrlDataCallback(bool (* callback)(const char *)) { this->_on_ctrl_data_callback = callback; }
	public:
		energyMqtt();
		virtual ~energyMqtt();
};
