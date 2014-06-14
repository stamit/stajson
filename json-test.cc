//
// simple input-output loop which handles JSON expressions, one per line
//
#include <iostream>
using namespace std;

#include <stajson.h>

int main(int argc, char **argv) {
	cerr <<
"This program will read lines form stdin in JSON format, decode them, encode\n"
"them and echo them back to stdout, along with a short description on stderr."
	<< endl;

	for (;;) {
		JSON json;

		cin >> json;

		//string in;
		//getline(cin,in);
		//if (!in.length() || in=="\n" || in=="\r\n") break;
		//json_decode(in,json);

		switch (json.type()) {

		case JSON_NULL:
			cerr << "I received a NULL object." << endl;
			break;

		case JSON_BOOLEAN:
			cerr << "I received a boolean with value "
			     << (json.boolean()?"\"true\"":"\"false\"")
			     << ". " << endl;
			break;

		case JSON_NUMBER:
			cerr << "I received a number with value \""
			     << json.number() << "\". " << endl;
			break;

		case JSON_STRING:
			cerr << "I received a string with "
			     << ((const string&)json).length()
			     << " characters." << endl;
			break;

		case JSON_ARRAY:
			cerr << "I received an array with "
			     << json.array().size()
			     << " elements:" << endl;
			for (JSONArray::iterator i = json.array().begin(),
			                         j = json.array().end() ;
			     i!=j ; ++i) {
				cerr << "\t" << json_encode(*i) << endl;
			}
			break;

		case JSON_OBJECT:
			cerr << "I received an object with "
			     << json.object().size()
			     << " members:" << endl;
			for (JSONObject::iterator i = json.object().begin(),
			                          j = json.object().end() ;
			     i!=j ; ++i) {
				cerr << "\t"<< json_encode(JSONString(i->first))
				     << ":" << json_encode(i->second)
				     << endl;
			}
			break;

		default:
			throw runtime_error("this should never happen");
		}

		cout << json << endl;
	}

	return 0;
}
