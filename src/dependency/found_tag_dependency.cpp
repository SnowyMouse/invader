// SPDX-License-Identifier: GPL-3.0-only

#include <invader/dependency/found_tag_dependency.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/build/build_workload.hpp>

#include <filesystem>

namespace Invader {
    static std::vector<File::TagFilePath> get_dependencies(const BuildWorkload &tag_compiled) {
        std::vector<File::TagFilePath> dependencies;
        for(auto &s : tag_compiled.structs) {
            for(auto &d : s.dependencies) {
                // Skip anything referencing ourselves
                if(d.tag_index == 0) {
                    continue;
                }

                // Continue
                auto &tag = tag_compiled.tags[d.tag_index];
                dependencies.emplace_back(tag.path, tag.tag_class_int);
            }
        }
        return dependencies;
    }

    std::vector<FoundTagDependency> FoundTagDependency::find_dependencies(const char *tag_path_to_find_2, Invader::TagClassInt tag_int_to_find, std::vector<std::filesystem::path> tags, bool reverse, bool recursive, bool &success) {
        std::vector<FoundTagDependency> found_tags;
        success = true;

        if(!reverse) {
            auto find_dependencies_in_tag = [&tags, &found_tags, &recursive, &success](const char *tag_path_to_find_2, Invader::TagClassInt tag_int_to_find, auto recursion) -> void {
                std::string tag_path_to_find = File::halo_path_to_preferred_path(tag_path_to_find_2);

                // See if we can open the tag
                bool found = false;
                for(auto &tags_directory : tags) {
                    std::filesystem::path tag_path = std::filesystem::path(tags_directory) / (tag_path_to_find + "." + tag_class_to_extension(tag_int_to_find));
                    auto tag_data = File::open_file(tag_path);
                    if(!tag_data.has_value()) {
                        eprintf_error("Failed to read tag %s", tag_path.string().c_str());
                        continue;
                    }

                    try {
                        auto dependencies = get_dependencies(BuildWorkload::compile_single_tag(tag_data->data(), tag_data->size()));
                        for(auto &dependency : dependencies) {
                            // Make sure it's not in found_tags
                            bool dupe = false;
                            for(auto &tag : found_tags) {
                                if(tag.path == dependency.path && tag.class_int == dependency.class_int) {
                                    dupe = true;
                                    break;
                                }
                            }
                            if(dupe) {
                                continue;
                            }

                            auto class_to_use = dependency.class_int;
                            std::string path_copy = File::halo_path_to_preferred_path(dependency.path + "." + tag_class_to_extension(class_to_use));

                            bool found = false;
                            for(auto &tags_directory : tags) {
                                auto complete_tag_path = std::filesystem::path(tags_directory) / path_copy;
                                if(std::filesystem::is_regular_file(complete_tag_path)) {
                                    found_tags.emplace_back(dependency.path, class_to_use, false, complete_tag_path);
                                    found = true;
                                    break;
                                }
                            }

                            if(!found) {
                                found_tags.emplace_back(dependency.path, class_to_use, true, std::nullopt);
                            }
                            else if(recursive) {
                                recursion(dependency.path.c_str(), class_to_use, recursion);
                            }
                        }
                        found = true;
                        break;
                    }
                    catch (std::exception &e) {
                        eprintf_error("Failed to compile tag %s: %s", tag_path.string().c_str(), e.what());
                        success = false;
                        return;
                    }
                }

                if(!found) {
                    eprintf_error("Failed to open tag %s.%s.", tag_path_to_find.c_str(), tag_class_to_extension(tag_int_to_find));
                    success = false;
                    return;
                }
            };

            find_dependencies_in_tag(tag_path_to_find_2, tag_int_to_find, find_dependencies_in_tag);
        }
        else {
            // Turn all forward slashes into backslashes if not on Windows
            std::string tag_path_to_find = File::preferred_path_to_halo_path(tag_path_to_find_2);

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
                            auto class_int = Invader::HEK::extension_to_tag_class(file.path().extension().string().c_str() + 1);

                            // Skip some obvious stuff as well as null tag class ints
                            if(
                                class_int == Invader::TagClassInt::TAG_CLASS_NULL ||
                                class_int == Invader::TagClassInt::TAG_CLASS_BITMAP ||
                                class_int == Invader::TagClassInt::TAG_CLASS_CAMERA_TRACK ||
                                class_int == Invader::TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT ||
                                class_int == Invader::TagClassInt::TAG_CLASS_PHYSICS ||
                                class_int == Invader::TagClassInt::TAG_CLASS_SOUND_ENVIRONMENT ||
                                class_int == Invader::TagClassInt::TAG_CLASS_UNICODE_STRING_LIST ||
                                class_int == Invader::TagClassInt::TAG_CLASS_WIND) {
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
                            
                            // Open it
                            auto fp = file.path();
                            auto tag_data = File::open_file(fp);
                            if(!tag_data.has_value()) {
                                eprintf_error("Failed to read tag %s", fp.string().c_str());
                                continue;
                            }

                            // Attempt to parse
                            try {
                                auto dependencies = get_dependencies(BuildWorkload::compile_single_tag(tag_data->data(), tag_data->size()));
                                for(auto &dependency : dependencies) {
                                    if(dependency.path == tag_path_to_find && dependency.class_int == tag_int_to_find) {
                                        found_tags.emplace_back(dir_tag_path, class_int, false, file.path());
                                        break;
                                    }
                                }
                            }
                            catch (std::exception &e) {
                                eprintf_warn("Warning: Failed to compile tag %s: %s", file.path().string().c_str(), e.what());
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
