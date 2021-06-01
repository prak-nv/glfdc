#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cmath>
#include <string>

namespace glfdc {

// Logarithm base 26
inline double log26(double x) noexcept
{
  // By power of highschool maths:
  return log(x)/log(26);
}

inline std::size_t ndigits_base26(std::size_t n) noexcept
{
  if (n < 1) return 1;
  return 1 + std::size_t(std::floor(log26(double(n))));
}

// Encode number using letters latin alphabet ie.
//
// for 0 <= n < 26 : a...z
// for 26 <= n < 26^2 : aa..zz
// etc.
template <bool Uppercase = false>
inline std::string to_base26(std::size_t n)
{
   const char start_symbol = (Uppercase)? 'A' : 'a';

   // FIXME: for 128b puters...
   // or FIXME: consteval log
   static_assert(sizeof(std::size_t) <= 8);

   const std::size_t ndigits26 = ndigits_base26(n);

   assert(ndigits26 <= 14);

   // log26(2^64) ~= 13.61
   std::array<char, 14> buffer;

   for (int i = ndigits26; i>0; --i)
   {
     buffer[i-1] = start_symbol + n % 26;
     n /= 26;
   }

   return std::string(buffer.data(), ndigits26);
}

template <bool Uppercase>
bool is_letter(char d) noexcept
{
  constexpr const char start_symbol = (Uppercase)? 'A' : 'a';
  constexpr const char end_symbol = (Uppercase)? 'Z' : 'z';

  return  d >= start_symbol && d <= end_symbol;
}

template <bool Uppercase = false>
inline std::size_t from_base26(const std::string& base26) noexcept
{
  constexpr const char start_symbol = (Uppercase)? 'A' : 'a';

  assert(!base26.empty());
  std::size_t value = 0;

  for (auto it = base26.rbegin(); it != base26.rend(); ++it)
  {
    assert(is_letter<Uppercase>(*it));
    value *= 26;
    value += (*it - start_symbol);
  }

  return value;
}

} // namespace glfdc
