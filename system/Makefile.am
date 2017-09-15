# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = --pedantic -Wall -std=c++14 -O2 -Wno-unused-function -fPIC \
	-I$(top_srcdir)/include -g

AM_LDFLAGS = -module -shared -static-libstdc++ -ldl -lconfig++

lib_LTLIBRARIES = libenergy_com.la libenergy_med.la

####################################################################

libenergy_com_la_SOURCES = energy_com_classes.cpp
libenergy_com_la_LDFLAGS = -module --static-libstdc++ -lconfig++

libenergy_med_la_SOURCES = energy_med_classes.cpp
libenergy_med_la_LDFLAGS = -module --static-libstdc++
