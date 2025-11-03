/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hash_table.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 17:16:15 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/03 15:39:44 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   HASH_TABLE_HPP
#define   HASH_TABLE_HPP
#include "base.hpp"
template <typename T>
struct hash_table {
	struct item {
		T			value;
		string_view	key;

		item(string_view key, T value): value(value), key(string_view::alloc(key)) { };
		item(void): value(), key() { };
		~item(void) { };
		item(const item& item): value(item.value), key(item.key) { };
		item& operator=(const item& item) {
			if (this != &item) {
				this->value = item.value;
				this->key = item.key;
			}
			return (*this);
		};

		bool		used(void) const {
			return ((bool)this->key);
		};
	};
	typedef struct item	hash_table_item;

	hash_table_item*	items;
	i32					count;
	i32					cap;

	b32					case_insensitive; /* could be a u8 flag */

	hash_table(i32 cap = 0): items(0), count(0), cap(0), case_insensitive(0) {
		if (cap > 0)
			this->resize(cap);
	};
	~hash_table(void) { };
	hash_table(const hash_table& table) {
		(*this) = table;
	};
	hash_table&	operator=(const hash_table& table) {
		if (this != &table) {
			this->items = table.items;
			this->count = table.count;
			this->cap = table.cap;

			this->case_insensitive = table.case_insensitive;
		}
		return (*this);
	};

	size_t		hash(const string_view& key, size_t cap) const {
		if (cap == 0ul)
			throw std::runtime_error("Cannot hash when cap is set to zero");
		size_t _hash = 0x811c9dc5;
		for (i32 i = 0; i < key.len; ++i) {
			char	b = key[i];
			if (case_insensitive && b >= 'A' && b <= 'Z') {
				b ^= (2 << 4);
			}
			_hash ^= (size_t)b;
			_hash *= 0x01000193;
		}
		return (_hash & (cap - 1ul));
	};
	void		resize(i32 new_cap) {
		if ((new_cap < this->cap && new_cap < this->count) || new_cap < 0)
			throw std::runtime_error("Cannot resize into less than count");
		hash_table_item*		new_items = new hash_table_item[new_cap]();
		hash_table_item*		old_items = this->items;
		for (i32 i = 0; i < this->cap; ++i) {
			hash_table_item&	iter = old_items[i];
			if (!iter.used())
				continue ;

			size_t index = this->hash(iter.key, (size_t)new_cap);
			while (new_items[index].used())
				index = (index + 1ul) % (size_t)new_cap;
			new_items[index] = iter;
		}
		delete[] old_items;

		this->cap = new_cap;
		this->items = new_items;
	};
	void		destroy(void) {
		for (i32 i = 0; i < this->cap; ++i) {
			if (!this->items[i].used())
				continue ;
			this->items[i].key.free();
		}
		delete[] this->items;

		this->items = 0;
		this->count = 0;
		this->cap = 0;
	}
	void		free(void) { /* NOTE(xenobas): Alias for destroy(void) */
		this->destroy();
	}

	void		set(const string_view& key, const T& value) {
		if (!(bool)key)
			throw std::runtime_error("Cannot set an empty key to a value");
		if (this->count >= this->cap / 2)
			this->resize(this->cap == 0 ? 8 : this->cap * 2);
		size_t	index = this->hash(key, (size_t)this->cap);
		while (this->items[index].used()) {
			if (this->items[index].key == key) {
				this->items[index].value = value;
				return ;
			}
			index = (index + 1ul) % (size_t)this->cap;
		}
		this->items[index] = hash_table_item(key, value);
		this->count++;
	};
	void		unset(const string_view& key) {
		if (!(bool)key)
			return ;
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			hash_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used())
				break ;
			if (item.key == key) {
				item = hash_table_item();
				this->count--;
				return ;
			}
			iter++;
		}
	}
	T&			get(const string_view& key) {
		if (!(bool)key)
			throw std::runtime_error("Cannot get an item with an empty key");
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			hash_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used())
				break ;
			if (item.key == key)
				return (item.value);
			iter++;
		}
		throw std::runtime_error("Item does not exist in the table");
	};
	const T&	get(const string_view& key) const {
		if (!(bool)key)
			throw std::runtime_error("Cannot get a const item with an empty key");
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			const hash_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used())
				break ;
			if (item.key == key)
				return (item.value);
			iter++;
		}
		throw std::runtime_error("Const item does not exist in the table");
	};
	bool		has(const string_view& key) const {
		if (!(bool)key || this->cap == 0 || this->count == 0)
			return (false);
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			const hash_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used())
				break ;
			if (item.key == key)
				return (true);
			iter++;
		}
		return (false);
	};
};
#endif /* HASH_TABLE_HPP */
