// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__GBXMODEL_HPP
#define INVADER__TAG__PARSER__COMPILE__GBXMODEL_HPP

namespace Invader::Parser {
    struct GBXModel;
    struct GBXModelGeometryPart;
    bool uncache_model_markers(GBXModel &model, bool fix);
    bool regenerate_missing_model_vertices(GBXModelGeometryPart &part, bool fix);
    bool regenerate_missing_model_vertices(GBXModel &model, bool fix);
}

#endif
