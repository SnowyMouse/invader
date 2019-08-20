/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "found_tag_dependency.hpp"
#include "../tag/compiled_tag.hpp"
#include "../eprintf.hpp"

#include <filesystem>

namespace Invader {
    std::vector<FoundTagDependency> FoundTagDependency::find_dependencies(const char *tag_path_to_find_2, Invader::HEK::TagClassInt tag_int_to_find, std::vector<std::string> tags, bool reverse, bool &success) {
        std::vector<FoundTagDependency> found_tags;
        success = false;

        std::string tag_path_to_find = tag_path_to_find_2;

        if(!reverse) {
            #ifndef _WIN32
            for(std::size_t i = 0; tag_path_to_find[i] != 0; i++) {
                if(tag_path_to_find[i] == '\\') {
                    tag_path_to_find[i] = '/';
                }
            }
            #endif

            // See if we can open the tag
            bool found = false;
            for(auto &tags_directory : tags) {
                std::filesystem::path tag_path = std::filesystem::path(tags_directory) / (tag_path_to_find + "." + tag_class_to_extension(tag_int_to_find));
                std::FILE *f = std::fopen(tag_path.string().data(), "rb");
                if(!f) {
                    continue;
                }
                std::fseek(f, 0, SEEK_END);
                long file_size = std::ftell(f);
                std::fseek(f, 0, SEEK_SET);
                auto tag_data = std::make_unique<std::byte []>(static_cast<std::size_t>(file_size));
                std::fread(tag_data.get(), file_size, 1, f);
                std::fclose(f);

                try {
                    Invader::CompiledTag tag(tag_path, tag_int_to_find, tag_data.get(), static_cast<std::size_t>(file_size));
                    for(auto &dependency : tag.dependencies) {
                        found_tags.emplace_back(dependency.path, dependency.tag_class_int, true);
                    }
                    found = true;
                    break;
                }
                catch (std::exception &e) {
                    eprintf("Failed to compile tag %s. %s\n", tag_path.string().data(), e.what());
                    return std::vector<FoundTagDependency>();
                }
            }

            if(!found) {
                eprintf("Failed to open tag %s.%s.\n", tag_path_to_find.data(), tag_class_to_extension(tag_int_to_find));
                return std::vector<FoundTagDependency>();
            }

            // Dedupe it
            for(auto dep = found_tags.begin(); dep != found_tags.end(); dep++) {
                bool deduped;
                do {
                    deduped = false;
                    for(auto dep2 = dep + 1; dep2 != found_tags.end(); dep2++) {
                        if(dep->path == dep2->path && dep->class_int == dep2->class_int) {
                            found_tags.erase(dep2);
                            deduped = true;
                            break;
                        }
                    }
                } while(deduped);
            }

            // Lastly, go through each dependency and see if it's broken or not
            for(auto &tag : found_tags) {
                std::string full_tag_path = tag.path + "." + Invader::HEK::tag_class_to_extension(tag.class_int);
                #ifndef _WIN32
                for(char &c : full_tag_path) {
                    if(c == '\\') {
                        c = '/';
                    }
                }
                #endif

                for(auto &tags_directory : tags) {
                    std::filesystem::path tag_path = std::filesystem::path(tags_directory) / full_tag_path;
                    if(std::filesystem::is_regular_file(tag_path)) {
                        tag.broken = false;
                    }
                }
            }
        }
        else {
            // Turn all forward slashes into backslashes if not on Windows
            #ifndef _WIN32
            for(std::size_t i = 0; tag_path_to_find[i] != 0; i++) {
                if(tag_path_to_find[i] == '/') {
                    tag_path_to_find[i] = '\\';
                }
            }
            #endif

            // Iterate
            for(auto &tags_directory : tags) {
                auto iterate_recursively = [&found_tags, &tag_int_to_find, &tag_path_to_find](const std::string &current_path, const std::filesystem::path &dir, auto &recursion) -> void {
                    for(auto file : std::filesystem::directory_iterator(dir)) {
                        if(file.is_directory()) {
                            std::string dir_tag_path = current_path + file.path().filename().string() + "\\";
                            recursion(dir_tag_path, file.path(), recursion);
                        }
                        else if(file.is_regular_file()) {
                            std::string dir_tag_path = current_path + file.path().filename().stem().string();
                            auto class_int = Invader::HEK::extension_to_tag_class(file.path().extension().string().data() + 1);

                            // Skip some obvious stuff as well as null tag class ints
                            if(
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_NULL ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_BITMAP ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_CAMERA_TRACK ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_PHYSICS ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_UNICODE_STRING_LIST ||
                                class_int == Invader::HEK::TagClassInt::TAG_CLASS_WIND) {
                                continue;
                            }

                            // If we already found this, ignore it
                            bool skip = false;
                            for(auto &f : found_tags) {
                                if(f.path == dir_tag_path && f.class_int == class_int) {
                                    skip = true;
                                    break;
                                }
                            }
                            if(skip) {
                                break;
                            }

                            // Attempt to open and read the tag
                            std::FILE *f = std::fopen(file.path().string().data(), "rb");
                            if(!f) {
                                eprintf("Failed to open tag %s.\n", file.path().string().data());
                                continue;
                            }

                            std::fseek(f, 0, SEEK_END);
                            long file_size = std::ftell(f);
                            std::fseek(f, 0, SEEK_SET);
                            auto tag_data = std::make_unique<std::byte []>(static_cast<std::size_t>(file_size));
                            std::fread(tag_data.get(), file_size, 1, f);
                            std::fclose(f);

                            // Attempt to parse
                            try {
                                Invader::CompiledTag tag(dir_tag_path, class_int, tag_data.get(), static_cast<std::size_t>(file_size));
                                for(auto &dependency : tag.dependencies) {
                                    if(dependency.path == tag_path_to_find && dependency.tag_class_int == tag_int_to_find) {
                                        found_tags.emplace_back(dir_tag_path, class_int, false);
                                        break;
                                    }
                                }
                            }
                            catch (std::exception &e) {
                                eprintf("Failed to compile tag %s. %s\n", file.path().string().data(), e.what());
                            }
                        }
                    }
                };

                std::filesystem::path tags_dir_path(tags_directory);
                iterate_recursively("", tags_dir_path, iterate_recursively);
            }
        }

        success = true;
        return found_tags;
    }
}
