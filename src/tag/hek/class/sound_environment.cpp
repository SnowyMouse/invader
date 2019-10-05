// SPDX-License-Identifier: GPL-3.0-only

#include "../compile.hpp"
#include "sound_environment.hpp"

namespace Invader::HEK {
    void compile_sound_environment_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(SoundEnvironment);
        FINISH_COMPILE
    }
}
