//
// program that measures the time it takes to decode JSON expressions
//
#include <sys/time.h>

#include <iostream>
using namespace std;

#include <stajson.h>

inline double doubletime() {
	struct timeval tv;
	if (gettimeofday(&tv, 0)==-1)
		throw runtime_error("cannot retrieve current date/time");
	return tv.tv_sec + tv.tv_usec/1000000.0;
}

int main(int argc, char **argv) {
	string in;

	{ double t = doubletime();
	getline(cin,in);
	t = doubletime() - t;
	cout << t << endl; }

	JSON json;

	{ double t = doubletime();
	json_decode(in, json);
	json.set(0);
	t = doubletime() - t;
	cout << t << endl; }
}
