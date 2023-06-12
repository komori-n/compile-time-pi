# compile-time-pi

This is a toy project that tries to calculate PI at compile time.

## How to Build

You need to make a modified clang that has no limit on the number of
constexpr-time calculations. Please see
<https://github.com/ushitora-anqou/constexpr-nn/blob/master/clang.diff>
for more detail.

Then, you can start calculating PI by just typing `make`. Note that this may
require more than one hour depending on your environment, so keep patient.

You can change the number of decimal digits by editing the template argument for
`GetPiString()` in `src/main.cpp`:

```cpp
  constexpr auto ans = GetPiString<100000>();
```

## License

CC0 1.0 Universal
