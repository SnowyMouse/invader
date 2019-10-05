// SPDX-License-Identifier: GPL-3.0-only

 #include "../compile.hpp"
 #include "fog.hpp"

 namespace Invader::HEK {
     void compile_fog_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
         BEGIN_COMPILE(Fog)
         ADD_DEPENDENCY_ADJUST_SIZES(tag.map);
         ADD_DEPENDENCY_ADJUST_SIZES(tag.background_sound);
         ADD_DEPENDENCY_ADJUST_SIZES(tag.sound_environment);
         FINISH_COMPILE
     }
 }
