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
#include <type_traits>
#include <optional>

namespace base64 {

	namespace detail {

		template <typename T>
		using mut = typename std::remove_const<T>::type;

		template <typename T>
		using ptr = T * const;

		using u8 = decltype(u8'\0') const; // clang C++2a; char8_t isn't a keyword yet ;)

		using ustring_view = std::basic_string_view<mut<u8>>;

		using ustring = std::basic_string<mut<u8>>;

		using usize = std::size_t const;

		constexpr ustring_view const operator""sv(
			ptr<u8> data,
			usize length
		) noexcept(true) {
			return ustring_view {
				data,
				length
			};
		}

		ustring const operator""s(
			ptr<u8> data,
			usize length
		) {
			return ustring {
				data,
				length
			};
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
			['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63,
		};

		using opt_ustring = std::optional<ustring>;

		// Converts binary data of length to base64 characters.
		opt_ustring encode(
			ustring_view const input
		) noexcept(true) {
			// I look at your data like the stream of unsigned bytes that it is
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
			// Could also do (pad = 3 - modulus_length), but that gives 3 when (modulus_length == 0).
			// 0 => 0
			// 1 => 1 pad 2
			// 2 => 2 pad 1
			// 3 => 0 no pad
			// 4 => 1 pad 2
			// 5 => 2 pad 1

			ustring return_value;

			try {
				usize final_length = 4u * (length + pad) / 3u;

				return_value = ustring(final_length, u8'\0');
			} catch(std::bad_alloc const&) {
				return opt_ustring {
					std::nullopt
				};
			}

			ptr<mut<u8>> res = return_value.data();

			// Counters
			mut<usize> result_counter = 0u;
			mut<usize> byte_no = 0u; // I need this after the loop

			// If there WAS padding, skip the last 3 octets and process below.
			// iterations for length: 0 => 0x, 1 => 0x, 2 => 0x, 3 => 1x, 4 => 1x, 5 => 1x, 6 => 2x, ...
			// be careful about unsigned overflow, don't subtract from length or it'll wrap around if (length < 3)
			for (; byte_no + 3u <= length; byte_no += 3u ) {
				auto const temp = data + byte_no;

				u8 byte0 = temp[0u];
				u8 byte1 = temp[1u];
				u8 byte2 = temp[2u];

				// Take first sextet (an octet (byte) is 8 bits, so a sextet is 6 bits, or a nibble and a half.)
				// and find out what number they are.
				res[result_counter++] = b64[byte0 >> 2u];
				// unsigned so 0's always come in from left (even though there is 
				// implicit int promotion on R&L sides prior to actual bitshift).
				// convert that number into the base64 alphabet.
				// the value in 6 bits can never be larger than 63.

				// the second sextet is part of the first byte and partly in the 2nd byte.
				res[result_counter++] = b64[((0x3u & byte0) << 4u) + (byte1 >> 4u)];

				// notice how I avoided the scary endian ghost by using an unsigned byte pointer for all this.

				// 3rd sextet is lower nibble of 2nd byte and upper half nibble of 3rd byte.
				res[result_counter++] = b64[((0x0Fu & byte1) << 2u) + (byte2 >> 6u)];

				// 4th sextet
				res[result_counter++] = b64[0x3Fu & byte2];
			}

			// The last 3 octets must be converted carefully as if len%3==1 or len%3==2 we must
			// "pretend" there are additional bits at the end.
			if ( 2u == pad ) {
				u8 temp = data[byte_no];

				// We are missing 2 bytes. So
				// - we will only extract 2 sextets when (length % 3 == 1)

				res[result_counter++] = b64[temp >> 2u];
				res[result_counter++] = b64[(0x3u & temp) << 4u];
				// "padded" by 0's, these 2 bits are still HI ORDER BITS.
				// Last 2 are ==, to indicate there's been a 2 byte-pad
				res[result_counter++] = u8'=';
				res[result_counter++] = u8'=';
			} else if ( 1u == pad ) {
				u8 temp0 = data[byte_no];
				u8 temp1 = data[byte_no + 1u];

				// When (length % 3 == 2) (2, 5, 8, 11) (missing 1 byte).
				// - 3 sextets

				res[result_counter++] = b64[temp0 >> 2u];
				res[result_counter++] = b64[((0x3u & temp0) << 4u) + (temp1 >> 4u)]; // SEX2 formula
				res[result_counter++] = b64[(0x0Fu & temp1) << 2u]; // only part of SEX3 that comes from byte#1
				res[result_counter++] = u8'=';
			}

			// copy and return
			return opt_ustring {
				return_value
			};
		}

		// Checks the integrity of a base64 string to make sure it is
		// made up of only characters in the base64 alphabet (array b64)
		inline bool is_valid_base64_char(u8 ch) {
			// return 64u != unb64[ch];
			return (u8'0' <= ch && ch <= u8'9') // between 0-9
				|| (u8'A' <= ch && ch <= u8'Z') // between A-Z
				|| (u8'a' <= ch && ch <= u8'z') // between a-z
				|| u8'+' == ch || u8'/' == ch; // other 2 valid chars, + ending chrs
		}

		bool base64integrity(
			ustring_view const potentially_invalid_base64
		) {
			u8 * const ascii = potentially_invalid_base64.data();
			usize length = potentially_invalid_base64.length();

			// looking for bad characters
			mut<usize> i = 0u; // used after loop

			for (; 2u + i < length; ++i) {
				if ( !is_valid_base64_char(ascii[i]) )  {
					// String is not valid base64
					return false;
				}
			}

			// Only last 2 can be '='
			// Check 2nd last:
			if ( u8'=' == ascii[i] ) {
				// If the 2nd last is = the last MUST be = too
				if ( u8'=' != ascii[1u + i] ) {
					return false;
				}
			} else if ( !is_valid_base64_char( ascii[i] ) ) { // not '=' or valid base64
				// 2nd last was invalid and not '='
				return false;
			}

			// check last

			++i;

			return !( u8'=' != ascii[i] && !is_valid_base64_char( ascii[i] ) );
			// (last char) string is not valid base64; Otherwise b64 string was valid.
		}

		template<bool const checkValidity>
		opt_ustring decode(
			ustring_view const input
		) noexcept(true) {
			if constexpr (checkValidity) {
				if ( !base64integrity( input ) ) {
					// `nullopt` if bad integrity.
					return opt_ustring {
						std::nullopt
					};
				}
			}

			ptr<u8> data = input.data();
			// the maximum value read out is 255,
			// and the value is never negative. This is a type of
			// "if statement" enforced by the type of the pointer.
			// This eliminates a possible bounds check on array lookups into unb64[]
			// (having values between 0 and 255 means it will always be
			// inside the bounds of the 256 element array).
			usize length = input.length();

			if ( length < 2u ) {
				// 2 accesses below would be OOB.
				// catch empty string, return nullopt as result.
				// you passed an invalid base64 string (too short)
				return opt_ustring {
					std::nullopt
				};
			}

			// Count == on the end to determine how much it was padded.
			// 0..2
			u8 pad = static_cast<u8>(u8'=' == data[length - 1u])
				   + static_cast<u8>(u8'=' == data[length - 2u]);

			ustring return_value;

			try {
				// You take the ascii string len and divide it by 4
				// to get #24lets (groups of 3 octets). You then * 3 to
				// get #octets total.
				usize final_length = length / 4u * 3u - pad;

				return_value = ustring(final_length, u8'\0');
			} catch(std::bad_alloc const&) {
				return opt_ustring {
					std::nullopt
				};
			}

			ptr<mut<u8>> res = return_value.data();

			mut<usize> counter = 0u; // counter for `data`
			mut<usize> char_no = 0u; // counter for what base64 char we're currently decoding

			// NEVER do the last group of 4 characters if either of the
			// last 2 chars were pad.
			for (; char_no + (4u + pad) <= length; char_no += 4u ) {
				auto const temp = data + char_no;

				// Get the numbers each character represents
				// Since ascii is ONE BYTE, the worst that can happen is
				// you get a bunch of 0's back (if the base64 string contained
				// characters not in the base64 alphabet).
				// The only way `base64::decode` will TELL you about this though
				// is if you use pass <true> (particularly because
				// there is a 3-4x performance hit, just for the integrity check.)
				u8 A = unb64[temp[0u]];
				u8 B = unb64[temp[1u]];
				u8 C = unb64[temp[2u]];
				u8 D = unb64[temp[3u]];

				// Just unmap each sextet to THE NUMBER it represents.
				// You then have to pack it in res,
				// we go in groups of 4 sextets, 
				// and pull out 3 octets per quad of sextets.
				//    res[0]       res[1]      res[2]
				// +-----------+-----------+-----------+
				// | 0000 0011   0111 1011   1010 1101 |
				// +-AAAA AABB   BBBB CCCC   CCDD DDDD
				// or them

				res[counter++] = static_cast<u8>((A << 2u) | (B >> 4u)); // or in last 2 bits of B

				// The 2nd byte is the bottom 4 bits of B for the upper nibble,
				// and the top 4 bits of C for the lower nibble.
				res[counter++] = static_cast<u8>((B << 4u) | (C >> 2u));
				res[counter++] = static_cast<u8>((C << 6u) | (D >> 0u)); // shove C up to top 2 bits, or with D
			}

			{
				ptr<u8> temp = data + char_no;

				if ( 1u == pad ) {
					// 1 padding character.
					//    data[0]     data[1]     data[2]
					// +-----------+-----------+-----------+
					// | 0000 0011   1111 1111   ~~~~ ~~~~ |
					// +-AAAA AABB   BBBB CCCC   XXXX XXXX  
					// We can pull 2 bytes out, not 3.
					// We have 3 characters A, B and C, not 4.
					u8 A = unb64[temp[0u]];
					u8 B = unb64[temp[1u]];
					u8 C = unb64[temp[2u]];

					res[counter++] = static_cast<u8>((A << 2u) | (B >> 4u));
					res[counter++] = static_cast<u8>((B << 4u) | (C >> 2u));
				} else if ( 2u == pad ) {
					u8 A = unb64[temp[0u]];
					u8 B = unb64[temp[1u]];

					res[counter++] = static_cast<u8>((A << 2u) | (B >> 4u));
				}
			}

			return opt_ustring {
				return_value
			};
		}

	} // namespace base64::detail

	constexpr auto const& encode = detail::encode;
	constexpr auto const& decode = detail::decode<true>;
	constexpr auto const& decode_nocheck = detail::decode<false>;

} // namespace base64
