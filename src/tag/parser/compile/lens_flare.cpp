// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void LensFlare::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        this->cos_falloff_angle = std::cos(this->falloff_angle);
        this->cos_cutoff_angle = std::cos(this->cutoff_angle);

        if(this->rotation_function_scale == 360.0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING_PEDANTIC, tag_index, "Function scale is set to 20626.480625° (360 radians). This is a errorneous default value set by older tools.");
        }
    }

    void LensFlare::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Bounds check the bitmap index. This does not use sequence index unlike most things.
        if(!workload.disable_recursion && !workload.disable_error_checking) {
            auto reflection_count = this->reflections.size();
            auto lens_flare_bitmap = this->bitmap.tag_id;
            if(!lens_flare_bitmap.is_null()) {
                std::size_t bitmap_count;
                bitmap_count = reinterpret_cast<Bitmap::struct_little *>(workload.structs[*workload.tags[lens_flare_bitmap.index].base_struct].data.data())->bitmap_data.count.read();
                for(std::size_t i = 0; i < reflection_count; i++) {
                    auto &reflection = this->reflections[i];
                    if(reflection.bitmap_index >= bitmap_count) {
                        char bitmap_path[256];
                        std::snprintf(bitmap_path, sizeof(bitmap_path), "%s.%s", this->bitmap.path.c_str(), HEK::tag_fourcc_to_extension(this->bitmap.tag_fourcc));
                        REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Bitmap #%zu in %s referenced in reflection #%zu is out of bounds (>= %zu)", static_cast<std::size_t>(reflection.bitmap_index), bitmap_path, i, bitmap_count);
                        throw InvalidTagDataException();
                    }
                }
            }
            else if(reflection_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Lens flare has %zu reflection%s, but no bitmap is referenced", reflection_count, reflection_count == 1 ? "" : "s");
            }
        }
    }
}
