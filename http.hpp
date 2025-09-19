/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aindjare <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 16:16:43 by aindjare          #+#    #+#             */
/*   Updated: 2025/09/19 17:54:36 by aindjare         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef   HTTP_HPP
#define   HTTP_HPP
#include "types.hpp"

namespace http {
	enum method_kind {
		METHOD_UNKNOWN,

		METHOD_GET,
		METHOD_HEAD,
		METHOD_POST,
		METHOD_PUT,
		METHOD_DELETE,
		METHOD_CONNECT,
		METHOD_OPTIONS,
		METHOD_TRACE,
	};
	typedef type::hash_map<type::string>	headers;
	struct request {
		u32									_index;
		type::dynamic_array<byte>			_bytes;
		bool								_done;
		bool								_fail;
		bool								_multipart;

		struct { /* method */
			type::string					string;
			method_kind						kind;
		}									method;             	
		type::string						uri;
		type::string						version;

		headers								headers;
		type::dynamic_array<byte>			body;

		request(void);
		~request(void);
		request(const request&);
		request&							operator=(const request&);
		void								free(void);
		static	bool						parse(request&, byte*, u32);
	};
};

#endif
