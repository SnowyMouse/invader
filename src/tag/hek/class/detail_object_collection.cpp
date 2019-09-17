/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

 #include "../compile.hpp"
 #include "detail_object_collection.hpp"

 namespace Invader::HEK {
     void compile_detail_object_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
         BEGIN_COMPILE(DetailObjectCollection)
         ADD_DEPENDENCY_ADJUST_SIZES(tag.sprite_plate);
         ADD_REFLEXIVE(tag.types);
         FINISH_COMPILE
     }
 }
