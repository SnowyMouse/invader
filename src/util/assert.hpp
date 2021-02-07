// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__UTIL_ASSERT_HPP
#define INVADER__UTIL_ASSERT_HPP

#include <exception>
#include <invader/printf.hpp>

#ifdef NDEBUG

#define invader_assert_m(expression, message) {}
#define invader_assert(expression) {}

#else

#define INVADER__UTIL_ASSERT_STR1(str) # str
#define INVADER__UTIL_ASSERT_STR2(str) INVADER__UTIL_ASSERT_STR1(str)

#define invader_assert_m(expression, message) if(!(expression)) { \
                                                  eprintf_error("%s:%i - invader_assert() failed!\n%s", __FILE__, __LINE__, message); \
                                                  std::terminate(); \
                                              }
#define invader_assert(expression) if(!(expression)) { \
                                       eprintf_error("%s:%i - invader_assert() failed!\n%s", __FILE__, __LINE__, INVADER__UTIL_ASSERT_STR2(expression)); \
                                       std::terminate(); \
                                   }
#endif
#endif
