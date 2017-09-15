#include "energy_environment.h"

#include <iostream>
#include <string>
#include <regex>
#include <json/json.h>

using namespace std;

class energyMed
{
	protected:
		struct energyError{
			unsigned int code = 0;
			string info = "";
		}_error;

		bool _success = true;

		bool _enable_logging = false;
		string _log_file;
		unsigned int _debug_level = 0;

	protected:
		bool isNull(int i) { return i == ENERGY_NULL_VALUE; };
		bool isNull(const string & s) { return s.empty(); };

		virtual string version() { return ""; }
		void logging(const char * func, unsigned int line, const string & text, unsigned int level = 0);

	public:
		string getLastError();
		bool error(int code, const string &info, const string & log_info = "");
		bool error(const char * func, unsigned int line, unsigned int code, const string &info, const string & log_info = "");
		void clearError();
		bool getLastExecSuccess() { return this->_success; }
		bool success();

	public:
		json_object * parseMessage(const char * msg);
		char * buildMessage(json_object * data);
	public:
		energyMed() {};
		virtual ~energyMed() {};
};
