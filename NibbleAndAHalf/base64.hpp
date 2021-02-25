/*

https://github.com/superwills/NibbleAndAHalf
base64.h -- Fast base64 encoding and decoding.
version 1.0.0, April 17, 2013 143a

Copyright (C) 2013 William Sherif

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

William Sherif
will.sherif@gmail.com

YWxsIHlvdXIgYmFzZSBhcmUgYmVsb25nIHRvIHVz

*/

#pragma once

#include <string_view>
#include <string>
#include <ostream>
#include <type_traits>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <optional>

namespace base64 {

	namespace detail {

		template <typename T>
		using mut = typename std::remove_const<T>::type;

		template <typename T>
		using ptr = T * const;

		using u8 = decltype(u8'\0') const;

		using ustring_view = std::basic_string_view<mut<u8>>;

		using ustring = std::basic_string<mut<u8>>;

		using usize = std::size_t const;

		ustring_view const operator""_usv(
			char const * const data,
			usize length
		) noexcept(true) {
			return ustring_view {
				reinterpret_cast<ptr<u8>>(data),
				length
			};
		}

		ustring const operator""_us(
			char const * const data,
			usize length
		) {
			return ustring {
				reinterpret_cast<ptr<u8>>(data),
				length
			};
		}

		std::ostream& operator << (
			std::ostream& self,
			ustring_view const& output
		) {
			std::copy(
				output.begin(),
				output.end(),
				std::ostreambuf_iterator<char>(self)
			);

			return self;
		}

		std::wostream& operator << (
			std::wostream& self,
			ustring_view const& output
		) {
			std::copy(
				output.begin(),
				output.end(),
				std::ostreambuf_iterator<wchar_t>(self)
			);

			return self;
		}

		constexpr u8 b64[] =
			u8"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		// inversion of b64[]
		constexpr u8 unb64[0x100] = {
			['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,
			['E'] = 4,  ['F'] = 5,  ['G'] = 6,  ['H'] = 7,
			['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
			['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15,
			['Q'] = 16, ['R'] = 17, ['S'] = 18, ['T'] = 19,
			['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
			['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27,
			['c'] = 28, ['d'] = 29, ['e'] = 30, ['f'] = 31,
			['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
			['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39,
			['o'] = 40, ['p'] = 41, ['q'] = 42, ['r'] = 43,
			['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
			['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51,
			['0'] = 52, ['1'] = 53, ['2'] = 54, ['3'] = 55,
			['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
			['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63
		};

		using opt_ustring = std::optional<ustring>;
	} // namespace base64::detail

	// Converts binary data of length to base64 characters.
	detail::opt_ustring encode(
		detail::ustring_view const input
	) noexcept(true) {
		using namespace detail;

		ptr<u8> data = input.data();
		usize length = input.length();

		// 0..2
		u8 pad = ({
			usize modulus_length = length % 3u;

			static_cast<u8>(
				((modulus_length & 1u) << 1u)
				+ ((modulus_length & 2u) >> 1u)
			);
			// 2 gives 1 and 1 gives 2, but 0 gives 0.
		});

		ustring _res;

		try {
			usize final_length = 4u * (length + pad) / 3u;

			_res = ustring(final_length, u8'\0');
		} catch(std::bad_alloc const&) {
			return opt_ustring {
				std::nullopt
			};
		}

		ptr<mut<u8>> res = _res.data();

		mut<usize> rc = 0u; // result counter
		mut<usize> byteNo = 0u; // I need this after the loop
		for (; byteNo <= length - 3u; byteNo += 3u ) {
			u8 byte0 = data[byteNo + 0u];
			u8 byte1 = data[byteNo + 1u];
			u8 byte2 = data[byteNo + 2u];

			res[rc++] = b64[byte0 >> 2u];
			res[rc++] = b64[((0x3u & byte0) << 4u) + (byte1 >> 4u)];
			res[rc++] = b64[((0x0Fu & byte1) << 2u) + (byte2 >> 6u)];
			res[rc++] = b64[0x3Fu & byte2];
		}

		if ( 2u == pad ) {
			u8 temp = data[byteNo];

			res[rc++] = b64[temp >> 2u];
			res[rc++] = b64[(0x3u & temp) << 4u];
			res[rc++] = u8'=';
			res[rc++] = u8'=';
		} else if ( 1u == pad ) {
			u8 temp0 = data[byteNo];
			u8 temp1 = data[byteNo + 1u];

			res[rc++] = b64[temp0 >> 2u];
			res[rc++] = b64[((0x3u & temp0) << 4u) + (temp1 >> 4u)];
			res[rc++] = b64[(0x0Fu & temp1) << 2u];
			res[rc++] = u8'=';
		}

		return opt_ustring {
			_res
		};
	}

	detail::opt_ustring decode(
		detail::ustring_view const input
	) noexcept(true) {
		using namespace detail;

		ptr<u8> data = input.data();
		usize length = input.length();

		if ( length < 2u ) {
			// 2 accesses below would be OOB.
			// catch empty string, return nullopt as result.
			// you passed an invalid base64 string (too short)
			return opt_ustring {
				std::nullopt
			};
		}

		// 0..2
		u8 pad = static_cast<u8>(u8'=' == data[length - 1u])
			   + static_cast<u8>(u8'=' == data[length - 2u]);

		ustring _res;

		try {
			usize final_length = 3u * length / 4u - pad;

			_res = ustring(final_length, u8'\0');
		} catch(std::bad_alloc const&) {
			return opt_ustring {
				std::nullopt
			};
		}

		ptr<mut<u8>> res = _res.data();

		mut<usize> cb = 0u;
		mut<usize> charNo = 0u;
		for (; charNo <= length - 4u - pad; charNo += 4u ) {
			auto const temp = data + charNo;
			u8 A = unb64[temp[0u]];
			u8 B = unb64[temp[1u]];
			u8 C = unb64[temp[2u]];
			u8 D = unb64[temp[3u]];

			res[cb++] = static_cast<u8>((A << 2u) | (B >> 4u));
			res[cb++] = static_cast<u8>((B << 4u) | (C >> 2u));
			res[cb++] = static_cast<u8>((C << 6u) | (D >> 0u));
		}

		{
			ptr<u8> temp = data + charNo;

			if ( 1u == pad ) {
				u8 A = unb64[temp[0u]];
				u8 B = unb64[temp[1u]];
				u8 C = unb64[temp[2u]];

				res[cb++] = static_cast<u8>((A << 2u) | (B >> 4u));
				res[cb++] = static_cast<u8>((B << 4u) | (C >> 2u));
			} else if ( 2u == pad ) {
				u8 A = unb64[temp[0u]];
				u8 B = unb64[temp[1u]];

				res[cb++] = static_cast<u8>((A << 2u) | (B >> 4u));
			}
		}

		return opt_ustring {
			_res
		};
	}

} // namespace base64
