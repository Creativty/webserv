/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_builder.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/01 13:55:48 by xenobas           #+#    #+#             */
/*   Updated: 2025/12/16 19:41:27 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   STRING_BUILDER_HPP
#define   STRING_BUILDER_HPP
#include "string_view.hpp"
#include "dynamic_array.hpp"

/* NOTE(xenobas): This specific struct utilizes RAII, unlike other structs */
struct string_builder {
	dynamic_array<char>	data;

	string_builder(void);
	string_builder(i32 size);
	~string_builder(void);

	void		write(char ch);
	void		write(const char* str);
	void		write(const string_view& ch);
	void		write(i64 n);

	string_view	to_string(void) const;
	string_view	to_view(void) const;
};

#endif /* STRING_BUILDER_HPP */
