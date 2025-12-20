/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dynamic_array.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 09:24:38 by aindjare          #+#    #+#             */
/*   Updated: 2025/12/11 18:05:47 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef DYNAMIC_ARRAY_HPP
#define DYNAMIC_ARRAY_HPP

#include "base.hpp"
#include <stdexcept>
template <typename T>
struct dynamic_array {
	T*	data;
	i32	len;
	i32	cap;

	dynamic_array(int cap = 0): data(0), len(0), cap(0) {
		if (cap > 0)
			this->resize(cap);
	};
	~dynamic_array(void) { };
	dynamic_array(const dynamic_array& array): data(array.data), len(array.len), cap(array.cap) { };
	dynamic_array	operator=(const dynamic_array& array) {
		if (this != &array) {
			this->data = array.data;
			this->len = array.len;
			this->cap = array.cap;
		}
		return (*this);
	};

	T&				operator[](i32 index) {
		if (index < 0)
			throw std::out_of_range("Cannot use negative indices in dynamic_array::operator[]");
		if (index >= len)
			throw std::out_of_range("index is out of range during dynamic_array::operator[]");
		return (data[index]);
	};
	const T&		operator[](i32 index) const {
		if (index < 0)
			throw std::out_of_range("Cannot use negative indices in dynamic_array::operator[]");
		if (index >= len)
			throw std::out_of_range("index is out of range during dynamic_array::operator[]");
		return (data[index]);
	};
	T&				operator[](u64 index) {
		if (index >= (u64)len)
			throw std::out_of_range("index is out of range during dynamic_array::operator[]");
		return (data[index]);
	};
	const T&		operator[](u64 index) const {
		if (index >= (u64)len)
			throw std::out_of_range("index is out of range during dynamic_array::operator[]");
		return (data[index]);
	};

	void	resize(i32 new_cap) {
		T*	old_data = this->data;
		T*	new_data = new T[new_cap]();
		for (i32 i = 0; i < this->len && i < new_cap; ++i)
			new_data[i] = old_data[i];

		this->data = new_data;
		this->cap = new_cap;
		delete[] old_data;
	};
	void	resize_fit(i32 requirement) {
		i32	new_cap = this->cap;
		if (new_cap == 0) {
			new_cap = 8;
		}
		while (new_cap < requirement) {
			new_cap *= 2;
		}

		resize(new_cap);
	}
	T&		push(const T& item) {
		if (this->len >= this->cap) {
			i32	new_cap = this->cap * 2;
			if (new_cap == 0) new_cap = 8;

			this->resize(new_cap);
		}
		return (this->data[this->len++] = item);
	};
	void	push(i32 size, const T* array) {
		if (size <= 0) {
			return ;
		}

		if (this->len + size >= this->cap) {
			resize_fit(this->len + size);
		}

		for (i32 i = 0; i < size; ++i)  {
			this->data[this->len++] = array[i];
		}
	};
	T		pop(void) {
		if (this->len <= 0)
			throw std::out_of_range("Cannot pop an empty dynamic_array");
		return (this->data[--this->len]);
	};
	void	free(void) {
		delete[]	this->data;

		this->data = 0;
		this->len = 0;
		this->cap = 0;
	};
	void	clear(void) {
		this->cap = 0;
		this->len = 0;
	}
};
#endif
