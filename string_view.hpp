/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_view.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/16 09:16:21 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/26 17:19:29 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   STRING_VIEW_HPP
#define   STRING_VIEW_HPP
#include <string>
#include <ostream>
#include "base.hpp"
struct string_view {
	char*	text;
	i32		len;

	string_view(void);
	~string_view(void);
	string_view(const string_view& view);
	string_view&		operator=(const string_view& view);

	string_view(const char* text);
	string_view(char* text, i32 len);
	string_view&		operator=(const char* text);

	operator			bool(void) const;

	bool				operator==(const string_view& view) const;
	bool				operator==(const char* text) const;
	bool				operator!=(const string_view& view) const;

	char&				operator[](i32 index);
	const char&			operator[](i32 index) const;
	char&				operator[](unsigned long long index);
	const char&			operator[](unsigned long long index) const;

	bool				has_prefix(const string_view& prefix) const;
	bool				has_suffix(const string_view& suffix) const;

	string_view			slice(i32 begin, i32 end) const;
	std::string			to_string(void) const;

	void				free(void);
	static string_view	alloc(const string_view& view);
};
std::ostream&	operator<<(std::ostream& stream, const string_view& view);
#endif /* STRING_VIEW_HPP */
