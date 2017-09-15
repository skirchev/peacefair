# what flags you want to pass to the C compiler & linker

energydir = $(prefix)/energy

AM_CPPFLAGS = --pedantic -Wall -std=c++14 -O2 -Wno-unused-function \
	-I$(top_srcdir)/include -I$(top_srcdir)/system -g

energy_PROGRAMS = energy_com energy_read energy_write energy_dump
####################################################################

energy_com_SOURCES = energy_com_tool.cpp
energy_com_LDFLAGS = -L$(top_srcdir)/system/.libs \
	-shared -lconfig++ -lenergy_com -lenergy_med -ljson -lmosquitto -lwiringPi -lwiringPiDev -pthread -ldl

energy_read_SOURCES = energy_read.cpp
energy_read_LDFLAGS = -L$(top_srcdir)/system/.libs \
	-shared -lconfig++ -lenergy_com -lenergy_med -ljson -lmosquitto -lwiringPi -lwiringPiDev -pthread -ldl

energy_write_SOURCES = energy_write.cpp
energy_write_LDFLAGS = -L$(top_srcdir)/system/.libs \
	-shared -lconfig++ -lenergy_com -lmosquitto -lwiringPi -lwiringPiDev -pthread -ldl

energy_dump_SOURCES = energy_dump.cpp
energy_dump_LDFLAGS = -L$(top_srcdir)/system/.libs \
	-shared -lconfig++ -lenergy_com -lenergy_med -ljson -lmosquitto -lwiringPi -lwiringPiDev -pthread -ldl
