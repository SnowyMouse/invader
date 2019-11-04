// SPDX-License-Identifier: GPL-3.0-only

 #include <invader/tag/hek/compile.hpp>
 #include <invader/tag/hek/class/detail_object_collection.hpp>

 namespace Invader::HEK {
     void compile_detail_object_collection_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
         BEGIN_COMPILE(DetailObjectCollection)
         ADD_DEPENDENCY_ADJUST_SIZES(tag.sprite_plate);
         ADD_REFLEXIVE(tag.types);
         FINISH_COMPILE
     }
 }
