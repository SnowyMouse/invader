// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__PAD_HPP
#define INVADER__HEK__PAD_HPP

#include <cstddef>

#define MAKE_PAD2(size, name) std::byte zzz_padding_ ## size ## _l ## name [ size ]
#define MAKE_PAD(size, name) MAKE_PAD2(size, name)
#define PAD(size) MAKE_PAD(size, __LINE__)

#endif
