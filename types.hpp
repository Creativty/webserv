/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 15:31:05 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 15:00:16 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   TYPES_HPP
#define   TYPES_HPP

#include <ostream>
#include <stdint.h>

#if __cplusplus <= 199711L
#define nullptr 0
#endif

/* Base types */

typedef uint8_t				byte;

typedef uint8_t				u8;
typedef uint16_t			u16;
typedef uint32_t			u32;
typedef uint64_t			u64;

typedef int8_t				i8;
typedef int16_t				i16;
typedef int32_t				i32;
typedef int64_t				i64;

/* Complex types */
namespace type {
typedef const char*		cstring;
u32						cstring_len(cstring s);

struct string {
	byte*				data;
	u32					len;

	string(void);
	~string(void);
	string(const string&);
	string(cstring);
	string(byte*, u32);
	string(cstring, u32);

	byte&				operator[](u64);
	string&				operator=(const string&);
	string&				operator=(cstring str);

	operator			bool() const;
	const byte&			operator[](u64) const;
	bool				operator==(cstring) const;
	bool				operator==(const string&) const;

	void				free(void);
	string				clone(void) const;

	string				slice(u32) const;
	string				slice(u32, u32) const;

	i32					find(byte) const;
	i32					find(const string&) const;
	bool				contains(const string&) const;
};

template <typename V>
struct dynamic_array {
	byte*				items;
	u32					len;
	u32					cap;

	dynamic_array(void);
	~dynamic_array(void);
	dynamic_array(const dynamic_array&);

	const V&			operator[](u64) const;

	V&					operator[](u64);
	dynamic_array&		operator=(const dynamic_array&);

	V&					push(const V&);
	void				free(void);
	void				clear(void);
	void				resize(u32);
	V*					resize_copyless(u32);
};

template <typename V>
struct hash_map {
	struct item_slot {
		string			key;
		V				value;
	};
	struct iterator {
		item_slot*		item;
		u32				index;
		hash_map&		map;

		iterator(hash_map& map);
		~iterator(void);

		bool	next(void);
	};

	item_slot*			items;
	u32					count;
	u32					cap;

	hash_map(void);
	~hash_map(void);
	hash_map(const hash_map&);

	const V&			operator[](const string&) const;

	hash_map&			operator=(const hash_map&);
	V&					operator[](const string&);

	bool				has(const string&) const ;

	V&					set(const string&, const V&);
	void				free(void);
	void				resize(u32);
	/* DISABLED(xenobas): iterator			iter(void); */
};
};

std::ostream&	operator<<(std::ostream&, const type::string&);

#ifdef    TYPES_HPP_IMPL
#include <stdexcept>

namespace type {
/* cstring */

u32						cstring_len(cstring s) {
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

/* dynamic_array */

template <typename V>
type::dynamic_array<V>::dynamic_array(void): items(nullptr), len(0u), cap(0u) {
}

template <typename V>
type::dynamic_array<V>::~dynamic_array(void) {
}

template <typename V>
type::dynamic_array<V>::dynamic_array(const dynamic_array& ref): items(ref.items), len(ref.len), cap(ref.cap) {
}

template <typename V>
const V&				type::dynamic_array<V>::operator[](u64 index) const {
	if (index >= cap)
		throw std::out_of_range("bounds check failed");

	const V& item = ((V*)items)[index];
	return (item);
}

template <typename V>
V&						type::dynamic_array<V>::operator[](u64 index) {
	if (index >= cap)
		throw std::out_of_range("bounds check failed");

	V& item = ((V*)items)[index];
	return (item);
}

template <typename V>
type::dynamic_array<V>&	type::dynamic_array<V>::operator=(const dynamic_array& ref) {
	if (this != &ref) {
		cap = ref.cap;
		len = ref.len;
		items = ref.items;
	}
	return (*this);
}

template <typename V>
V&						type::dynamic_array<V>::push(const V& value) {
	if (len >= cap)
		resize((cap == 0u) ? 8u : (cap * 2u));

	V&	slot = ((V*)items)[len++];
	slot = value;

	return (slot);
}

template <typename V>
void				type::dynamic_array<V>::free(void) {
	delete[]	items;

	cap = 0u;
	len = 0u;
	items = nullptr;
}

template <typename V>
void				type::dynamic_array<V>::clear(void) {
	len = 0u;
}

template <typename V>
void				type::dynamic_array<V>::resize(u32 cap_new) {
	byte*	items_old = items;

	byte*	items_new = new byte[cap_new * sizeof(V)]();
	for (u32 i = 0u; i < cap * (u32)sizeof(V); ++i)
		items_new[i] = items_old[i];

	cap = cap_new;
	items = items_new;
	delete[]	items_old;
}

template <typename V>
V*					type::dynamic_array<V>::resize_copyless(u32 cap_new) {
	byte*	items_old = items;
	byte*	items_new = new byte[cap_new * sizeof(V)]();

	cap = cap_new;
	items = items_new;
	return (items_old);
}

/* hash_map */

u64			hash_map_hash(const type::string& key) {
	/* algorithm: fnv_1a */
	u64	hash = 14695981039346656037ul;
	for (u32 i = 0; i < key.len; i++) {
		hash ^= (u64)(byte)(key[i]);
		hash ^= 1099511628211ul;
	}
	return (hash);
}

template <typename V>
i64			hash_map_index(const type::hash_map<V>& map, const type::string& key) {
	if (!(bool)key)
		throw std::runtime_error("empty keys are disallowed in hash_map");
	if (map.cap > 0u && map.count > 0u) {
		u64	hash = hash_map_hash(key);
		u64	index = (u64)(hash & ((u64)map.cap - 1ul));
		while ((bool)map.items[index].key) {
			if (map.items[index].key == key)
				return ((i64)index);
			index = (index + 1) % (u64)map.cap;
		}
	}
	return (-1l);
}

template <typename V>
u64			hash_map_slot(const type::hash_map<V>& map, const type::string& key) {
	if (!(bool)key)
		throw std::runtime_error("empty keys are disallowed in hash_map");
	if (map.cap == 0ul)
		throw std::out_of_range("cannot slot into an empty hash_map");
	u64	hash = hash_map_hash(key);
	u64	index = (u64)(hash & ((u64)map.cap - 1ul));
	while ((bool)map.items[index].key) {
		if (map.items[index].key == key)
			return (index);
		index = (index + 1) % (u64)map.cap;
	}
	return (index);
}

template <typename V>
type::hash_map<V>::hash_map(void): items(nullptr), count(0u), cap(0u) {
}

template <typename V>
type::hash_map<V>::~hash_map(void) {
}

template <typename V>
type::hash_map<V>::hash_map(const hash_map& ref): items(ref.items), count(ref.count), cap(ref.cap) {
}

template <typename V>
const V&			type::hash_map<V>::operator[](const string& key) const {
	i64	index = hash_map_index(*this, key);
	if (index < 0l)
		throw std::out_of_range("hash_map does not contain the provided key");
	return (items[index].value);
}

template <typename V>
type::hash_map<V>&	type::hash_map<V>::operator=(const hash_map& ref) {
	if (this != &ref) {
		cap = ref.cap;
		count = ref.count;
		items = ref.items;
	}
	return (*this);
}

template <typename V>
V&					type::hash_map<V>::operator[](const string& key) {
	i64	index = hash_map_index(*this, key);
	if (index < 0l)
		throw std::out_of_range("hash_map does not contain the provided key");
	return (items[index].value);
}

template <typename V>
bool				type::hash_map<V>::has(const string& key) const {
	i64	index = hash_map_index(*this, key);
	return (index >= 0l);
}

template <typename V>
V&					type::hash_map<V>::set(const string& key, const V& value) {
	if (count >= cap / 2u) resize((cap == 0u) ? 8u : (cap * 2u));

	u64			index = hash_map_slot(*this, key);
	item_slot&	item = items[index];
	if (!(bool)item.key) {
		item.key = key.clone();
		count++;
	}
	item.value = value;
	return (item.value);
}

template <typename V>
void				type::hash_map<V>::free(void) {
	iterator	iter(*this);
	for (bool ok = iter.next(); ok; ok = iter.next()) {
		type::string& key = iter.item->key;
		key.free();
	}

	cap = 0u;
	count = 0u;
	delete[] items;
}

template <typename V>
void				type::hash_map<V>::resize(u32 cap_new) {
	type::hash_map<V>					map_new;
	map_new.cap = cap_new;
	map_new.items = new type::hash_map<V>::item_slot[cap_new]();

	iterator	iter(*this);
	for (bool ok = iter.next(); ok; ok = iter.next()) {
		type::hash_map<V>::item_slot*	item = iter.item;
		map_new.set(item->key, item->value);
		item->key.free();
	}

	delete[] items;
	*this = map_new;
}

template <typename V>
type::hash_map<V>::iterator::iterator(type::hash_map<V>& map): item(nullptr), index(0u), map(map) {
}

template <typename V>
type::hash_map<V>::iterator::~iterator(void) {
}

template <typename V>
bool				type::hash_map<V>::iterator::next(void) {
	while (index < map.cap) {
		size_t	cursor = index++;
		if ((bool)map.items[cursor].key) {
			item = &map.items[cursor];
			return (true);
		}
	}
	return (false);
}
}

std::ostream&	operator<<(std::ostream& stream, const type::string& str) {
	if (str.data == nullptr) return (stream << "(null)");

	stream << '"';
	for (u32 i = 0u; i < str.len; ++i) {
		switch (str.data[i]) {
			case '\a': {
				stream << "\\a";
			} break;
			case '\b': {
				stream << "\\b";
			} break;
			case '\t': {
				stream << "\\t";
			} break;
			case '\n': {
				stream << "\\n";
			} break;
			case '\0': {
				stream << "\\0";
			} break;
			default: {
				stream << str.data[i];
			} break;
		}
	}
	stream << '"';
	return (stream);
}

#endif /* TOML_IMPL_HPP */

#endif /* TYPES_HPP */
