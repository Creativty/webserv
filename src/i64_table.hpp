/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   i64_table.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <rahimos.123@gmail.com>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/17 21:52:38 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/21 05:16:39 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INT_TABLE_HPP
#define INT_TABLE_HPP
#include "base.hpp"

#define I64_TABLE_CAP_INITIAL 128

template <typename T>
struct i64_table {
	struct item {
		T	value;
		i64	key;
		b32	used;

		item(i64 key, T value): value(value), key(key), used(1) { };
		item(void): value(), key(), used(0) { };
		~item(void) { };
		item(const item& item): value(item.value), key(item.key), used(item.used) { };
		item& operator=(const item& item) {
			if (this != &item) {
				this->value = item.value;
				this->key = item.key;
				this->used = item.used;
			}
			return (*this);
		};
	};
	typedef struct item	i64_table_item;

	i64_table_item*	items;
	i32				count;
	i32				cap;

	i64_table(i32 cap = 0): items(0), count(0), cap(0) {
		if (cap > 0)
			this->resize(cap);
	};
	~i64_table(void) { };
	i64_table(const i64_table& table) {
		(*this) = table;
	};
	i64_table&	operator=(const i64_table& table) {
		if (this != &table) {
			this->items = table.items;
			this->count = table.count;
			this->cap = table.cap;
		}
		return (*this);
	};

	size_t		hash(const i64& key, size_t cap) const {
		if (cap == 0ul)
			throw std::runtime_error("Cannot hash when cap is set to zero");
		size_t _hash = 0x811c9dc5;
		for (i32 i = 0; i < cast(i32)size_of(i64); ++i) {
			byte	b = (cast(byte*)(&key))[i];

			_hash ^= (size_t)b;
			_hash *= 0x01000193;
		}
		return (_hash & (cap - 1ul));
	};
	void		resize(i32 new_cap) {
		if ((new_cap < this->cap && new_cap < this->count) || new_cap < 0)
			throw std::runtime_error("Cannot resize into less than count");
		i64_table_item*		new_items = new i64_table_item[new_cap]();
		i64_table_item*		old_items = this->items;
		for (i32 i = 0; i < this->cap; ++i) {
			i64_table_item&	iter = old_items[i];
			if (!iter.used)
				continue ;

			size_t index = this->hash(iter.key, (size_t)new_cap);
			while (new_items[index].used)
				index = (index + 1ul) % (size_t)new_cap;
			new_items[index] = iter;
		}
		delete[] old_items;

		this->cap = new_cap;
		this->items = new_items;
	};
	void		destroy(void) {
		delete[] this->items;

		this->items = 0;
		this->count = 0;
		this->cap = 0;
	}
	void		free(void) { /* NOTE(xenobas): Alias for destroy(void) */
		this->destroy();
	}

	/* TODO(xenobas): Risky business */
	void		set(const i64& key, const T& value) {
		if (this->count >= this->cap / 2)
			this->resize(this->cap == 0 ? I64_TABLE_CAP_INITIAL : this->cap * 2);
		size_t	index = this->hash(key, (size_t)this->cap);
		while (this->items[index].used) {
			if (this->items[index].key == key) {
				this->items[index].value = value;
				return ;
			}
			index = (index + 1ul) % (size_t)this->cap;
		}
		this->items[index] = i64_table_item(key, value);
		this->count++;
	};
	void		unset(const i64& key) {
		if (!(bool)key)
			return ;
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			i64_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used)
				break ;
			if (item.key == key) {
				item = i64_table_item();
				this->count--;
				return ;
			}
			iter++;
		}
	}
	T&			get(const i64& key) {
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			i64_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used)
				break ;
			if (item.key == key)
				return (item.value);
			iter++;
		}
		throw std::runtime_error("Item does not exist in the table");
	};
	const T&	get(const i64& key) const {
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			const i64_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used)
				break ;
			if (item.key == key)
				return (item.value);
			iter++;
		}
		throw std::runtime_error("Const item does not exist in the table");
	};
	bool		has(const i64& key) const {
		if (!(bool)key || this->cap == 0 || this->count == 0)
			return (false);
		size_t	iter = 0ul;
		size_t	index = this->hash(key, (size_t)this->cap);
		while (iter < (size_t)this->cap) {
			const i64_table_item&	item = this->items[(index + iter) % (size_t)this->cap];
			if (!item.used)
				break ;
			if (item.key == key)
				return (true);
			iter++;
		}
		return (false);
	};
};

#endif
