/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strconv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 15:56:12 by aindjare          #+#    #+#             */
/*   Updated: 2025/10/17 16:02:28 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STRCONV_HPP
#define STRCONV_HPP
#include "base.hpp"
#include "string_view.hpp"
namespace strconv {
	bool	parse_i64(const string_view& str, i64& n);
};
#endif
