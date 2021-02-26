NibbleAndAHalf
==============

Forked from "Nibble And A Half," this is a C++ library that provides fast base64 encoding and decoding, all in a single header file.

- To use, only need #include "base64.hpp":
  https://github.com/00ff0000red/NibbleAndAHalf/blob/master/NibbleAndAHalf/base64.hpp

The header provides the following API:
```c++
namespace base64 {
    std::optional<ustring> (&encode)(ustring_view const) noexcept(true);

    std::optional<ustring> (&decode)(ustring_view const) noexcept(true);

    std::optional<ustring> (&decode_nocheck)(ustring_view const) noexcept(true);
}
```

Where ustring is a type alias of `std::u8string`, and `ustring_view` is an alias of `std::u8string_view`.

`decode` returns an empty `std::optional` if the string contains any invalid base64 characters, whereas `decode_nocheck` will treat them as if they were all `'A'` characters.

Nothing is introduced into the global scope by importing the file.
