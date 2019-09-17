/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

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
