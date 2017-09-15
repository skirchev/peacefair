/*###############################################################################################
#       This file contains common directives                                                    #
#       file: energy_environment.h                                                              #
#       Created by: Stefan Kirchev, 	stefan.kirchev@gmail.com                                #
#                                                                                               #
#       OS: Unix like                                                                           #
#       History: 2017-09-11      version:   1.0 - first release                                 #
###############################################################################################*/

#pragma once
#ifndef __ENERGY_ENVIRONMENT__
#define __ENERGY_ENVIRONMENT__	1

#include <string>
#define ENERGY_NULL_VALUE					0

#define ENERGY_CONFIG_FILE_PATH				"/etc/energy/energy.cfg"
#define LOG_DIR								"/var/log/energy"

//###############################################################################################
#define ENERGY_MQTT_KEEPALIVE_TIME			60
#define ENERGY_MQTT_BROKER_HOSTNAME			"localhost"
#define ENERGY_MQTT_BROKER_PORT				1883
#define ENERGY_MQTT_CONN_TIMEOUT			1000
#define ENERGY_MQTT_CONN_MAX_PACKETS		1000
#define ENERGY_MQTT_MAX_THREADS				5
#define ENERGY_MQTT_REPORT_INTERVAL			60

#define ENERGY_MQTT_MSG_PROFILE_NAME		"energy"
#define ENERGY_MQTT_CTRL_PROFILE_NAME		"control"

//###############################################################################################
#define ENERGY_SERIAL_MESSAGE_LENGTH		7
#define ENERGY_SERIAL_CMD_VOLTAGE			0xB0
#define ENERGY_SERIAL_ANSWER_VOLTAGE		0xA0
#define ENERGY_SERIAL_CMD_CURRENT			0xB1
#define ENERGY_SERIAL_ANSWER_CURRENT		0xA1
#define ENERGY_SERIAL_CMD_ACT_POWER			0xB2
#define ENERGY_SERIAL_ANSWER_ACT_POWER		0xA2
#define ENERGY_SERIAL_CMD_ENERGY			0xB3
#define ENERGY_SERIAL_ANSWER_ENERGY			0xA3
#define ENERGY_SERIAL_CMD_ADDRESS			0xB4
#define ENERGY_SERIAL_ANSWER_ADDRESS		0xA4
#define ENERGY_SERIAL_CMD_ALARM_THR			0xB5
#define ENERGY_SERIAL_ANSWER_ALARM_THR		0xA5

//###############################################################################################
#define ENERGY_SERIAL_DEVICE				"/dev/ttyAMA0"
#define ENERGY_SERIAL_BAUD_RATE				9600

//###############################################################################################

#endif
