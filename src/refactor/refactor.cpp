// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <string>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/command_line_option.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/file/file.hpp>

using namespace Invader::File;

std::size_t refactor_tags(const char *file_path, const std::vector<std::pair<TagFilePath, TagFilePath>> &replacements, bool check_only, bool dry_run) {
    // Open the tag
    auto tag = open_file(file_path);
    if(!tag.has_value()) {
        eprintf_error("Failed to open %s", file_path);
        std::exit(EXIT_FAILURE);
    }

    // Get the header
    std::vector<std::byte> file_data;
    std::size_t count = 0;

    try {
        const auto *header = reinterpret_cast<const Invader::HEK::TagFileHeader *>(tag->data());
        Invader::HEK::TagFileHeader::validate_header(header, tag->size());

        auto tag_data = Invader::Parser::ParserStruct::parse_hek_tag_file(tag->data(), tag->size());
        count = tag_data->refactor_references(replacements);
        if(count) {
            file_data = tag_data->generate_hek_tag_data(header->tag_class_int);
        }
        else {
            return count;
        }
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to refactor in %s", file_path);
        std::exit(EXIT_FAILURE);
    }

    if(!check_only) {
        if(!dry_run && !save_file(file_path, file_data)) {
            eprintf_error("Error: Failed to write to %s. This tag will need to be manually edited.", file_path);
            return 0;
        }
        oprintf_success("Replaced %zu reference%s in %s", count, count == 1 ? "" : "s", file_path);
    }

    return count;
}

enum RefactorMode {
    REFACTOR_MODE_COPY,
    REFACTOR_MODE_MOVE,
    REFACTOR_MODE_NO_MOVE
};

int main(int argc, char * const *argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show license and credits.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("dry-run", 'D', 0, "Do not actually make any changes. This is useful for checking for errors before committing anything, although filesystem errors may not be caught.");
    options.emplace_back("mode", 'M', 1, "Specify what to do with the file if it exists. Can be: copy, move, no-move", "<mode>");
    options.emplace_back("recursive", 'r', 2, "Recursively move all tags in a directory. This will fail if a tag is present in both the old and new directories, it cannot be used with no-move. This can only be specified once per operation and cannot be used with --tag.", "<f> <t>");
    options.emplace_back("tag", 'T', 2, "Refactor an individual tag. This can be specified multiple times but cannot be used with --recursive.", "<f> <t>");
    options.emplace_back("class", 'c', 2, "Refactor all tags of a given class to another class. All tags in the destination class must exist. This can be specified multiple times but cannot be used with --recursive or -M move.", "<f> <t>");
    options.emplace_back("single-tag", 's', 1, "Make changes to a single tag, only, rather than the whole tag directory.", "<path>");

    static constexpr char DESCRIPTION[] = "Find and replace tag references.";
    static constexpr char USAGE[] = "<-M <mode>> [options]";

    struct RefactorOptions {
        std::vector<std::string> tags;
        bool dry_run = false;
        std::optional<RefactorMode> mode;
        const char *single_tag = nullptr;

        std::vector<std::pair<TagFilePath, TagFilePath>> replacements;
        std::vector<std::pair<Invader::HEK::TagClassInt, Invader::HEK::TagClassInt>> class_replacements;
        std::vector<std::pair<Invader::HEK::TagClassInt, Invader::HEK::TagClassInt>> reverse_class_replacements;
        std::optional<std::pair<std::string, std::string>> recursive;
    } refactor_options;

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<RefactorOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, refactor_options, [](char opt, const std::vector<const char *> &arguments, auto &refactor_options) {
        auto get_class = [](auto *argument) -> Invader::HEK::TagClassInt {
            auto tag_class = Invader::HEK::extension_to_tag_class(argument);
            if(!tag_class) {
                eprintf_error("Error: %s is not a valid tag class", argument);
                std::exit(EXIT_FAILURE);
            }
            return tag_class;
        };
        
        switch(opt) {
            case 't':
                refactor_options.tags.emplace_back(arguments[0]);
                return;
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
                return;
            case 'M':
                if(std::strcmp(arguments[0], "move") == 0) {
                    refactor_options.mode = RefactorMode::REFACTOR_MODE_MOVE;
                }
                else if(std::strcmp(arguments[0], "no-move") == 0) {
                    refactor_options.mode = RefactorMode::REFACTOR_MODE_NO_MOVE;
                }
                else if(std::strcmp(arguments[0], "copy") == 0) {
                    refactor_options.mode = RefactorMode::REFACTOR_MODE_COPY;
                }
                else {
                    eprintf_error("Unknown mode %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                return;
            case 'r':
                refactor_options.recursive = { arguments[0], arguments[1] };
                return;
            case 'c':
                refactor_options.class_replacements.emplace_back(get_class(arguments[0]), get_class(arguments[1]));
                return;
            case 'T':
                try {
                    refactor_options.replacements.emplace_back(split_tag_class_extension(preferred_path_to_halo_path(arguments[0])).value(), split_tag_class_extension(preferred_path_to_halo_path(arguments[1])).value());
                }
                catch(std::bad_optional_access &) {
                    eprintf_error("Error: Invalid path pair: \"%s\" and \"%s\"", arguments[0], arguments[1]);
                    std::exit(EXIT_FAILURE);
                }
                return;
            case 'D':
                refactor_options.dry_run = true;
                return;
            case 's':
                refactor_options.single_tag = arguments[0];
                return;
        }
    });

    auto &replacements = refactor_options.replacements;
    auto &class_replacements = refactor_options.class_replacements;
    
    if(replacements.size() && refactor_options.recursive) {
        eprintf_error("Error: --recursive and --tag cannot be used at the same time");
        return EXIT_FAILURE;
    }
    
    if(class_replacements.size() && refactor_options.recursive) {
        eprintf_error("Error: --recursive and --class cannot be used at the same time");
        return EXIT_FAILURE;
    }
    
    if(!refactor_options.mode.has_value()) {
        eprintf_error("Error: No mode specified. Use -h for more information.");
        return EXIT_FAILURE;
    }

    if(*refactor_options.mode == RefactorMode::REFACTOR_MODE_NO_MOVE && refactor_options.recursive) {
        eprintf_error("Error: -M no-move and --recursive cannot be used at the same time");
        return EXIT_FAILURE;
    }
    
    auto move_or_copy_file = (*refactor_options.mode == RefactorMode::REFACTOR_MODE_MOVE || *refactor_options.mode == RefactorMode::REFACTOR_MODE_COPY);

    if(refactor_options.tags.size() == 0) {
        refactor_options.tags.emplace_back("tags");
    }

    // Figure out what we need to do
    std::vector<TagFile *> replacements_files;
    std::vector<TagFile> all_tags = load_virtual_tag_folder(refactor_options.tags);
    std::vector<TagFile> single_tag;
    std::vector<TagFile> *tag_to_modify;
    
    // Resolve all class replacements
    for(auto &r : class_replacements) {
        for(auto &t : all_tags) {
            if(t.tag_class_int == r.second) {
                auto split_from = *Invader::File::split_tag_class_extension(Invader::File::preferred_path_to_halo_path(t.tag_path));
                auto split_to = split_from;
                split_from.class_int = r.first;
                refactor_options.replacements.emplace_back(split_from, split_to);
            }
        }
    }
    
    // Make sure we aren't changing tag classes if move
    if(move_or_copy_file) {
        for(auto &t : refactor_options.replacements) {
            if(t.first.class_int != t.second.class_int) {
                eprintf_error("Error: Tag classes cannot be changed with -M move or -M copy");
                return EXIT_FAILURE;
            }
        }
    }
    
    // Next, any refactors that are just refactoring the tag to itself (effectively no operation)
    std::size_t index = 0;
    while(index < refactor_options.replacements.size()) {
        auto &replacement = refactor_options.replacements[index];
        if(replacement.first == replacement.second) {
            refactor_options.replacements.erase(refactor_options.replacements.begin() + index);
        }
        else {
            index++;
        }
    }
    
    // Do we only need to go through one tag?
    if(refactor_options.single_tag) {
        tag_to_modify = &single_tag;

        // Add this tag to the end
        auto &tag = single_tag.emplace_back();
        auto single_tag_maybe = split_tag_class_extension(halo_path_to_preferred_path(refactor_options.single_tag));
        if(!single_tag_maybe.has_value()) {
            eprintf_error("Error: %s is not a valid tag path", refactor_options.single_tag);
            return EXIT_FAILURE;
        }

        // Get the path together
        tag.tag_class_int = single_tag_maybe->class_int;
        tag.tag_path = single_tag_maybe->path + "." + tag_class_to_extension(single_tag_maybe->class_int);

        // Find it
        auto file_path_maybe = Invader::File::tag_path_to_file_path(tag.tag_path, refactor_options.tags, true);
        if(!file_path_maybe.has_value()) {
            eprintf_error("Error: %s was not found in any tag directory", refactor_options.single_tag);
            return EXIT_FAILURE;
        }

        tag.full_path = *file_path_maybe;
    }
    else {
        tag_to_modify = &all_tags;
    }

    auto unmaybe = [](const auto &value, const char *arg) -> const auto & {
        if(!value.has_value()) {
            eprintf_error("Error: %s is not a valid reference", arg);
            std::exit(EXIT_FAILURE);
        }
        return value.value();
    };

    // If recursive, we need to go through each tag in the tag directory for a match
    if(refactor_options.recursive.has_value()) {
        auto from_halo = remove_trailing_slashes(preferred_path_to_halo_path(refactor_options.recursive->first));
        auto from_halo_size = from_halo.size();
        auto to_halo = remove_trailing_slashes(preferred_path_to_halo_path(refactor_options.recursive->second));

        // Go through each tag to find a match
        for(auto &t : all_tags) {
            auto halo_path = preferred_path_to_halo_path(t.tag_path);
            if(halo_path.find(from_halo) == 0 && halo_path[from_halo_size] == '\\') {
                TagFilePath from = unmaybe(split_tag_class_extension(halo_path), t.tag_path.c_str());
                TagFilePath to = {to_halo + from.path.substr(from_halo_size), t.tag_class_int};
                replacements.emplace_back(std::move(from), std::move(to));
                replacements_files.emplace_back(&t);
            }
        }

        if(replacements.size() == 0) {
            eprintf_error("No tags were found in %s", halo_path_to_preferred_path(from_halo).c_str());
            return EXIT_FAILURE;
        }
    }
    else {
        if(move_or_copy_file) {
            for(auto &i : replacements) {
                bool added = false;
                auto joined = i.first.join();
                for(auto &t : all_tags) {
                    if(preferred_path_to_halo_path(t.tag_path) == joined) {
                        replacements_files.emplace_back(&t);
                        added = true;
                        break;
                    }
                }

                if(!added) {
                    eprintf_error("Error: %s was not found.", joined.c_str());
                    return EXIT_FAILURE;
                }
            }
        }

        // If we're moving tags, we can't change tag classes
        if(move_or_copy_file && !refactor_options.dry_run) {
            for(auto &i : replacements) {
                if(i.first.class_int != i.second.class_int) {
                    eprintf_error("Error: Tag class cannot be changed if moving tags.");
                    return EXIT_FAILURE;
                }
            }
        }
    }

    // Go through all the tags and see what needs edited
    std::size_t total_tags = 0;
    std::size_t total_replaced = 0;
    std::vector<TagFile *> tags_to_do;

    for(auto &tag : *tag_to_modify) {
        if(refactor_tags(tag.full_path.string().c_str(), replacements, true, refactor_options.dry_run)) {
            tags_to_do.emplace_back(&tag);
        }
    }

    // Now actually do it
    for(auto *tag : tags_to_do) {
        std::size_t count = refactor_tags(tag->full_path.string().c_str(), replacements, false, refactor_options.dry_run);
        if(count) {
            total_replaced += count;
            total_tags++;
        }
    }

    oprintf("Replaced %zu reference%s in %zu tag%s\n", total_replaced, total_replaced == 1 ? "" : "s", total_tags, total_tags == 1 ? "" : "s");

    // Move everything
    if(!refactor_options.dry_run && move_or_copy_file) {
        auto replacement_count = replacements.size();
        bool deleted_error_shown = false;
        for(std::size_t i = 0; i < replacement_count; i++) {
            auto *file = replacements_files[i];
            auto &replacement = replacements[i];

            auto new_path = std::filesystem::path(refactor_options.tags[file->tag_directory]) / (halo_path_to_preferred_path(replacement.second.path) + "." + tag_class_to_extension(replacement.second.class_int));

            // Create directories. If this fails, it probably matters, but it's not critical in and of itself
            try {
                std::filesystem::create_directories(new_path.parent_path());
            }
            catch(std::exception &) {}

            // Rename, copying as a last resort
            bool renamed = false;
            try {
                switch(*refactor_options.mode) {
                    case RefactorMode::REFACTOR_MODE_MOVE:
                        std::filesystem::rename(file->full_path, new_path);
                        break;
                    case RefactorMode::REFACTOR_MODE_COPY:
                        std::filesystem::copy(file->full_path, new_path);
                        break;
                    default:
                        std::terminate();
                }
                renamed = true;
            }
            catch(std::exception &e) {
                try {
                    std::filesystem::copy(file->full_path, new_path);
                    eprintf_error("Error: Failed to move %s to %s, thus it was copied instead: %s\n", file->full_path.string().c_str(), new_path.string().c_str(), e.what());
                }
                catch(std::exception &e) {
                    eprintf_error("Error: Failed to move or copy %s to %s: %s\n", file->full_path.string().c_str(), new_path.string().c_str(), e.what());
                }
            }

            // Lastly, delete empty directories if possible, obviously doing that only if renamed
            if(renamed) {
                auto delete_directory_if_empty = [](std::filesystem::path directory, auto &delete_directory_if_empty, int depth) -> bool {
                    if(++depth == 256) {
                        return false;
                    }
                    for(auto &i : std::filesystem::directory_iterator(directory)) {
                        if(!i.is_directory() || !delete_directory_if_empty(i.path(), delete_directory_if_empty, depth)) {
                            return false;
                        }
                    }
                    std::filesystem::remove_all(directory);
                    return true;
                };
                try {
                    auto parent = file->full_path.parent_path();
                    while(delete_directory_if_empty(parent, delete_directory_if_empty, 0)) {
                        parent = parent.parent_path();
                    }
                }
                catch(std::exception &) {
                    deleted_error_shown = true;
                }
            }
        }
        if(deleted_error_shown) {
            eprintf_error("Error: Failed to delete some empty directories");
        }
    }
}
