NibbleAndAHalf
==============

"Nibble And A Half" is a C++ library that provides fast base64 encoding and decoding, all in a single header file.

- All test related functions moved to testbase64.h.  To use, only need #include "base64.hpp":
  https://github.com/00ff0000red/NibbleAndAHalf/blob/master/NibbleAndAHalf/base64.hpp

The header provides the following API:
```c++
namespace base64 {
    std::optional<ustring> encode(ustring_view const) nothrow(true);

    std::optional<ustring> decode(ustring_view const) nothrow(true);
}
```

Where ustring is a type alias of `std::u8string`, and `ustring_view` is an alias of `std::u8string_view`.

Nothing is introduced into the global scope by importing the file.
