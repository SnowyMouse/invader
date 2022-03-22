// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__EXPRESSION_HPP
#define INVADER__EDIT__EXPRESSION_HPP

#include <cstdint>

namespace Invader::Edit {
    double evaluate_expression(const char *expression, double input);
    std::int64_t evaluate_expression(const char *expression, std::int64_t input);
}

#endif
