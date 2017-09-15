/*###############################################################################################
#       SK Tools                                                                                #
#       file: sk_tools.h                                                                        #
#       Created by: Stefan Kirchev, 	stefan.kirchev@gmail.com                                #
#                                                                                               #
#       History: 2015-10-29      version:   1.0 - first release                                 #
#                2016-04-01                 1.1 - toDate added                                  #
#                2016-10-31                 1.2 - string padding added                          #
###############################################################################################*/

#pragma once
#ifndef __SK_TOOLS__H_
#define __SK_TOOLS__H_	1

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>

#include <sys/time.h>  
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <cmath>
#include <sstream>
#include <cstring>
//#include <regex>

using namespace std;

extern "C" bool __running_as_daemon__;

namespace sk{
	typedef map <string, string> smap_t;
	typedef vector <string> sarray_t;
	typedef vector <unsigned int> uarray_t;
	typedef vector <int> darray_t;

//=====================================================================================================
	inline string boolToString(bool b) { return b ? "true" : "false"; }
	inline bool stringToBool(const string &s) { return s.compare("true") == 0; }

//=====================================================================================================
	inline string itoa(int i)
	{
		char str[33];
		memset(str, 0x00, 33);
		sprintf(str, "%d", i);

		return string(str);
	}

//=====================================================================================================
	inline string itoa(unsigned int u)
	{
		char str[33];
		memset(str, 0x00, 33);
		sprintf(str, "%u", u);

		return string(str);
	}

//=====================================================================================================
	inline string itoa(long int l)
	{
		char str[33];
		memset(str, 0x00, 33);
		sprintf(str, "%ld", l);

		return string(str);
	}

//=====================================================================================================
	inline string itoa(unsigned long int lu)
	{
		char str[33];
		memset(str, 0x00, 33);
		sprintf(str, "%lu", lu);

		return string(str);
	}

//=====================================================================================================
	inline string itoa(double d, unsigned int precision = 8)
	{
		char * str = new char [33 + precision];
		memset(str, 0x00, 33 + precision);
		sprintf(str, ("%." + itoa((int) precision) + "f").c_str(), d);

		string out(str);
		delete [] str;

		return out;
	}

//=====================================================================================================
	inline string itoa(long double d, unsigned int precision = 8)
	{
		char * str = new char [33 + precision];
		memset(str, 0x00, 33 + precision);
		sprintf(str, ("%." + itoa((int) precision) + "Lf").c_str(), d);

		string out(str);
		delete [] str;

		return out;
	}

//=====================================================================================================
	inline vector<unsigned char> stringHexToBin(string str)
	{
		vector<unsigned char> data;
		unsigned int value;
		
		if(str.length() % 2) str.insert(0, "0");
		
		for(unsigned int i = 0; i < str.length(); i += 2)
		{
			stringstream ss(str.substr(i, 2));
			ss >> std::hex >> value;
			data.push_back(value);
		}
		
		return data;
	}

//=====================================================================================================
	inline string binHexToString(unsigned char *data, size_t size)
	{
		string str = "";
		char buff[3];
		memset(buff, 0x00, 3);

		for(unsigned int i = 0; i < size; i++)
		{
			sprintf(buff, "%.2X", data[i]);
			str.append(buff);
		}

		return str;
	}

//=====================================================================================================
	inline vector<unsigned char> stringToBin(string str)
	{
		vector<unsigned char> data;
		for(unsigned int i = 0; i < str.length(); i++) data.push_back(str.at(i));
		
		return data;
	}

//=====================================================================================================
	inline string binToString(unsigned char * in, size_t size)
	{
		string str = "";
		for(unsigned int i = 0; i < size; i++) str.append(1, in[i]);

		return str;
	}

//=====================================================================================================
	inline vector<unsigned char> intToBin(unsigned int in) { return intToBin((unsigned long int) in); }
	inline vector<unsigned char> intToBin(unsigned long int in)
	{
		vector<unsigned char> data;

		do{
			unsigned int value = in % 256;
			data.push_back(value);
			in = in >> 8;
		}while(in > 0);

		return data;
	}

//=====================================================================================================
	inline unsigned long int binToLongInt(unsigned char * in, size_t size)
	{
		unsigned long int value = 0;
		for(unsigned int i = 0; i < size; i++) value += in[size - i - 1] * pow(256, i);

		return value;
	}
	inline unsigned int binToInt(unsigned char * in, size_t size) { return (unsigned int) binToLongInt(in, size); }

//=====================================================================================================
	inline void normalizeIntBin(vector <unsigned char> &in, size_t size)
	{
		while(in.size() < size) in.push_back(0x00);

		vector <unsigned char> data;
		for(int i = size; i > 0; i--) data.push_back(in.at(i - 1));

		in.clear();
		in = data;
		data.clear();
	}

//=====================================================================================================
	inline vector<unsigned char> intToBinNor(unsigned long int in, size_t size)
	{
		vector<unsigned char> data = intToBin(in);
		normalizeIntBin(data, size);

		return data;
	}
	inline vector<unsigned char> intToBinNor(unsigned int in, size_t size) { return intToBinNor((unsigned long int) in, size); }

//=====================================================================================================
	inline string getDate(time_t time, string format = "%Y-%m-%d %X%z")
	{
		char buff[80];
		memset(buff, 0x00, 80);
		struct tm * tstruct = format.back() == 'Z' ? gmtime(&time) : localtime(&time);
		strftime(buff, sizeof(buff), format.c_str(), tstruct);

		return string(buff);
	}
/*
//=====================================================================================================
	inline string getDateFormat(string time_details)
	{
		if(std::regex_match(time_details, std::regex("^\\d{4}-\\d{2}-\\d{2}$"))) return "%Y-%m-%d";
		else if(std::regex_match(time_details, std::regex("^\\d{4}-\\d{2}-\\d{2}( )+\\d{2}:\\d{2}:\\d{2}$"))) return "%Y-%m-%d %X";
		else if(std::regex_match(time_details, std::regex("^\\d{4}-\\d{2}-\\d{2}( )+\\d{2}:\\d{2}:\\d{2}[-\\+]\\d{3}$"))) return "%Y-%m-%d %X%z";
		else if(std::regex_match(time_details, std::regex("^\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}Z$"))) return "%Y-%m-%dT%XZ";

		return "";
	}
*/

//=====================================================================================================
	inline string getDate(string format = "%Y-%m-%d %X%z", struct tm *in = NULL)
	{
		time_t now = time(0);
		char buff[80];
		memset(buff, 0x00, 80);
		struct tm tstruct = (in != NULL ? *in : (format.back() == 'Z' ? *gmtime(&now) : *localtime(&now)));
		strftime(buff, sizeof(buff), format.c_str(), &tstruct);

		return string(buff);
	}

//=====================================================================================================
	inline time_t toDate(string time_details, string format = "%Y-%m-%d %X%z")
	{
		struct tm tstruct;
		strptime(time_details.c_str(), format.c_str(), &tstruct);

		return format.back() == 'Z' ? timegm(&tstruct) : mktime(&tstruct);
	}

//=====================================================================================================
	inline vector <string> mapKeys(map <string, string> &in)
	{
		vector <string> out;
		for(map <string, string>::iterator it = in.begin(); it != in.end(); it ++) out.push_back(it->first);

		return out;
	}

//=====================================================================================================
	inline vector <string> mapValues(map <string, string> &in)
	{
		vector <string> out;
		for(map <string, string>::iterator it = in.begin(); it != in.end(); it ++) out.push_back(it->second);

		return out;
	}

//=====================================================================================================
	inline bool diffMap(map <string, string> &in1, map <string, string> &in2, map <string, string> &out)
	{
		if(in1.size() != in2.size()) return false;

		for(map <string, string>::iterator it1 = in1.begin(); it1 != in1.end(); it1 ++)
		{
			map <string, string>::iterator it2 = in2.find(it1->first);
			if(it2 == in2.end()) return false;

			if(it1->second.compare(it2->second) == 0) continue;
			out.insert(make_pair(it2->first, it2->second));
		}

		return true;
	}

//=====================================================================================================
	inline map <string, string> mapIntersect(map <string, string> &in, vector <string> &keys)
	{
		map <string, string> out;

		for(vector <string>::iterator it = keys.begin(); it != keys.end(); it ++)
		{
			map <string, string>::iterator it2 = in.find(*it);
			if(it2 == in.end()) out.insert(make_pair(it2->first, it2->second));
		}

		return out;
	}

//=====================================================================================================
	inline string implode(const string &delimiter, vector <string> &in)
	{

		switch (in.size())
		{
			case 0: return "";
			case 1: return in[0];
			default:
				ostringstream out;
				copy(in.begin(), in.end() - 1, ostream_iterator <string >(out, delimiter.c_str()));
				out << *in.rbegin();

				return out.str();
		}		
	}

//=====================================================================================================
	inline vector <string> explode(const char delimiter, const string &in)
	{
		vector <string> out;

		string::const_iterator cur = in.begin();
		string::const_iterator beg = in.begin();

		while ( cur < in.end() )
		{
			if ( *cur == delimiter )
			{
				out.insert( out.end() , string( beg , cur) );
				beg = ++cur;
			}else{
				cur ++;
			}
		}

		out.insert( out.end() , string( beg , cur) );		

		return out;
	}

//=====================================================================================================
	inline vector <string> explode(const string &delimiter, const string &in, unsigned int limit = 0)
	{
		vector <string> out;
		if(in.empty()) return out;

		char * begin = (char *) in.c_str();
		char * pos = NULL;

		while((pos = strstr(begin, delimiter.c_str())) != NULL)
		{
			if(limit > 0 && limit - 1 == out.size()) break;

			out.push_back(string(begin).substr(0, pos - begin));
			begin = pos + delimiter.length();
		}
		out.push_back(string(begin));

		return out;
	}

//=====================================================================================================
	static inline string &ltrim(string & in, const char delimiter = ' ')
	{
		while(in.c_str() == strchr(in.c_str(), delimiter)) in.erase(0, 1);
        return in;
	}

//=====================================================================================================
	static inline string &rtrim(string & in, const char delimiter = ' ')
	{
		while(in.c_str() + (in.length() - 1) == strrchr(in.c_str(), delimiter)) in.erase(in.length() - 1, 1);
        return in;
	}

//=====================================================================================================
	static inline string &trim(string & in, const char delimiter = ' ')
	{
		return ltrim(rtrim(in, delimiter), delimiter);
	}

//=====================================================================================================
	static inline string padleft(string & in, const char character, size_t n)
	{
		if(n <= 0 || n <= in.length()) return in;
        return string(n - in.length(), character) + in;
	}

//=====================================================================================================
	static inline string padright(string & in, const char character, size_t n)
	{
		if(n <= 0 || n <= in.length()) return in;
        return in + string(n - in.length(), character);
	}

//=====================================================================================================
	static inline string wrap(const string & in, const string & wrapper)
	{
		return string(wrapper) + string(in) + string(wrapper);
	}

//=====================================================================================================
	static inline const char * copy(const char * in)
	{
		if( ! in ) return NULL;

		char * out = new char[strlen(in) + 1];
		memset( out, 0, strlen(in) + 1);
		strncpy( out, in, strlen( in ));

		return out;
	}

//=====================================================================================================
	static inline string urlEncode(const string & value) {
		ostringstream escaped;
		escaped.fill('0');
		escaped << hex;

		for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
		{
			string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
			{
				escaped << c;
				continue;
			}

		// Any other characters are percent-encoded
			escaped << uppercase;
			escaped << '%' << setw(2) << int((unsigned char) c);
			escaped << nouppercase;
		}

		return escaped.str();
	}

//=====================================================================================================
	static inline string urlDecode(string & in)
	{
		string out;
		char ch;
		unsigned int ii;

		for (size_t i = 0; i < in.length(); i++)
		{
			if (int(in[i]) == 37)
			{
				sscanf(in.substr(i + 1, 2).c_str(), "%x", &ii);
				ch = static_cast<char> (ii);
				out += ch;
				i = i + 2;
			} else {
				out += in[i];
			}
		}
		return (out);
	}

//=====================================================================================================
	static inline string getFileName(const string & in)
	{
		size_t i = in.rfind('/', in.length());
		if (i != string::npos) return (in.substr(i + 1, in.length() - i));

		return in;
	}

//=====================================================================================================
	static inline string getFileExt(const string & in)
	{
	   size_t i = in.rfind('.', in.length());
	   if (i != string::npos) return(in.substr(i + 1, in.length() - i));

	   return "";
	}
}

#endif
