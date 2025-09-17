/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 15:31:05 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/17 16:05:09 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   TYPES_HPP
#define   TYPES_HPP

#include <ostream>
#include <stdexcept>
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
		bool				operator!=(cstring) const;
		bool				operator!=(const string&) const;

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

		dynamic_array(void): items(nullptr), len(0u), cap(0u) {
		}
		~dynamic_array(void) {
		}
		dynamic_array(const dynamic_array& ref): items(ref.items), len(ref.len), cap(ref.cap) {
		}

		const V&			operator[](u64 index) const {
			if (index >= cap)
				throw std::out_of_range("bounds check failed");

			const V& item = ((V*)items)[index];
			return (item);
		}

		V&					operator[](u64 index) {
			if (index >= cap)
				throw std::out_of_range("bounds check failed");

			V& item = ((V*)items)[index];
			return (item);
		}
		dynamic_array&		operator=(const dynamic_array& ref) {
			if (this != &ref) {
				cap = ref.cap;
				len = ref.len;
				items = ref.items;
			}
			return (*this);
		}

		V&					push(const V& value) {
			if (len >= cap)
				resize((cap == 0u) ? 8u : (cap * 2u));

			V&	slot = ((V*)items)[len++];
			slot = value;

			return (slot);
		}
		void				free(void) {
			delete[]	items;

			cap = 0u;
			len = 0u;
			items = nullptr;
		}
		void				clear(void) {
			len = 0u;
		}
		void				resize(u32 cap_new) {
			byte*	items_old = items;

			byte*	items_new = new byte[cap_new * sizeof(V)]();
			for (u32 i = 0u; i < cap * (u32)sizeof(V); ++i)
				items_new[i] = items_old[i];

			cap = cap_new;
			items = items_new;
			delete[]	items_old;
		}
		V*					resize_copyless(u32 cap_new) {
			byte*	items_old = items;
			byte*	items_new = new byte[cap_new * sizeof(V)]();

			cap = cap_new;
			items = items_new;
			return (items_old);
		}
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

			iterator(hash_map& map): item(nullptr), index(0u), map(map) {
			}
			~iterator(void) {
			}

			bool	next(void) {
				while (index < map.cap) {
					size_t	cursor = index++;
					if ((bool)map.items[cursor].key) {
						item = &map.items[cursor];
						return (true);
					}
				}
				return (false);
			}
		};

	/* hash_map */
		item_slot*			items;
		u32					count;
		u32					cap;

		hash_map(void): items(nullptr), count(0u), cap(0u) {
		}
		~hash_map(void) {
		}
		hash_map(const hash_map& ref): items(ref.items), count(ref.count), cap(ref.cap) {
		}

		static u64			get_hash(const type::string& key) {
			/* algorithm: fnv_1a */
			u64	hash = 14695981039346656037ul;
			for (u32 i = 0u; i < key.len; ++i) {
				hash ^= (u64)(byte)(key[i]);
				hash ^= 1099511628211ul;
			}
			return (hash);
		}
		i64					get_index(const string& key) const {
			if (!(bool)key)
				throw std::runtime_error("empty keys are disallowed in hash_map");
			if (cap > 0u && count > 0u) {
				u64	hash = get_hash(key);
				u64	index = (u64)(hash & ((u64)cap - 1ul));
				while ((bool)items[index].key) {
					if (items[index].key == key)
						return ((i64)index);
					index = (index + 1) % (u64)cap;
				}
			}
			return (-1l);
		}
		u64					get_slot(const string& key) const {
			if (!(bool)key)
				throw std::runtime_error("empty keys are disallowed in hash_map");
			if (cap == 0ul)
				throw std::out_of_range("cannot slot into a nil hash_map");
			u64	hash = get_hash(key);
			u64	index = (u64)(hash & ((u64)cap - 1ul));
			while ((bool)items[index].key) {
				if (items[index].key == key)
					return (index);
				index = (index + 1) % (u64)cap;
			}
			return (index);
		}

		const V&			operator[](const string& key) const {
			i64	index = get_index(key);
			if (index < 0l)
				throw std::out_of_range("hash_map does not contain the provided key");
			return (items[index].value);
		}

		hash_map&			operator=(const hash_map& ref) {
			if (this != &ref) {
				cap = ref.cap;
				count = ref.count;
				items = ref.items;
			}
			return (*this);
		}
		V&					operator[](const string& key) {
			i64	index = get_index(key);
			if (index < 0l)
				throw std::out_of_range("hash_map does not contain the provided key");
			return (items[index].value);
		}

		bool				has(const string& key) const {
			return (get_index(key) >= 0l);
		}

		V&					set(const string& key, const V& value) {
			if (count >= cap / 2u) resize((cap == 0u) ? 8u : (cap * 2u));

			u64			index = get_slot(key);
			item_slot&	item = items[index];
			if (!(bool)item.key) {
				item.key = key.clone();
				count++;
			}
			item.value = value;
			return (item.value);
		}
		void				free(void) {
			iterator	iter(*this);
			for (bool ok = iter.next(); ok; ok = iter.next()) {
				type::string& key = iter.item->key;
				key.free();
			}

			cap = 0u;
			count = 0u;
			delete[] items;
		}
		void				resize(u32 cap_new) {
			iterator	iter(*this);
			hash_map	map_new;

			map_new.cap = cap_new;
			map_new.items = new item_slot[cap_new]();
			for (bool ok = iter.next(); ok; ok = iter.next()) {
				type::hash_map<V>::item_slot*	item = iter.item;
				map_new.set(item->key, item->value);
				item->key.free();
			}

			delete[] items;
			*this = map_new;
		}
		/* DISABLED(xenobas): iterator			iter(void); */
	};
};

std::ostream&	operator<<(std::ostream&, const type::string&);

#endif /* TYPES_HPP */
