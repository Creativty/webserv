/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strconv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 16:15:02 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 16:15:53 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   STRCONV_HPP
#define   STRCONV_HPP

#include "types.hpp"
namespace strconv {
	bool	parse_i64(const type::string&, i64&);
};

#endif /* STRCONV_HPP */
