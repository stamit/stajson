#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stajson.h>
#include <string.h>

using namespace std;

static std::string::const_iterator json_decode(const std::string& in, std::string::const_iterator pos, JSON& out);

//
// boolean encoding
//
void json_encode(const JSONBool& in, std::string& out) {
	out.append(in ? "true" : "false");
}
void json_encode(const JSONBool& in, ostream& out) {
	out << (in ? "true" : "false");
}

//
// number encoding
//
void json_encode(const JSONNumber& in, std::string& out) {
	out.append(json_fmt("%.15g",(double)in));  // XXX is this right?
}
void json_encode(const JSONNumber& in, ostream& out) {
	out.flags(ios::fmtflags());
	out.precision(15);
	out << (double)in;
}


//
// string encoding
//
void json_encode(const JSONString& in, std::string& out) {
	const std::string& s = in;

	out.push_back('"');
	for (size_t i = 0 ; i < s.length() ; ++i) {
		switch (s[i]) {
		case '"': out.append("\\\""); break;
		case '\\': out.append("\\\\"); break;
		case '\b': out.append("\\b"); break;
		case '\f': out.append("\\f"); break;
		case '\n': out.append("\\n"); break;
		case '\r': out.append("\\r"); break;
		case '\t': out.append("\\t"); break;
		default:
			if (s[i]<32) {
				out.append(json_fmt("\\u%.4x", (unsigned char)s[i]));
			} else {
				out.push_back(s[i]);
			}
		}
	}
	out.push_back('"');
}
void json_encode(const JSONString& in, ostream& out) {
	const std::string& s = in;

	out << '"';
	for (size_t i = 0 ; i < s.length() ; ++i) {
		switch (s[i]) {
		case '"': out << "\\\""; break;
		case '\\': out << "\\\\"; break;
		case '\b': out << "\\b"; break;
		case '\f': out << "\\f"; break;
		case '\n': out << "\\n"; break;
		case '\r': out << "\\r"; break;
		case '\t': out << "\\t"; break;
		default:
			if (s[i]<32) {
				out.flags(ios::hex | ios::right);
				out.width(4);
				out.fill('0');
				out << (unsigned int)s[i];
			} else {
				out << s[i];
			}
		}
	}
	out << '"';
}


//
// array encoding
//
void json_encode(const JSONArray& a, std::string& out) {
	out.push_back('[');
	for (JSONArray::const_iterator i=a.begin() ; i!=a.end() ; ++i) {
		if (i!=a.begin())
			out.push_back(',');
		json_encode(*i, out);
	}
	out.push_back(']');
}
void json_encode(const JSONArray& a, ostream& out) {
	out << '[';
	for (JSONArray::const_iterator i=a.begin() ; i!=a.end() ; ++i) {
		if (i!=a.begin())
			out << ',';
		json_encode(*i, out);
	}
	out << ']';
}


//
// object encoding
//
void json_encode(const JSONObject& o, std::string& out) {
	out.push_back('{');
	for (JSONObject::const_iterator i=o.begin() ; i!=o.end() ; ++i) {
		if (i!=o.begin())
			out.push_back(',');
		json_encode(JSONString(i->first), out);
		out.push_back(':');
		json_encode(i->second, out);
	}
	out.push_back('}');
}
void json_encode(const JSONObject& o, ostream& out) {
	out << '{';
	for (JSONObject::const_iterator i=o.begin() ; i!=o.end() ; ++i) {
		if (i!=o.begin())
			out << ',';
		json_encode(JSONString(i->first), out);
		out << ':';
		json_encode(i->second, out);
	}
	out << '}';
}


//
// encoding
//
void json_encode(const JSON& in, std::string& out) {
	switch (in.type()) {
	case JSON_NULL: out.append("null"); break;
	case JSON_BOOLEAN: json_encode(in.boolean(), out); break;
	case JSON_NUMBER: json_encode(in.number(), out); break;
	case JSON_STRING: json_encode(in.string(), out); break;
	case JSON_ARRAY: json_encode(in.array(), out); break;
	case JSON_OBJECT: json_encode(in.object(), out); break;
	}
}
void json_encode(const JSON& in, ostream& out) {
	switch (in.type()) {
	case JSON_NULL: out << "null"; break;
	case JSON_BOOLEAN: json_encode(in.boolean(), out); break;
	case JSON_NUMBER: json_encode(in.number(), out); break;
	case JSON_STRING: json_encode(in.string(), out); break;
	case JSON_ARRAY: json_encode(in.array(), out); break;
	case JSON_OBJECT: json_encode(in.object(), out); break;
	}
}


////////////////////////////////////////////////////////////////////////////////


//
// decoding from string
//
void json_decode(const std::string& in, JSON& out) {
	std::string::const_iterator p = json_decode(in,in.begin(),out);
	for (; p != in.end() ; ++p) {
		switch (*p) {
		case ' ': case '\t': case '\r': case '\n':
			break;
		default:
			throw runtime_error(json_fmt("%d: JSON syntax error",p-in.begin()));
		}
	}
}
std::string::const_iterator json_decode(const std::string& in, std::string::const_iterator start, JSON& out) {
	int state = 0;
	std::string::const_iterator pos;
	std::string *str = 0;
	JSONArray *array=0;
	JSONObject *object=0;
	JSON key;
	for (pos=start ; pos!=in.end() ; ++pos) {
		switch (state) {
		case 0:
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				++start;
				break;

			case 'n':
				if (in.end()-pos<4 || strncmp(&*pos,"null",4)!=0)
					throw runtime_error(json_fmt("%d: JSON syntax error: expected \"null\"",pos-in.begin()));

				pos+=4;
				//state=1;
				goto accept_null;

			case 'f':
				if (in.end()-pos<5 || strncmp(&*pos,"false",5)!=0)
					throw runtime_error(json_fmt("%d: JSON syntax error: expected \"false\"",pos-in.begin()));

				pos+=5;
				//state=2;
				goto accept_false;
			case 't':
				if (in.end()-pos<4 || strncmp(&*pos,"true",4)!=0)
					throw runtime_error(json_fmt("%d: JSON syntax error: expected \"true\"",pos-in.begin()));

				pos+=4;
				//state=3;
				goto accept_true;

			case '-':
				state=4;
				break;
			case '0':
				state=6;
				break;
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				state=5;
				break;

			case '"':
				str = new string();
				state=12;
				break;
			case '[':
				array = new JSONArray();
				state=15;
				break;
			case '{':
				object = new JSONObject();
				state=19;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: invalid token",pos-in.begin()));
			}
			break;

		//
		// NULL
		//
		//case 1:  // after null
		//	goto accept_null;

		//
		// BOOLEAN
		//
		//case 2:  // after true
		//	goto accept_true;
		//case 3:  // after false
		//	goto accept_false;

		//
		// NUMBER
		//
		case 4:  // after minus sign
			switch (*pos) {
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				state=5;
				break;
			case '0':
				state=6;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected digit after minus sign",pos-in.begin()));
			}
			break;
		case 5:  // after first digit
			switch (*pos) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				break;
			case '.':
				state=7;
				break;
			case 'E': case 'e':
				state=9;
				break;
			default:
				goto accept_number;
			}
			break;
		case 6:  // after zero first digit
			switch (*pos) {
			case '.':
				state=7;
				break;
			case 'E': case 'e':
				state=9;
				break;
			default:
				goto accept_number;
			}
			break;
		case 7:  // after decimal point
			switch (*pos) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				state=8;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected digit after decimal point",pos-in.begin()));
			}
			break;
		case 8:  // after first decimal digit
			switch (*pos) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				break;
			case 'E': case 'e':
				state=9;
				break;
			default:
				goto accept_number;
			}
			break;
		case 9:  // after E
			switch (*pos) {
			case '+': case '-':
				state=10;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				state=11;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected digit after 'E'",pos-in.begin()));
			}
			break;
		case 10:  // after E+ or E-
			switch (*pos) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				state=11;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected digit after 'E+' or 'E-'",pos-in.begin()));
			}
			break;
		case 11:  // after first exponent digit
			switch (*pos) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				break;
			default:
				goto accept_number;
			}
			break;

		//
		// STRING
		//
		case 12:  // after opening quotes
			switch (*pos) {
			case '"':
				//state=13;
				++pos;
				goto accept_string;
			case '\\':
				state=14;
				break;
			default:
				str->push_back(*pos);
			}
			break;
		//case 13:  // after closing quotes
		//	goto accept_string;
		case 14:  // after backslash
			switch (*pos) {
			case '"': case '\\': case '/':
				str->push_back(*pos);
				break;
			case 'b':
				str->push_back('\b');
				break;
			case 'f':
				str->push_back('\f');
				break;
			case 'n':
				str->push_back('\n');
				break;
			case 'r':
				str->push_back('\r');
				break;
			case 't':
				str->push_back('\t');
				break;
			case 'u': {
				if (in.end()-pos < 4) {
					delete str; str = 0;
					throw runtime_error(json_fmt("%d: JSON syntax error: expected at least 4 characters after '\\u'",pos-in.begin()));
				}
				
				unsigned long hex=0;
				size_t i;
				for (i = 0 ; i < 4 ; ++i) {
					++pos;
					hex *= 16;
					switch (*pos) {
					case '0': hex += 0; break;
					case '1': hex += 1; break;
					case '2': hex += 2; break;
					case '3': hex += 3; break;
					case '4': hex += 4; break;
					case '5': hex += 5; break;
					case '6': hex += 6; break;
					case '7': hex += 7; break;
					case '8': hex += 8; break;
					case '9': hex += 9; break;
					case 'a': case 'A': hex += 10; break;
					case 'b': case 'B': hex += 11; break;
					case 'c': case 'C': hex += 12; break;
					case 'd': case 'D': hex += 13; break;
					case 'e': case 'E': hex += 14; break;
					case 'f': case 'F': hex += 15; break;
					default:
						goto endhex;
					}
				}
				endhex:

				if (!i) {
					delete str; str = 0;
					throw runtime_error(json_fmt("%d: JSON syntax error: no hex digits follow '\\u'",
					                    pos-in.begin()));
				}

				wchar_t wc[2];
				wc[0] = hex;
				wc[1] = 0;
				char mb[17];
				mb[16] = 0;
				i = wcstombs(mb,wc,16);
				if (i==(size_t)-1 || i>16) {

					//fprintf(stderr, "%ld: JSON could not convert wide char (ignoring): %lX\n", pos-in.begin(), hex);

					//throw runtime_error(json_fmt("%d: JSON could not convert wide char (code %X)",
					//                        pos-in.begin(), hex));

					//str->push_back(' ');
					//delete str; str = 0;
				} else if (i) {
					for (size_t j = 0 ; j < i ; ++j)
						str->push_back(mb[j]);
					//str->append(mb,i);
				}
				} break;
			default:
				delete str; str = 0;
				throw runtime_error(json_fmt("%d: JSON syntax error: invalid backslash escape",pos-in.begin()));
			}
			state=12;
			break;

		//
		// ARRAY
		//
		case 15:  // after array start
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ']':
				//state=16;
				++pos;
				goto accept_array;
			default:
				array->push_back(JSON());
				pos = json_decode(in, pos, array->back())-1;
				state=17;
			}
			break;
		//case 16:  // after array end
		//	goto accept_array;
		case 17:  // after array element
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ',':
				state=18;
				break;
			case ']':
				//state=16;
				++pos;
				goto accept_array;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected ',' or ']' after array element",pos-in.begin()));
			}
			break;
		case 18:  // after comma
			array->push_back(JSON());
			pos = json_decode(in, pos, array->back())-1;
			state=17;
			break;

		//
		// OBJECT
		//
		case 19:  // after object begin
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case '}':
				//state=20;
				++pos;
				goto accept_object;
			case '"':
				pos = json_decode(in, pos, key)-1;
				//if (key.type()!=JSON_STRING)
				//	throw runtime_error(json_fmt("%d: JSON key not string",pos-in.begin()));
				state=21;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected '}' or '\"' after object start",pos-in.begin()));
			}
			break;
		//case 20:  // after object end
		//	goto accept_object;
		case 21:  // after key
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ':':
				state=22;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected ':' after object key",pos-in.begin()));
			}
			break;
		case 22:
			pos = json_decode(in, pos, (*object)[key.string()])-1;
			state=23;
			break;
		case 23:  // after value
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ',':
				state=24;
				break;
			case '}':
				//state=20;
				++pos;
				goto accept_object;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected ',' or '}' after object key-value pair",pos-in.begin()));
			}
			break;
		case 24:  // after comma
			switch (*pos) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case '"':
				pos = json_decode(in, pos, key)-1;
				//if (key.type()!=JSON_STRING)
				//	throw runtime_error(json_fmt("%d: JSON key not string",pos-in.begin()));
				state=21;
				break;
			default:
				throw runtime_error(json_fmt("%d: JSON syntax error: expected '\"' after comma in object",pos-in.begin()));
			}
			break;
		}
	}

	switch (state) {

	accept_null: case 1:
		out.set(0);
		break;

	accept_true: case 2:
		out = true;
		break;

	accept_false: case 3:
		out = false;
		break;

	accept_number: case 5: case 6: case 8: case 11: {
		std::string s = in.substr(start-in.begin(),pos-start);
		const char *startp=s.c_str();
		char *endp=0;
		double v = strtod(startp,&endp);
		if (startp==endp)
			throw runtime_error(json_fmt("%d: JSON number does not parse",start-in.begin()));
		out = v;
		} break;

	accept_string: case 13:
		out = *str;
		delete str; str = 0;
		break;

	accept_array: case 16:
		out.set(array);
		break;
	
	accept_object: case 20:
		out.set(object);
		break;

	default:
		throw runtime_error(json_fmt("%d: JSON invalid end of input",pos-in.begin()));
	}

	return pos;
}


////////////////////////////////////////////////////////////////////////////////


//
// decoding from istream
//
void json_decode(std::istream& in, JSON& out) {
	int state = 0;
	std::string *str;
	JSONArray *array=0;
	JSONObject *object=0;
	JSON key;
	string number;
	for (char c = in.get() ; in.good() ; c = in.get()) {
		switch (state) {
		case 0:
			switch (c) {

			case ' ': case '\t': case '\r': case '\n':
				break;

			case 'n':
				c = in.get();
				if (!in.good() || c!='u') throw runtime_error("JSON syntax error: expected \"null\"");
				c = in.get();
				if (!in.good() || c!='l') throw runtime_error("JSON syntax error: expected \"null\"");
				c = in.get();
				if (!in.good() || c!='l') throw runtime_error("JSON syntax error: expected \"null\"");

				//state=1;
				goto accept_null;

			case 'f':
				c = in.get();
				if (!in.good() || c!='a') throw runtime_error("JSON syntax error: expected \"false\"");
				c = in.get();
				if (!in.good() || c!='l') throw runtime_error("JSON syntax error: expected \"false\"");
				c = in.get();
				if (!in.good() || c!='s') throw runtime_error("JSON syntax error: expected \"false\"");
				c = in.get();
				if (!in.good() || c!='e') throw runtime_error("JSON syntax error: expected \"false\"");

				//state=2;
				goto accept_false;
			case 't':
				c = in.get();
				if (!in.good() || c!='r') throw runtime_error("JSON syntax error: expected \"true\"");
				c = in.get();
				if (!in.good() || c!='u') throw runtime_error("JSON syntax error: expected \"true\"");
				c = in.get();
				if (!in.good() || c!='e') throw runtime_error("JSON syntax error: expected \"true\"");

				//state=3;
				goto accept_true;

			case '-':
				number.push_back(c);
				state=4;
				break;
			case '0':
				number.push_back(c);
				state=6;
				break;
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				number.push_back(c);
				state=5;
				break;

			case '"':
				str = new string();
				state=12;
				break;
			case '[':
				array = new JSONArray();
				state=15;
				break;
			case '{':
				object = new JSONObject();
				state=19;
				break;
			default:
				throw runtime_error("JSON syntax error: invalid token");
			}
			break;

		//
		// NULL
		//
		//case 1:  // after null
		//	in.unget();
		//	goto accept_null;

		//
		// BOOLEAN
		//
		//case 2:  // after true
		//	in.unget();
		//	goto accept_true;
		//case 3:  // after false
		//	in.unget();
		//	goto accept_false;

		//
		// NUMBER
		//
		case 4:  // after minus sign
			switch (c) {
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				number.push_back(c);
				state=5;
				break;
			case '0':
				number.push_back(c);
				state=6;
				break;
			default:
				throw runtime_error("JSON syntax error: expected digit after minus sign");
			}
			break;
		case 5:  // after first digit
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				number.push_back(c);
				//state=5;
				break;
			case '.':
				number.push_back(c);
				state=7;
				break;
			case 'E': case 'e':
				number.push_back(c);
				state=9;
				break;
			default:
				in.unget();
				goto accept_number;
			}
			break;
		case 6:  // after zero first digit
			switch (c) {
			case '.':
				number.push_back(c);
				state=7;
				break;
			case 'E': case 'e':
				number.push_back(c);
				state=9;
				break;
			default:
				in.unget();
				goto accept_number;
			}
			break;
		case 7:  // after decimal point
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				number.push_back(c);
				state=8;
				break;
			default:
				throw runtime_error("JSON syntax error: expected digit after decimal point");
			}
			break;
		case 8:  // after first decimal digit
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				number.push_back(c);
				//state=8;
				break;
			case 'E': case 'e':
				number.push_back(c);
				state=9;
				break;
			default:
				in.unget();
				goto accept_number;
			}
			break;
		case 9:  // after E
			switch (c) {
			case '+': case '-':
				number.push_back(c);
				state=10;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				number.push_back(c);
				state=11;
				break;
			default:
				throw runtime_error("JSON syntax error: expected digit after 'E'");
			}
			break;
		case 10:  // after E+ or E-
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				number.push_back(c);
				state=11;
				break;
			default:
				throw runtime_error("JSON syntax error: expected digit after 'E+' or 'E-'");
			}
			break;
		case 11:  // after first exponent digit
			switch (c) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				number.push_back(c);
				//state=11;
				break;
			default:
				in.unget();
				goto accept_number;
			}
			break;

		//
		// STRING
		//
		case 12:  // after opening quotes
			switch (c) {
			case '"':
				//state=13;
				goto accept_string;
			case '\\':
				state=14;
				break;
			default:
				str->push_back(c);
			}
			break;
		//case 13:  // after closing quotes
		//	in.unget();
		//	goto accept_string;
		case 14:  // after backslash
			switch (c) {
			case '"': case '\\': case '/':
				str->push_back(c);
				break;
			case 'b':
				str->push_back('\b');
				break;
			case 'f':
				str->push_back('\f');
				break;
			case 'n':
				str->push_back('\n');
				break;
			case 'r':
				str->push_back('\r');
				break;
			case 't':
				str->push_back('\t');
				break;
			case 'u': {
				unsigned long hex=0;
				size_t i;
				for (i = 0 ; i < 4 ; ++i) {
					c = in.get();
					if (!in.good()) {
						delete str; str = 0;
						throw runtime_error("JSON syntax error: expected at least 4 characters after '\\u'");
					}

					hex *= 16;
					switch (c) {
					case '0': hex += 0; break;
					case '1': hex += 1; break;
					case '2': hex += 2; break;
					case '3': hex += 3; break;
					case '4': hex += 4; break;
					case '5': hex += 5; break;
					case '6': hex += 6; break;
					case '7': hex += 7; break;
					case '8': hex += 8; break;
					case '9': hex += 9; break;
					case 'a': case 'A': hex += 10; break;
					case 'b': case 'B': hex += 11; break;
					case 'c': case 'C': hex += 12; break;
					case 'd': case 'D': hex += 13; break;
					case 'e': case 'E': hex += 14; break;
					case 'f': case 'F': hex += 15; break;
					default:
						in.unget();
						goto endhex;
					}
				}
				endhex:

				if (!i) {
					delete str; str = 0;
					throw runtime_error("JSON syntax error: no hex digits follow '\\u'");
				}

				wchar_t wc[2];
				wc[0] = hex;
				wc[1] = 0;

				char mb[17];
				mb[16] = 0;

				i = wcstombs(mb,wc,16);
				if (i==(size_t)-1 || i>16) {
					//fprintf(stderr, "JSON could not convert wide char (ignoring): %lX\n", hex);
				} else if (i) {
					for (size_t j = 0 ; j < i ; ++j)
						str->push_back(mb[j]);
				}
				} break;
			default:
				delete str; str = 0;
				throw runtime_error("JSON syntax error: invalid backslash escape");
			}
			state=12;
			break;

		//
		// ARRAY
		//
		case 15:  // after array start
			switch (c) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ']':
				//state=16;
				goto accept_array;
			default:
				in.unget();
				array->push_back(JSON());
				json_decode(in, array->back());
				state=17;
			}
			break;
		//case 16:  // after array end
		//	goto accept_array;
		case 17:  // after array element
			switch (c) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ',':
				//state=18;
				array->push_back(JSON());
				json_decode(in, array->back());
				//state=17;
				break;
			case ']':
				//state=16;
				goto accept_array;
			default:
				throw runtime_error("JSON syntax error: expected ',' or ']' after array element");
			}
			break;
		//case 18:  // after comma
		//	in.unget();
		//	array->push_back(JSON());
		//	json_decode(in, array->back());
		//	state=17;
		//	break;

		//
		// OBJECT
		//
		case 19:  // after object begin
			switch (c) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case '}':
				//state=20;
				goto accept_object;
			case '"':
				in.unget();
				json_decode(in, key);
				if (key.type()!=JSON_STRING)
					throw runtime_error("JSON key not string");
				state=21;
				break;
			default:
				throw runtime_error("JSON syntax error: expected '}' or '\"' after object start");
			}
			break;
		//case 20:  // after object end
		//	goto accept_object;
		case 21:  // after key
			switch (c) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ':':
				//state=22;
				json_decode(in, (*object)[key.string()]);
				state=23;
				break;
			default:
				throw runtime_error("JSON syntax error: expected ':' after object key");
			}
			break;
		//case 22:
		//	in.unget();
		//	{ JSON& value = (*object)[key.string()];
		//	json_decode(in, value); }
		//	state=23;
		//	break;
		case 23:  // after value
			switch (c) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case ',':
				state=24;
				break;
			case '}':
				state=20;
				goto accept_object;
			default:
				throw runtime_error("JSON syntax error: expected ',' or '}' after object key-value pair");
			}
			break;
		case 24:  // after comma
			switch (c) {
			case ' ': case '\t': case '\r': case '\n':
				break;
			case '"':
				in.unget();
				json_decode(in, key);
				if (key.type()!=JSON_STRING)
					throw runtime_error("JSON key not string");
				state=21;
				break;
			default:
				throw runtime_error("JSON syntax error: expected '\"' after comma in object");
			}
			break;
		}
	}

	switch (state) {

	accept_null: case 1:
		out.set(0);
		break;

	accept_true: case 2:
		out = true;
		break;

	accept_false: case 3:
		out = false;
		break;

	accept_number: case 5: case 6: case 8: case 11: {
		const char *startp=number.c_str();
		char *endp=0;
		double v = strtod(startp,&endp);
		if (startp==endp)
			throw runtime_error("JSON number does not parse");
		out = v;
		} break;

	accept_string: case 13:
		out = *str;
		delete str; str = 0;
		break;

	accept_array: case 16:
		out.set(array);
		break;
	
	accept_object: case 20:
		out.set(object);
		break;

	default:
		throw runtime_error("JSON no token");
	}
}
