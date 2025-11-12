/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   base.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 15:56:46 by aindjare          #+#    #+#             */
/*   Updated: 2025/11/12 13:45:24 by xenobas          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BASE_HPP
#define BASE_HPP

#include <stdint.h>

#define cast(T) (T)
#define unused(V) ((void)(V))
#define count_of(V) (sizeof(V) / sizeof(V[0]))

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef u8			byte;
typedef i32			b32;
typedef const char*	cstring;

#define U32_MAX (4294967295u)
#define U64_MAX (1844674407370955161ull)

#define I32_MIN (-2147483648)
#define I32_MAX (+2147483647)

#define I64_MIN (-9223372036854775807ll)
#define I64_MAX (+9223372036854775807ll)

template <typename T>
void		MEM_zero(T& value) {
	u64		count = sizeof(T);
	byte*	bytes = cast(byte*)(&value);
	for (u64 i = 0; i < count; ++i) {
		bytes[i] = 0;
	}
}

#endif
