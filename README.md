NibbleAndAHalf
==============

Forked from "Nibble And A Half," this is a C++ library that provides fast base64 encoding and decoding, all in a single header file.

- To use, only need #include "base64.hpp":
  https://github.com/00ff0000red/NibbleAndAHalf/blob/master/NibbleAndAHalf/base64.hpp

The header provides the following API:
```c++
namespace base64 {
    std::u8string (&encode)(u8string_view const);

    std::optional<std::u8string> (&decode)(u8string_view const);

    std::optional<std::u8string> (&decode_nocheck)(u8string_view const);
}
```

`decode` returns an empty `std::optional` if the string contains any invalid base64 characters, whereas `decode_nocheck` will treat them as if they were all `'A'` characters.

If the input string has an incorrect amount of padding, then an empty `std::optional` is returned.

All functions may throw `std::bad_alloc`.

According to the original author, from whom this code is forked from, using `decode_nocheck` should yield at least 3x performance gains.

Nothing is introduced into the global scope by importing the file.
