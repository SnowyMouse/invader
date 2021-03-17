// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__RECOVER__RECOVER_HPP
#define INVADER__RECOVER__RECOVER_HPP

#include <filesystem>

#include <invader/hek/fourcc.hpp>

namespace Invader::Parser {
    struct ParserStruct;
}

namespace Invader::Recover {
    /**
     * Recover tag data
     * @param tag        tag to recover
     * @param path       tag path
     * @param data       data directory to recover to
     * @param tag_fourcc tag class fourcc
     * @param overwrite  overwrite data
     */
    void recover(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, HEK::TagFourCC tag_fourcc, bool overwrite);
}

#endif
