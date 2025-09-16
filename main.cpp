/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/13 17:00:44 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/16 15:43:35 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define TYPES_HPP_IMPL
#include "types.hpp"

#include <iostream>

i32	main_toml(i32 argc, type::cstring* argv);

i32	main(i32 argc, type::cstring *argv) {
	return (main_toml(argc, argv));
}
