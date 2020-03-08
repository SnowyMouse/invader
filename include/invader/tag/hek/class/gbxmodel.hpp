// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__GBXMODEL_HPP
#define INVADER__TAG__HEK__CLASS__GBXMODEL_HPP

namespace Invader::Parser {
    struct GBXModel;
}

namespace Invader::HEK {
    void uncache_model_markers(Parser::GBXModel &model);
}

#endif
