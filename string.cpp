#include "types.hpp"

u32						type::cstring_len(type::cstring s) {
	u32	len = 0u;
	if (s != NULL)
		while (s[len]) ++len;
	return (len);
}

/* string */

type::string::string(void): data(nullptr), len(0u) {
}

type::string::~string(void) {
}

type::string::string(const type::string& ref): data(ref.data), len(ref.len) {
}

type::string::string(type::cstring ref): data((byte*)ref), len(type::cstring_len(ref)) {
}

type::string::string(byte* ref, u32 len): data(ref), len(len) {
}

type::string::string(type::cstring ref, u32 len): data((byte*)ref), len(len) {
	u32	ref_len = type::cstring_len(ref);
	if (ref_len < len)
		len = ref_len;
}

byte&					type::string::operator[](u64 index) {
	if (index >= (u64)len)
		throw std::out_of_range("bounds check failed");
	return (data[index]);
}

type::string&			type::string::operator=(const type::string& ref) {
	if (this != &ref) {
		len = ref.len;
		data = ref.data;
	}
	return (*this);
}

type::string&			type::string::operator=(type::cstring ref) {
	*this = string(ref);
	return (*this);
}

const byte&				type::string::operator[](u64 index) const {
	if (index >= (u64)len)
		throw std::out_of_range("bounds check failed");
	return (data[index]);
}

bool					type::string::operator==(cstring rhs) const {
	return (*this == string(rhs));
}

bool					type::string::operator==(const string& rhs) const {
	if (len != rhs.len) return (false);
	return (find(rhs) == 0);
}

bool					type::string::operator!=(cstring rhs) const {
	return (!(*this == string(rhs)));
}

bool					type::string::operator!=(const string& rhs) const {
	return (!(*this == rhs));
}

type::string::operator	bool() const {
	return (data != nullptr && len > 0u);
}

void					type::string::free(void) {
	delete[]	data;

	*this = type::string();
}

type::string			type::string::clone(void) const {
	string	str(*this);

	str.data = new byte[str.len]();
	for (u32 i = 0u; i < str.len * (u32)sizeof(char); ++i)
		str.data[i] = data[i];
	return (str);
}

type::string			type::string::slice(u32 begin) const {
	return (slice(begin, len));
}

type::string			type::string::slice(u32 begin, u32 end) const {
	end = (end < begin) ? begin : end;
	return (string((begin > len) ? nullptr : &data[begin], end - begin));
}

i32						type::string::find(byte to_find) const {
	if (to_find != '\0') {
		for (u32 i = 0u; i < len; i++)
			if (data[i] == to_find) return ((i32)i);
	}
	return (-1);
}

i32						type::string::find(const string& to_find) const {
	if (to_find.len <= len) {
		for (u32 i = 0u; i < len; ++i) {
			u32		delta = 0u;
			bool	match = true;
			while (delta < to_find.len && i + delta < len && match) {
				match = (to_find[delta] == data[i + delta]);
				delta++;
			}
			if (delta == to_find.len && match) return ((i32)i);
		}
	}
	return (-1);
}

bool					type::string::contains(const string& to_find) const {
	return (find(to_find) != -1);
}
