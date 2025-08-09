/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sv.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/08 22:40:04 by xenobas           #+#    #+#             */
/*   Updated: 2025/08/08 22:42:08 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "string_view.hpp"
#include <iostream>

int	main(int argc, const char **argv) {
	for (int i = 1; i < argc; i++) {
		string_view	sv = argv[i];
		if (i % 2)
			std::cout << sv << std::endl;
		else
			std::cout << string_view_fmt() <<  sv << std::endl;
	}
	return (0);
}
