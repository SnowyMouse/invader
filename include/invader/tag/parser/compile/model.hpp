// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__MODEL_HPP
#define INVADER__TAG__PARSER__COMPILE__MODEL_HPP

#include <cstdint>

namespace Invader::Parser {
    struct GBXModel;
    struct GBXModelGeometryPart;
    bool uncache_model_markers(GBXModel &model, bool fix);
    bool regenerate_missing_model_vertices(GBXModelGeometryPart &part, GBXModel &model, bool fix);
    bool regenerate_missing_model_vertices(GBXModel &model, bool fix);
    
    struct Model;
    struct ModelGeometryPart;
    bool uncache_model_markers(Model &model, bool fix);
    bool regenerate_missing_model_vertices(ModelGeometryPart &part, Model &model, bool fix);
    bool regenerate_missing_model_vertices(Model &model, bool fix);
    
    enum MaxCompressedModelNodeIndex : std::uint8_t {
        MAX_COMPRESSED_MODEL_NODE_INDEX = static_cast<std::int8_t>(INT8_MAX / 3)
    };
    
    /**
     * Convert a model tag into a gbxmodel tag.
     * @param  model model tag to convert
     * @return       converted tag
     */
    GBXModel convert_model_to_gbxmodel(const Model &model);
    
    /**
     * Convert a gbxmodel tag into a model tag.
     * @param  model gbxmodel tag to convert
     * @return       converted tag
     */
    Model convert_gbxmodel_to_model(const GBXModel &model);
}

#endif
