/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xenobas <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 22:07:43 by xenobas           #+#    #+#             */
/*   Updated: 2025/08/08 22:59:11 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "core.hpp"
#include "toml.hpp"

int	main(int argc, const char **argv) {
	u32 x = 32;
	(void)x;
	for (u64 i = 1ul; i < cast(u64)argc; i++) {
		string_view	arg(argv[i]);

		TOML_Table*	toml = toml_from_file(arg);
		(void)toml;
	}
	return (0);
}
