// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BUILD__BUILD_WORKLOAD_HPP
#define INVADER__BUILD__BUILD_WORKLOAD_HPP

#include <vector>
#include <optional>
#include <string>
#include <filesystem>
#include <chrono>
#include "../hek/map.hpp"
#include "../resource/resource_map.hpp"
#include "../tag/parser/parser.hpp"

namespace Invader {
    class BuildWorkload {
    public:
        /**
         * Compile a map
         * @param scenario               scenario tag to use
         * @param tags_directories       tags directories to use
         * @param engine_target          target a specific engine
         * @param maps_directory         maps directory to use; ignored if building a Dark Circlet map
         * @param with_index             tag index to use
         * @param no_external_tags       do not use cached tags; ignored if building a Dark Circlet map
         * @param always_index_tags      always use cached tags; ignored if building a Dark Circlet map
         * @param verbose                output non-error messages to console
         * @param forge_crc              forge the CRC32 of the map
         * @param tag_data_address       address the tag data will be loaded to
         * @param rename_scenario        rename the scenario's base name (preserving the root path)
         * @param optimize_space         should dedupe structs
         * @param compress               Zstd-compress the resulting map file
         * @param hide_pedantic_warnings hide pedantic warnings
         */
        static std::vector<std::byte> compile_map (
            const char *scenario,
            const std::vector<std::string> &tags_directories,
            HEK::CacheFileEngine engine_target = HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET,
            std::string maps_directory = std::string(),
            bool no_external_tags = false,
            bool always_index_tags = false,
            bool verbose = false,
            const std::optional<std::vector<std::pair<TagClassInt, std::string>>> &with_index = std::nullopt,
            const std::optional<std::uint32_t> &forge_crc = std::nullopt,
            const std::optional<std::uint32_t> &tag_data_address = std::nullopt,
            const std::optional<std::string> &rename_scenario = std::nullopt,
            bool optimize_space = false,
            bool compress = false,
            bool hide_pedantic_warnings = false
        );

        /**
         * Compile a single tag
         * @param tag               tag to use
         * @param tag_class_int     tag class
         * @param tags_directories  tags directories to use
         * @param recursion         should use recursion
         */
        static BuildWorkload compile_single_tag(const char *tag, TagClassInt tag_class_int, const std::vector<std::string> &tags_directories, bool recursion = false);

        /**
         * Compile a single tag
         * @param tag_data          tag data to use
         * @param tag_data_size     tag class
         * @param tags_directories  tags directories to use
         * @param recursion         should use recursion
         */
        static BuildWorkload compile_single_tag(const std::byte *tag_data, std::size_t tag_data_size, const std::vector<std::string> &tags_directories = std::vector<std::string>(), bool recursion = false);

        /** Denotes an individual tag dependency */
        struct BuildWorkloadDependency {
            /** Index of the depended tag */
            std::size_t tag_index;

            /** Offset of the depended tag */
            std::size_t offset;

            /** It's just the tag ID */
            bool tag_id_only = false;

            bool operator==(const BuildWorkloadDependency &other) const noexcept {
                return this->tag_index == other.tag_index && this->offset == other.offset && this->tag_id_only == other.tag_id_only;
            }
        };

        /** Denotes a pointer to another struct */
        struct BuildWorkloadStructPointer {
            /** Index of the depended struct */
            std::size_t struct_index;

            /** Offset of the pointer */
            std::size_t offset;

            bool operator==(const BuildWorkloadStructPointer &other) const noexcept {
                return this->struct_index == other.struct_index && this->offset == other.offset;
            }
        };

        /** Denotes an individual tag struct */
        struct BuildWorkloadStruct {
            /** Data in the struct */
            std::vector<std::byte> data;

            /** Dependencies in the struct */
            std::vector<BuildWorkloadDependency> dependencies;

            /** Struct dependencies in the struct */
            std::vector<BuildWorkloadStructPointer> pointers;

            /** Offset of the struct in tag data if it's currently present */
            std::optional<std::size_t> offset;

            /** This struct cannot be deduped */
            bool unsafe_to_dedupe = false;

            /** BSP index */
            std::optional<std::size_t> bsp = 0;

            /**
             * Resolve the pointer
             * @param offset offset of the pointer
             * @return       the struct index if found
             */
            std::optional<std::size_t> resolve_pointer(std::size_t offset) const noexcept {
                for(auto &p : pointers) {
                    if(p.offset == offset) {
                        return p.struct_index;
                    }
                }
                return std::nullopt;
            }

            /**
             * Resolve the pointer at the given address
             * @param pointer_pointer pointer to look at
             * @return                the struct index if found
             */
            std::optional<std::size_t> resolve_pointer(const HEK::LittleEndian<HEK::Pointer> *pointer_pointer) const noexcept {
                return this->resolve_pointer(reinterpret_cast<const std::byte *>(pointer_pointer) - this->data.data());
            }

            /**
             * Get whether or not a struct can be safely deduped and replaced with this
             * @param  other other struct to check
             * @return       true if it can be
             */
            bool can_dedupe(const BuildWorkloadStruct &other) const noexcept;
        };

        /** Denotes an individual tag */
        struct BuildWorkloadTag {
            /** Path of the tag */
            std::string path;

            /** Class of the tag */
            TagClassInt tag_class_int;

            /** Asset data structs */
            std::vector<std::size_t> asset_data;

            /** Index of the tag (in a resource map) */
            std::optional<std::size_t> resource_index;

            /** Base struct index of the tag */
            std::optional<std::size_t> base_struct;

            /** The asset data is external */
            bool external_asset_data = false;

            /** The tag is stubbed out */
            bool stubbed = false;

            /** Tag path offset of the tag */
            std::size_t path_offset;
        };

        /** Bitmaps resources */
        std::vector<Resource> bitmaps;

        /** Sounds resources */
        std::vector<Resource> sounds;

        /** Loc resources */
        std::vector<Resource> loc;

        /** Structs being worked with */
        std::vector<BuildWorkloadStruct> structs;

        /** Vertices for models */
        std::vector<Parser::GBXModelVertexUncompressed::struct_little> model_vertices;

        /** Indices for models */
        std::vector<HEK::LittleEndian<HEK::Index>> model_indices;

        /** Raw data for bitmaps and sounds */
        std::vector<std::vector<std::byte>> raw_data;

        /** Tags being worked with */
        std::vector<BuildWorkloadTag> tags;

        /** BSP struct */
        std::optional<std::size_t> bsp_struct;

        /** BSP count */
        std::size_t bsp_count = 0;

        /** Cache file type */
        std::optional<HEK::CacheFileType> cache_file_type;

        /** Recursion is disabled - also disables showing most errors as well as various tags using other tags' data */
        bool disable_recursion = false;

        /** Hide pedantic warnings */
        bool hide_pedantic_warnings = false;

        /** Are we building a stock map? */
        bool building_stock_map = false;

        /** Engine target */
        HEK::CacheFileEngine engine_target;

        /** Part count */
        std::size_t part_count = 0;

        /**
         * Add the tag
         * @param tag_path      path of the tag
         * @param tag_class_int class of the tag
         * @return              index of the tag
         */
        std::size_t compile_tag_recursively(const char *tag_path, TagClassInt tag_class_int);

        /**
         * Compile the tag data
         * @param tag_data      path of the tag
         * @param tag_data_size size of the tag
         * @param tag_index     index of the tag
         * @param tag_class_int explicitly give a tag class
         */
        void compile_tag_data_recursively(const std::byte *tag_data, std::size_t tag_data_size, std::size_t tag_index, std::optional<TagClassInt> tag_class_int = std::nullopt);

        enum ErrorType {
            /**
             * These are warnings that can be turned off. They report issues that don't affect the game, but the user may want to be made aware of them.
             */
            ERROR_TYPE_WARNING_PEDANTIC,

            /**
             * These are warnings that cannot be turned off. They report issues that do affect the game but still result in a valid cache file.
             */
            ERROR_TYPE_WARNING,

            /**
             * These are errors that prevent saving the cache file. They report issues that result in a potentially unplayable yet still technically valid cache file.
             */
            ERROR_TYPE_ERROR,

            /**
             * These are errors that halt building the cache file. They report issues that result in invalid and/or unusable cache files.
             */
            ERROR_TYPE_FATAL_ERROR
        };

        /**
         * Report an error
         * @param type      error type
         * @param error     error message
         * @param tag_index tag index
         */
        void report_error(ErrorType type, const char *error, std::optional<std::size_t> tag_index = std::nullopt);

    private:
        BuildWorkload() = default;

        std::chrono::steady_clock::time_point start;
        const char *scenario;
        std::size_t scenario_index;
        std::uint32_t tag_data_address;
        std::size_t tag_data_size;
        std::vector<std::byte> build_cache_file();
        const std::vector<std::string> *tags_directories;
        void add_tags();
        void generate_tag_array();
        bool optimize_space;
        std::size_t warnings = 0;
        std::size_t errors = 0;
        void dedupe_structs();
        std::vector<std::vector<std::byte>> map_data_structs;
        std::vector<std::byte> all_raw_data;
        std::size_t generate_tag_data();
        void generate_bitmap_sound_data(std::size_t file_offset);
        HEK::TagString scenario_name = {};
        void set_scenario_name(const char *name);
        std::optional<std::uint32_t> forge_crc;
        bool verbose = false;
        std::size_t raw_bitmap_size = 0;
        std::size_t raw_sound_size = 0;
        bool compress = false;
        bool always_index_tags = false;
        void april_fools() const noexcept;
        void externalize_tags() noexcept;
        void delete_raw_data(std::size_t index);
        std::size_t stubbed_tag_count = 0;
        std::size_t indexed_data_amount = 0;
    };

    #define REPORT_ERROR_PRINTF(workload, type, tag_index, ...) { \
        char report_error_message[2048]; \
        std::snprintf(report_error_message, sizeof(report_error_message), __VA_ARGS__); \
        (workload).report_error(Invader::BuildWorkload::ErrorType::type, report_error_message, tag_index); \
    }
}

#endif
