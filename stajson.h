//
// simple JSON parsing and dumping module
//
// - stores std::string strings
// - intended to be used with UTF-8 encoded data or single-byte encodings
// - does not preserve order of {} object members
// - JSONArray and JSONObject types are based on std::vector and std::map, and work the same way
// - no reference-counting or garbage-collection, even for object and array types
// - type errors are derived from runtime_error, since data normally comes from outside sources
// - NOT thread-safe; hold a lock before doing anything
//
#ifndef STACHART__JSON_HH__HEADER__
#define STACHART__JSON_HH__HEADER__

//#ifndef _GNU_SOURCE
//#define _GNU_SOURCE  // XXX for vasprintf (printf to allocated string)
//#endif

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>


enum JSONType {
	JSON_NULL=0,
	JSON_BOOLEAN,
	JSON_NUMBER,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
};
class JSONBase;
class JSON;
class JSONBool;
class JSONNumber;
class JSONString;
class JSONArray;
class JSONObject;


//
// data is tagged with a JSONType
//
class JSONBase {
	JSONType t;
protected:
	inline JSONBase(JSONType type) : t(type) { }
public:
	inline static void operator delete(void *p);

	inline JSONType type() const { return t; }

	inline JSONBase *copy();
};


//
// JSON is a simple pointer-holding class, with "syntactic sugar"
//
class JSON {
	JSONBase *p;
public:
	inline JSON();
	inline JSON(bool v);
	inline JSON(double v);
	inline JSON(const char *v);
	inline JSON(const std::string& v);
	inline JSON(const JSONArray& x);
	inline JSON(const JSONObject& x);
	inline JSON(const JSON& x);
	inline ~JSON();

	inline JSON& operator=(bool v);
	inline JSON& operator=(double v);
	inline JSON& operator=(const char *v);
	inline JSON& operator=(const std::string& v);
	inline JSON& operator=(const JSONArray& x);
	inline JSON& operator=(const JSONObject& x);
	inline JSON& operator=(const JSON& x);

	inline JSONBase *get();
	inline const JSONBase *get() const;
	inline void set(JSONBase *pp);  // takes ownership; do not delete @pp later; previous @p is deleted

	inline JSONType type() const;

	inline JSONBool& boolean();
	inline const JSONBool& boolean() const;
	inline JSONNumber& number();
	inline const JSONNumber& number() const;
	inline JSONString& string();
	inline const JSONString& string() const;
	inline JSONArray& array();
	inline const JSONArray& array() const;
	inline const JSONObject& object() const;
	inline JSONObject& object();

	inline bool has(int i);
	inline bool has(size_t i);
	inline bool has(const char *s);
	inline bool has(const std::string& s);

	inline JSON& get(int i);
	inline const JSON& get(int i) const;
	inline JSON& get(size_t i);
	inline const JSON& get(size_t i) const;
	inline JSON& get(const char *s);
	inline const JSON& get(const char *s) const;
	inline JSON& get(const std::string& s);
	inline const JSON& get(const std::string& s) const;

	inline JSON& operator[](int i);
	inline JSON& operator[](size_t i);
	inline JSON& operator[](const char *s);
	inline JSON& operator[](const std::string& s);

	inline operator bool() const;
	inline operator double() const;
	inline operator std::string() const;
};


//
// the 5 JSON types
//
class JSONBool : public JSONBase {
	bool v;
public:
	inline JSONBool() : JSONBase(JSON_BOOLEAN), v(false) { }
	inline JSONBool(bool value) : JSONBase(JSON_BOOLEAN), v(value) { }
	inline JSONBool(const JSONBool& x) : JSONBase(JSON_BOOLEAN), v(x.v) { }

	inline bool value() const { return v; }
	inline operator bool() const { return v; }
};
typedef JSONBool JSONBoolean;

class JSONNumber : public JSONBase {
	double v;
public:
	inline JSONNumber() : JSONBase(JSON_NUMBER), v(0.0) { }
	inline JSONNumber(double value) : JSONBase(JSON_NUMBER), v(value) { }
	inline JSONNumber(const JSONNumber& x) : JSONBase(JSON_NUMBER), v(x.v) { }

	inline double value() const { return v; }
	inline operator double() const { return v; }
};

class JSONString : public JSONBase {
	std::string v;
public:
	inline JSONString() : JSONBase(JSON_STRING), v("") { }
	inline JSONString(const JSONString& x) : JSONBase(JSON_STRING), v(x.v) { }
	inline JSONString(const char *value) : JSONBase(JSON_STRING), v(value) { }
	inline JSONString(const std::string& value) : JSONBase(JSON_STRING), v(value) { }

	inline operator const char*() const { return v.c_str(); }
	inline const std::string& value() const { return v; }
	inline operator const std::string&() const { return v; }

	inline const char *c_str() const { return v.c_str(); }
};

class JSONArray : public JSONBase {
	std::vector<JSON> v;
public:
	typedef JSON value_type;
	typedef value_type *pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::vector<JSON>::size_type size_type;
	typedef std::vector<JSON>::difference_type difference_type;
private:
	template <typename ITER, typename ELEM>
	class iterator_t {
		friend class JSONArray;

		typedef iterator_t<ITER,ELEM> iterator; 

		ITER i;
		iterator_t(const ITER& x) : i(x) { }
	public:
		iterator_t() { }
		iterator_t(const iterator& x) : i(x.i) { }

		ELEM& operator*() const { return *i; }
		ELEM *operator->() const { return &*i; }

		bool operator==(const iterator& x) const { return i==x.i; }
		bool operator!=(const iterator& x) const { return i!=x.i; }

		iterator& operator++() { ++i; return *this; }
		iterator operator++(int) { return i++; }
		iterator& operator--() { --i; return *this; }
		iterator operator--(int) { return i--; }

		iterator operator+(size_type n) const { return i+n; }
		iterator operator-(size_type n) const { return i-n; }
		difference_type operator-(const iterator& x) const { return i-x.i; }
		iterator& operator+=(size_type n) { i+=n; return *this; }
		iterator& operator-=(size_type n) { i-=n; return *this; }

		ELEM& operator[](size_type n) const { return i[n]; }

		void swap(iterator& x) { swap(i,x.i); }
	};
public:
	typedef iterator_t<std::vector<JSON>::iterator,JSON> iterator;
	typedef iterator_t<std::vector<JSON>::reverse_iterator,JSON> reverse_iterator;
	typedef iterator_t<std::vector<JSON>::const_iterator,const JSON> const_iterator;
	typedef iterator_t<std::vector<JSON>::const_reverse_iterator,const JSON> const_reverse_iterator;

	inline JSONArray() : JSONBase(JSON_ARRAY) { }
	inline JSONArray(size_type n) : JSONBase(JSON_ARRAY), v(n) { }
	inline JSONArray(size_type n, const_reference t) : JSONBase(JSON_ARRAY), v(n, t) { }
	inline JSONArray(const JSONArray& x) : JSONBase(JSON_ARRAY), v(x.v) { }
	template <class InputIterator>
	inline JSONArray(InputIterator a, InputIterator b) : JSONBase(JSON_ARRAY), v(a,b) { } 

	inline JSONArray& operator=(const JSONArray& x) { v=x.v; return *this; }

	inline iterator begin() { return v.begin(); }
	inline iterator end() { return v.end(); }
	inline const_iterator begin() const { return v.begin(); }
	inline const_iterator end() const { return v.end(); }
	inline reverse_iterator rbegin() { return v.rbegin(); }
	inline reverse_iterator rend() { return v.rend(); }
	inline const_reverse_iterator rbegin() const { return v.rbegin(); }
	inline const_reverse_iterator rend() const { return v.rend(); }

	inline size_type size() const { return v.size(); }
	inline size_type max_size() const { return v.max_size(); }
	inline size_type capacity() const { return v.capacity(); }
	inline bool empty() const { return v.empty(); }
	inline reference operator[](size_type n) { return v[n]; }
	inline const_reference operator[](size_type n) const { return v[n]; }
	inline reference get(size_type n) { return v.at(n); }
	inline const_reference get(size_type n) const { return v.at(n); }
	inline reference at(size_type n) { return v.at(n); }
	inline const_reference at(size_type n) const { return v.at(n); }
	inline void reserve(size_type n) { v.reserve(n); }
	inline reference front() { return v.front(); }
	inline const_reference front() const { return v.front(); }
	inline reference back() { return v.back(); }
	inline const_reference back() const { return v.back(); }
	inline void push_back(const_reference x) { v.push_back(x); }
	inline void pop_back() { v.pop_back(); }
	inline void swap(JSONArray& x) { v.swap(x.v); }

	inline iterator insert(iterator pos, bool x) { return v.insert(pos.i, JSON(x)); }
	inline iterator insert(iterator pos, double x) { return v.insert(pos.i, JSON(x)); }
	inline iterator insert(iterator pos, const char *x) { return v.insert(pos.i, JSON(x)); }
	inline iterator insert(iterator pos, const std::string& x) { return v.insert(pos.i, JSON(x)); }
	inline iterator insert(iterator pos, const_reference x) { return v.insert(pos.i, x); }
	inline void insert(iterator pos, size_type n, const_reference x) { v.insert(pos.i, n, x); }
	template <class InputIterator>
	inline iterator insert(iterator pos, InputIterator f, InputIterator l) { return v.insert(pos.i, f, l); }

	inline iterator erase(iterator pos) { return v.erase(pos.i); }
	inline iterator erase(iterator f, iterator l) { return v.erase(f.i,l.i); }

	inline bool operator==(const JSONArray& x) const { return v==x.v; }
	inline bool operator<(const JSONArray& x) const { return v<x.v; }
};

class JSONObject : public JSONBase {
	std::map<std::string,JSON> v;
public:
	typedef std::string key_type;
	typedef JSON data_type;
	typedef std::pair<const key_type,data_type> value_type;
	typedef value_type *pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::map<std::string,JSON>::size_type size_type;
	typedef std::map<std::string,JSON>::difference_type difference_type;
private:
	template <typename ITER, typename ELEM>
	class iterator_t {
		friend class JSONObject;

		typedef iterator_t<ITER,ELEM> iterator; 

		ITER i;
		iterator_t(const ITER& x) : i(x) { }
	public:
		iterator_t() { }
		iterator_t(const iterator& x) : i(x.i) { }

		ELEM& operator*() const { return *i; }
		ELEM *operator->() const { return &*i; }

		bool operator==(const iterator& x) const { return i==x.i; }
		bool operator!=(const iterator& x) const { return i!=x.i; }

		iterator& operator++() { ++i; return *this; }
		iterator operator++(int) { return i++; }
		iterator& operator--() { --i; return *this; }
		iterator operator--(int) { return i--; }

		void swap(iterator& x) { swap(i,x.i); }
	};
public:
	typedef iterator_t<std::map<std::string,JSON>::iterator,value_type> iterator;
	typedef iterator_t<std::map<std::string,JSON>::reverse_iterator,value_type> reverse_iterator;
	typedef iterator_t<std::map<std::string,JSON>::const_iterator,const value_type> const_iterator;
	typedef iterator_t<std::map<std::string,JSON>::const_reverse_iterator,const value_type> const_reverse_iterator;

	inline JSONObject() : JSONBase(JSON_OBJECT) { }
	inline JSONObject(const JSONObject& x) : JSONBase(JSON_OBJECT), v(x.v) { }
	template <class InputIterator>
	inline JSONObject(InputIterator a, InputIterator b) : JSONBase(JSON_OBJECT), v(a,b) { } 

	inline JSONObject& operator=(const JSONObject& x) { v=x.v; return *this; }
	inline void swap(JSONObject& x) { v.swap(x.v); }

	inline iterator begin() { return v.begin(); }
	inline iterator end() { return v.end(); }
	inline const_iterator begin() const { return v.begin(); }
	inline const_iterator end() const { return v.end(); }
	inline reverse_iterator rbegin() { return v.rbegin(); }
	inline reverse_iterator rend() { return v.rend(); }
	inline const_reverse_iterator rbegin() const { return v.rbegin(); }
	inline const_reverse_iterator rend() const { return v.rend(); }

	inline size_type size() const { return v.size(); }
	inline size_type max_size() const { return v.max_size(); }
	inline bool empty() const { return v.empty(); }

	inline std::pair<iterator,bool> insert(const_reference x) {
		std::pair<std::map<std::string,JSON>::iterator,bool> p = v.insert(x);
		return std::pair<iterator,bool>(p.first,p.second);
	}
	inline iterator insert(iterator pos, const_reference x) { return v.insert(pos.i, x); }
	template <class InputIterator>
	inline void insert(InputIterator f, InputIterator l) { v.insert(f, l); }

	inline void erase(iterator pos) { v.erase(pos.i); }
	inline size_type erase(const key_type& k) { return v.erase(k); }
	inline void erase(iterator f, iterator l) { v.erase(f.i,l.i); }
	inline void clear() { v.clear(); }

	inline iterator find(const key_type& k) { return v.find(k); }
	inline const_iterator find(const key_type& k) const { return v.find(k); }
	inline size_type count(const key_type& k) const { return v.count(k); }
	inline bool has(const key_type& s) const { return v.find(s)!=v.end(); }

	inline iterator lower_bound(const key_type& k) { return v.lower_bound(k); }
	inline const_iterator lower_bound(const key_type& k) const { return v.lower_bound(k); }
	inline iterator upper_bound(const key_type& k) { return v.upper_bound(k); }
	inline const_iterator upper_bound(const key_type& k) const { return v.upper_bound(k); }
	inline std::pair<iterator,iterator> equal_range(const key_type& k) { return equal_range(k); }
	inline std::pair<const_iterator,const_iterator> equal_range(const key_type& k) const { return equal_range(k); }

	inline JSON& get(const key_type& s);
	inline const JSON& get(const key_type& s) const;

	inline data_type& operator[](const key_type& k) { return v[k]; }

	inline bool operator==(const JSONObject& x) const { return v==x.v; }
	inline bool operator<(const JSONObject& x) const { return v<x.v; }
};


class json_type_error : public std::runtime_error {
public:
	json_type_error(const std::string& s) : std::runtime_error(s) { }
};


////////////////////////////////////////////////////////////////////////////////


void json_encode(const JSON& in, std::string& out);
inline std::string json_encode(const JSON& in) { std::string out; json_encode(in,out); return out; }
void json_encode(const JSON& in, std::ostream& out);
inline std::ostream& operator<<(std::ostream& out, const JSON& in) { json_encode(in,out); return out; }

void json_encode(const JSONBool& in, std::string& out);
inline std::string json_encode(const JSONBool& in) { std::string out; json_encode(in,out); return out; }
void json_encode(const JSONBool& in, std::ostream& out);
inline std::ostream& operator<<(std::ostream& out, const JSONBool& in) { json_encode(in,out); return out; }

void json_encode(const JSONNumber& in, std::string& out);
inline std::string json_encode(const JSONNumber& in) { std::string out; json_encode(in,out); return out; }
void json_encode(const JSONNumber& in, std::ostream& out);
inline std::ostream& operator<<(std::ostream& out, const JSONNumber& in) { json_encode(in,out); return out; }

void json_encode(const JSONString& in, std::string& out);
inline std::string json_encode(const JSONString& in) { std::string out; json_encode(in,out); return out; }
void json_encode(const JSONString& in, std::ostream& out);
inline std::ostream& operator<<(std::ostream& out, const JSONString& in) { json_encode(in,out); return out; }

void json_encode(const JSONArray& in, std::string& out);
inline std::string json_encode(const JSONArray& in) { std::string out; json_encode(in,out); return out; }
void json_encode(const JSONArray& in, std::ostream& out);
inline std::ostream& operator<<(std::ostream& out, const JSONArray& in) { json_encode(in,out); return out; }

void json_encode(const JSONObject& in, std::string& out);
inline std::string json_encode(const JSONObject& in) { std::string out; json_encode(in,out); return out; }
void json_encode(const JSONObject& in, std::ostream& out);
inline std::ostream& operator<<(std::ostream& out, const JSONObject& in) { json_encode(in,out); return out; }

void json_decode(const std::string& in, JSON& out);
inline JSON json_decode(const std::string& in) { JSON out; json_decode(in,out); return out; }
void json_decode(std::istream& in, JSON& out);
inline std::istream& operator>>(std::istream& in, JSON& out) { json_decode(in,out); return in; }

inline static std::string json_fmt(const char *fmt, ...);


//
// JSONBase
//
inline static void JSONBase::operator delete(void *p) {
	if (p) switch ( ((JSONBase*)p)->t ) {
	case JSON_NULL: break;
	case JSON_BOOLEAN: ::delete ((JSONBool*)p); break;
	case JSON_NUMBER: ::delete ((JSONNumber*)p); break;
	case JSON_STRING: ::delete ((JSONString*)p); break;
	case JSON_ARRAY: ::delete ((JSONArray*)p); break;
	case JSON_OBJECT: ::delete ((JSONObject*)p); break;
	}
}
inline JSONBase *JSONBase::copy() {
	switch (t) {
	case JSON_NULL:
		return 0;
	case JSON_BOOLEAN:
		return new JSONBool(*(JSONBool*)this);
	case JSON_NUMBER:
		return new JSONNumber(*(JSONNumber*)this);
	case JSON_STRING:
		return new JSONString(*(JSONString*)this);
	case JSON_ARRAY:
		return new JSONArray(*(JSONArray*)this);
	case JSON_OBJECT:
		return new JSONObject(*(JSONObject*)this);
	default:
		throw std::logic_error("corrupt JSON object");
	}
}


//
// JSON
//
inline JSON::JSON() : p(0) {
}
inline JSON::JSON(bool v) : p(new JSONBool(v)) {
}
inline JSON::JSON(double v) : p(new JSONNumber(v)) {
}
inline JSON::JSON(const char *v) : p(new JSONString(v)) {
}
inline JSON::JSON(const std::string& v) : p(new JSONString(v)) {
}
inline JSON::JSON(const JSONArray& v) : p(new JSONArray(v)) {
}
inline JSON::JSON(const JSONObject& v) : p(new JSONObject(v)) {
}
inline JSON::JSON(const JSON& x) : p(x.p ? x.p->copy() : 0) {
}
inline JSON::~JSON() {
	if (p) delete p;
}
inline JSON& JSON::operator=(bool v) {
	if (p) delete p;
	p = new JSONBool(v);
	return *this;
}
inline JSON& JSON::operator=(double v) {
	if (p) delete p;
	p = new JSONNumber(v);
	return *this;
}
inline JSON& JSON::operator=(const char *v) {
	if (p) delete p;
	p = new JSONString(v);
	return *this;
}
inline JSON& JSON::operator=(const std::string& v) {
	if (p) delete p;
	p = new JSONString(v);
	return *this;
}
inline JSON& JSON::operator=(const JSONArray& v) {
	if (p) delete p;
	p = new JSONArray(v);
	return *this;
}
inline JSON& JSON::operator=(const JSONObject& v) {
	if (p) delete p;
	p = new JSONObject(v);
	return *this;
}
inline JSON& JSON::operator=(const JSON& x) {
	if (p) delete p;
	p = x.p ? x.p->copy() : 0;
	return *this;
}

inline JSONBase *JSON::get() {
	return p;
}
inline const JSONBase *JSON::get() const {
	return p;
}
inline void JSON::set(JSONBase *pp) {  // takes ownership
	if (p) delete p;
	p = pp;
}

inline JSONType JSON::type() const {
	return p ? p->type() : JSON_NULL;
}

inline JSONBool& JSON::boolean() {
	if (type()!=JSON_BOOLEAN) throw json_type_error("not a JSON boolean");
	return *(JSONBool*)p;
}
inline const JSONBool& JSON::boolean() const {
	if (type()!=JSON_BOOLEAN) throw json_type_error("not a JSON boolean");
	return *(JSONBool*)p;
}
inline JSONNumber& JSON::number() {
	if (type()!=JSON_NUMBER) throw json_type_error("not a JSON number");
	return *(JSONNumber*)p;
}
inline const JSONNumber& JSON::number() const {
	if (type()!=JSON_NUMBER) throw json_type_error("not a JSON number");
	return *(JSONNumber*)p;
}
inline JSONString& JSON::string() {
	if (type()!=JSON_STRING) throw json_type_error("not a JSON string");
	return *(JSONString*)p;
}
inline const JSONString& JSON::string() const {
	if (type()!=JSON_STRING) throw json_type_error("not a JSON string");
	return *(JSONString*)p;
}
inline JSONArray& JSON::array() {
	if (type()!=JSON_ARRAY) throw json_type_error("not a JSON array");
	return *(JSONArray*)p;
}
inline const JSONArray& JSON::array() const {
	if (type()!=JSON_ARRAY) throw json_type_error("not a JSON array");
	return *(JSONArray*)p;
}
inline const JSONObject& JSON::object() const {
	if (type()!=JSON_OBJECT) throw json_type_error("not a JSON object");
	return *(JSONObject*)p;
}
inline JSONObject& JSON::object() {
	if (type()!=JSON_OBJECT) throw json_type_error("not a JSON object");
	return *(JSONObject*)p;
}

inline JSON& JSON::operator[](int i) {
	return array()[i];
}
inline JSON& JSON::operator[](size_t i) {
	return array()[i];
}
inline JSON& JSON::operator[](const char *s) {
	return object()[s];
}
inline JSON& JSON::operator[](const std::string& s) {
	return object()[s];
}
inline bool JSON::has(int i) {
	return i>=0 && array().size()>(size_t)i;
}
inline bool JSON::has(size_t i) {
	return array().size()>i;
}
inline bool JSON::has(const char *s) {
	return object().has(s);
}
inline bool JSON::has(const std::string& s) {
	return object().has(s);
}
inline JSON& JSON::get(int i) {
	return array().at(i);
}
inline const JSON& JSON::get(int i) const {
	return array().at(i);
}
inline JSON& JSON::get(size_t i) {
	return array().at(i);
}
inline const JSON& JSON::get(size_t i) const {
	return array().at(i);
}
inline JSON& JSON::get(const char *s) {
	JSONObject& o = object();
	JSONObject::iterator i = o.find(s);
	if (i==o.end()) throw std::out_of_range(json_fmt("JSON object has no element %s: %s", json_encode(JSONString(s)).c_str(), json_encode(o).c_str()));
	return i->second;
}
inline const JSON& JSON::get(const char *s) const {
	const JSONObject& o = object();
	JSONObject::const_iterator i = o.find(s);
	if (i==o.end()) throw std::out_of_range(json_fmt("JSON object has no element %s: %s", json_encode(JSONString(s)).c_str(), json_encode(o).c_str()));
	return i->second;
}
inline JSON& JSON::get(const std::string& s) {
	JSONObject& o = object();
	JSONObject::iterator i = o.find(s);
	if (i==o.end()) throw std::out_of_range(json_fmt("JSON object has no element %s: %s", json_encode(JSONString(s)).c_str(), json_encode(o).c_str()));
	return i->second;
}
inline const JSON& JSON::get(const std::string& s) const {
	const JSONObject& o = object();
	JSONObject::const_iterator i = o.find(s);
	if (i==o.end()) throw std::out_of_range(json_fmt("JSON object has no element %s: %s", json_encode(JSONString(s)).c_str(), json_encode(o).c_str()));
	return i->second;
}

// to make this module independent
inline static std::string json_fmt(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char *s = 0; vasprintf(&s, fmt, args);
	va_end(args);

	std::string ss(s);
	free(s);
	return ss;
}
inline static double json_strtod(const char *str) {
	char *endp=0;
	double v = ::strtod(str,&endp);
	if (str==endp)
		throw std::invalid_argument(json_fmt("invalid floating-point value: %s", str));
	while (*endp) {
		if (!isspace(*endp))
			throw std::invalid_argument(json_fmt("invalid floating-point value: %s", str));
		++endp;
	}
	return v;
}

inline JSON::operator bool() const {
	switch (type()) {
	case JSON_NULL: return false;
	case JSON_BOOLEAN: return ((JSONBool*)p)->value();
	case JSON_NUMBER: return ((JSONNumber*)p)->value() != 0.0;
	case JSON_STRING: return ((JSONString*)p)->value().length() != 0;
	case JSON_ARRAY: throw json_type_error("JSON array cannot be converted to type bool");
	case JSON_OBJECT: default: throw json_type_error("JSON object cannot be converted to type bool");
	}
}
inline JSON::operator double() const {
	switch (type()) {
	case JSON_NULL: return 0.0;
	case JSON_BOOLEAN: return ((JSONBool*)p)->value() ? 1.0 : 0.0;
	case JSON_NUMBER: return ((JSONNumber*)p)->value();
	case JSON_STRING: return json_strtod( ((JSONString*)p)->value().c_str() );
	case JSON_ARRAY: throw json_type_error("JSON array cannot be converted to type double");
	case JSON_OBJECT: default: throw json_type_error("JSON object cannot be converted to type double");
	}
}
inline JSON::operator std::string() const {
	switch (type()) {
	case JSON_NULL: throw json_type_error("JSON NULL cannot be converted to type string");
	case JSON_BOOLEAN: throw json_type_error("JSON boolean cannot be converted to type string");
	case JSON_NUMBER: return json_fmt("%.15g", ((JSONNumber*)p)->value());
	case JSON_STRING: return ((JSONString*)p)->value();
	case JSON_ARRAY: throw json_type_error("JSON array cannot be converted to type string");
	case JSON_OBJECT: default: throw json_type_error("JSON object cannot be converted to type string");
	}
}


//
// JSONObject
//
inline JSON& JSONObject::get(const JSONObject::key_type& s) {
	iterator i = find(s);
	if (i==end()) throw std::out_of_range(json_fmt("JSON object has no element %s: %s", json_encode(JSONString(s)).c_str(), json_encode(*this).c_str()));
	return i->second;
}
inline const JSON& JSONObject::get(const JSONObject::key_type& s) const {
	const_iterator i = find(s);
	if (i==end()) throw std::out_of_range(json_fmt("JSON object has no element %s: %s", json_encode(JSONString(s)).c_str(), json_encode(*this).c_str()));
	return i->second;
}


//
// operations with JSON* objects
//
template <typename ITER, typename ELEM>
inline void swap(JSONArray::iterator& a, JSONArray::iterator& b) {
	a.swap(b);
}

template <typename ITER, typename ELEM>
inline void swap(JSONObject::iterator& a, JSONObject::iterator& b) {
	a.swap(b);
}

inline bool operator==(const JSON& a, bool b) {
	return a.type()==JSON_BOOLEAN && ((JSONBool*)a.get())->value()==b;
}
inline bool operator==(bool b, const JSON& a) {
	return a==b;
}
inline bool operator==(const JSON& a, double b) {
	return a.type()==JSON_NUMBER && ((JSONNumber*)a.get())->value()==b;
}
inline bool operator==(double b, const JSON& a) {
	return a==b;
}
inline bool operator==(const JSON& a, const std::string& b) {
	return a.type()==JSON_STRING && ((JSONString*)a.get())->value()==b;
}
inline bool operator==(const std::string& b, const JSON& a) {
	return a==b;
}
inline bool operator==(const JSON& a, const char *b) {
	return a.type()==JSON_STRING && ((JSONString*)a.get())->value()==b;
}
inline bool operator==(const char *b, const JSON& a) {
	return a==b;
}
inline bool operator==(const JSON& a, const JSON& b) {
	if (a.type()==b.type()) {
		switch (a.type()) {
		case JSON_NULL: return true;
		case JSON_BOOLEAN: return ((JSONBool*)a.get())->value() == ((JSONBool*)b.get())->value();
		case JSON_NUMBER: return ((JSONNumber*)a.get())->value() == ((JSONNumber*)b.get())->value();
		case JSON_STRING: return ((JSONString*)a.get())->value() == ((JSONString*)b.get())->value();
		case JSON_ARRAY: return (*(JSONArray*)a.get()) == (*(JSONArray*)b.get());
		case JSON_OBJECT: return (*(JSONObject*)a.get()) == (*(JSONObject*)b.get());
		}
	} else {
		return false;
	}
}

inline bool operator!=(const JSON& a, bool b) {
	return !(a==b);
}
inline bool operator!=(bool b, const JSON& a) {
	return !(a==b);
}
inline bool operator!=(const JSON& a, double b) {
	return !(a==b);
}
inline bool operator!=(double b, const JSON& a) {
	return !(a==b);
}
inline bool operator!=(const JSON& a, const std::string& b) {
	return !(a==b);
}
inline bool operator!=(const std::string& b, const JSON& a) {
	return !(a==b);
}
inline bool operator!=(const JSON& a, const char *b) {
	return !(a==b);
}
inline bool operator!=(const char *b, const JSON& a) {
	return !(a==b);
}
inline bool operator!=(const JSON& a, const JSON& b) {
	return !(a==b);
}

inline bool operator<(const JSON& a, const JSON& b) {
	if (a.type()==b.type()) {
		switch (a.type()) {
		case JSON_NULL: return false;
		case JSON_BOOLEAN: return ((JSONBool*)a.get())->value() < ((JSONBool*)b.get())->value();
		case JSON_NUMBER: return ((JSONNumber*)a.get())->value() < ((JSONNumber*)b.get())->value();
		case JSON_STRING: return ((JSONString*)a.get())->value() < ((JSONString*)b.get())->value();
		case JSON_ARRAY: return (*(JSONArray*)a.get()) < (*(JSONArray*)b.get());
		case JSON_OBJECT: return (*(JSONObject*)a.get()) < (*(JSONObject*)b.get());
		}
	} else {
		throw json_type_error("incompatible JSONBase types in comparison");
	}
}
inline bool operator>(const JSON& b, const JSON& a) {
	return a<b;
}
inline bool operator<=(const JSON& a, const JSON& b) {
	return !(b<a);
}
inline bool operator>=(const JSON& a, const JSON& b) {
	return !(a<b);
}

inline bool operator==(const JSONObject& a, const JSONObject& b) {
	return a.operator==(b);
}
inline bool operator<(const JSONObject& a, const JSONObject& b) {
	return a.operator<(b);
}

#endif
