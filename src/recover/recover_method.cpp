// SPDX-License-Identifier: GPL-3.0-only

#include <invader/file/file.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/parser/parser.hpp>
#include "recover_method.hpp"

namespace Invader::Recover {
    [[noreturn]] static void create_directories_save_and_quit(const std::filesystem::path &path, const std::vector<std::byte> &data) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path());
        
        // Save it
        if(!File::save_file(path, data)) {
            eprintf_error("Failed to write to %s", path.string().c_str());
            std::exit(EXIT_FAILURE);
        }
        
        oprintf_success("Recovered %s", path.string().c_str());
        std::exit(EXIT_SUCCESS);
    }
    
    static void recover_tag_collection(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data) {
        auto *tag_collection = dynamic_cast<const Parser::TagCollection *>(&tag);
        if(!tag_collection) {
            return;
        }
        
        // Output it
        std::string output;
        for(auto &i : tag_collection->tags) {
            output += i.reference.path + "." + HEK::tag_fourcc_to_extension(i.reference.tag_fourcc) + "\n";
        }
        
        // Create directories
        auto file_path = data / (path + ".txt");
        
        auto *output_data = output.data();
        create_directories_save_and_quit(file_path, std::vector<std::byte>(reinterpret_cast<const std::byte *>(output_data), reinterpret_cast<const std::byte *>(output_data + output.size())));
    }
    
    void recover(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, HEK::TagFourCC tag_fourcc) {
        recover_tag_collection(tag, path, data);
    
        eprintf_warn("Data cannot be recovered from tag class %s", HEK::tag_fourcc_to_extension(tag_fourcc));
        std::exit(EXIT_FAILURE);
    }
}
