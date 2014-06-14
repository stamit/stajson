lib_LTLIBRARIES = libstajson.la
bin_PROGRAMS = json-test json-speed

libstajson_la_SOURCES = stajson.cc
libstajson_la_LDFLAGS = -version-info 0:1

json_test_SOURCES = json-test.cc
json_test_LDADD = -lstajson
json_test_CXXFLAGS = -pedantic -Wall -Werror

json_speed_SOURCES = json-speed.cc
json_speed_LDADD = -lstajson
json_speed_CXXFLAGS = -pedantic -Wall -Werror

run : json-speed json-test libstajson.la
	./json-test
#	./json-speed <test.json

debug : json-speed json-test libstajson.la
	gdb --args json-test
