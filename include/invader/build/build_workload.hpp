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
#include "../error_handler/error_handler.hpp"

namespace Invader {
    class BuildWorkload : public ErrorHandler {
    public:
        struct BuildParameters {
            /**
            * Select how much is output
            */
            enum BuildVerbosity {
                /** Everything is hidden */
                BUILD_VERBOSITY_QUIET,
                
                /** Errors are hidden */
                BUILD_VERBOSITY_HIDE_ERRORS,
                
                /** All warnings are hidden */
                BUILD_VERBOSITY_HIDE_WARNINGS,
                
                /** Pedantic warnings are hidden */
                BUILD_VERBOSITY_HIDE_PEDANTIC,
                
                /** Everything is output */
                BUILD_VERBOSITY_SHOW_ALL
            };
            
            /**
             * Scenario tag path with extension
             */
            std::string scenario;
            
            /**
             * New scenario name?
             */
            std::optional<std::string> rename_scenario;
            
            /**
             * Tags directories to use
             */
            std::vector<std::filesystem::path> tags_directories;
            
            /**
             * Index to use
             */
            std::optional<std::vector<File::TagFilePath>> index;
            
            /**
             * Bitmap data
             */
            std::optional<std::vector<Resource>> bitmap_data;
            
            /**
             * Sound data
             */
            std::optional<std::vector<Resource>> sound_data;
            
            /**
             * Loc data
             */
            std::optional<std::vector<Resource>> loc_data;
            
            /**
             * How verbose to make the output
             */
            BuildVerbosity verbosity = BuildVerbosity::BUILD_VERBOSITY_SHOW_ALL;
            
            /**
             * Forge the CRC32?
             */
            std::optional<std::uint32_t> forge_crc;
        
            /**
             * Optimize for space?
             */
            bool optimize_space = false;
            
            /**
             * Control how cache files are built. Changing these may result in an incompatible cache file
             */
            struct BuildParametersDetails {
                /**
                * Select how raw data is handled
                */
                enum RawDataHandling {
                    /** Retain all raw data in the map */
                    RAW_DATA_HANDLING_RETAIN_ALL,
                    
                    /** Retain only if it isn't found in the resource maps */
                    RAW_DATA_HANDLING_RETAIN_AUTOMATICALLY,
                    
                    /** (Custom Edition only) Always assume resource files' assets match */
                    RAW_DATA_HANDLING_ALWAYS_INDEX
                };
                
                /**
                 * Cache file engine to put in the header
                 */
                HEK::CacheFileEngine build_cache_file_engine;
                 
                /**
                 * Tag data address to use
                 */
                std::uint64_t build_tag_data_address;
                
                /**
                 * Maximum tag space to use
                 */
                std::uint64_t build_maximum_tag_space;
                
                /**
                 * Maximum cache file size to use
                 */
                std::uint64_t build_maximum_cache_file_size;
                
                /**
                 * Compress?
                 */
                bool build_compress;
                
                /**
                 * Compression level to use if compressing
                 */
                std::optional<int> compression_level;
                
                /**
                 * Use MCC-style compression?
                 */
                bool build_compress_mcc;
                
                /**
                 * Do BSPs occupy tag space?
                 */
                bool build_bsps_occupy_tag_space;
            
                /**
                 * How to handle raw data handling
                 */
                RawDataHandling build_raw_data_handling;
                
                /**
                 * Version to use. This generally only impacts Xbox maps as it's unread in all of the official PC releases.
                 */
                std::string build_version;
                
                /**
                 * Instantiate some default, working parameters for the target engine
                 * @param engine engine
                 */
                BuildParametersDetails(HEK::CacheFileEngine engine) noexcept;
                
                BuildParametersDetails(const BuildParametersDetails &) = default;
                BuildParametersDetails(BuildParametersDetails &&) = default;
            } details;
                
            /**
             * Instantiate some default, working parameters for the target engine
             * @param scenario scenario tag path to use
             * @param tags     tags directories to use
             * @param engine   engine target to use
             */
            BuildParameters(const std::string &scenario, const std::vector<std::filesystem::path> &tags_directories, HEK::CacheFileEngine engine);
                
            /**
             * Instantiate a simple set of parameters
             * @param engine engine target to use
             */
            BuildParameters(HEK::CacheFileEngine engine = HEK::CacheFileEngine::CACHE_FILE_NATIVE) noexcept;
            
            BuildParameters(const BuildParameters &) = default;
            BuildParameters(BuildParameters &&) = default;
        };
        
        /**
         * Compile a map
         * @param parameters build parameters to use
         */
        static std::vector<std::byte> compile_map(const BuildParameters &parameters);

        /**
         * Compile a single tag
         * @param tag               tag to use
         * @param tag_class_int     tag class
         * @param tags_directories  tags directories to use
         * @param recursion         should use recursion
         * @param error_checking    have error checking besides completely invalid tags
         */
        static BuildWorkload compile_single_tag(const char *tag, TagClassInt tag_class_int, const std::vector<std::filesystem::path> &tags_directories, bool recursion = false, bool error_checking = false);

        /**
         * Compile a single tag
         * @param tag_data          tag data to use
         * @param tag_data_size     tag class
         * @param tags_directories  tags directories to use
         * @param recursion         should use recursion
         * @param error_checking    have error checking besides completely invalid tags
         */
        static BuildWorkload compile_single_tag(const std::byte *tag_data, std::size_t tag_data_size, const std::vector<std::filesystem::path> &tags_directories = std::vector<std::filesystem::path>(), bool recursion = false, bool error_checking = false);

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

            /** We're only limited to 32-bit pointers */
            bool limit_to_32_bits = false;
            
            /** Data offset of the depended struct */
            std::size_t struct_data_offset = 0;

            bool operator==(const BuildWorkloadStructPointer &other) const noexcept {
                return this->struct_index == other.struct_index && this->offset == other.offset && this->struct_data_offset == other.struct_data_offset;
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
             * Resolve the pointer at the given address
             * @param pointer_pointer pointer to look at
             * @return                the struct index if found
             */
            std::optional<std::size_t> resolve_pointer(const HEK::LittleEndian<HEK::Pointer64> *pointer_pointer) const noexcept {
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

            /** Original tag class, if applicable */
            std::optional<TagClassInt> alias;

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

        /** Structs being worked with */
        std::vector<BuildWorkloadStruct> structs;

        /** Uncompressed vertices for models */
        std::vector<Parser::ModelVertexUncompressed::struct_little> uncompressed_model_vertices;

        /** Compressed vertices for models */
        std::vector<Parser::ModelVertexCompressed::struct_little> compressed_model_vertices;

        /** Indices for models */
        std::vector<HEK::LittleEndian<HEK::Index>> model_indices;
        
        struct BuildWorkloadModelPart {
            /** index of the struct the part is in */
            std::size_t struct_index;
            
            /** offset of the part */
            std::size_t offset;
        };
        
        /** Model data parts' struct indices and their offsets */
        std::vector<BuildWorkloadModelPart> model_parts;

        /** Raw data for bitmaps and sounds */
        std::vector<std::vector<std::byte>> raw_data;

        /** Tags being worked with */
        std::vector<BuildWorkloadTag> tags;

        /** BSP count */
        std::size_t bsp_count = 0;

        /** Cache file type */
        std::optional<HEK::CacheFileType> cache_file_type;

        /** Recursion is disabled - also disables showing various tags using other tags' data */
        bool disable_recursion = false;

        /** Disabling error checking (besides completely invalid tag data) */
        bool disable_error_checking = false;

        /** Are we building a stock map? */
        bool building_stock_map = false;
        
        /** 
         * Get the build parameters
         * @return build parameters
         */
        const BuildParameters *get_build_parameters() const noexcept {
            return this->parameters;
        }

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
        
        ~BuildWorkload() override = default;

    private:
        BuildWorkload();

        std::chrono::steady_clock::time_point start;
        const char *scenario;
        std::size_t scenario_index;
        std::vector<std::byte> build_cache_file();
        void add_tags();
        void generate_tag_array();
        void dedupe_structs();
        std::vector<std::vector<std::byte>> map_data_structs;
        std::vector<std::byte> all_raw_data;
        std::size_t generate_tag_data();
        void generate_bitmap_sound_data(std::size_t file_offset);
        HEK::TagString scenario_name = {};
        void set_scenario_name(const char *name);
        std::size_t raw_bitmap_size = 0;
        std::size_t raw_sound_size = 0;
        void externalize_tags() noexcept;
        void delete_raw_data(std::size_t index);
        std::size_t stubbed_tag_count = 0;
        std::size_t indexed_data_amount = 0;
        std::size_t raw_data_indices_offset;
        std::uint32_t tag_file_checksums = 0;
        const BuildParameters *parameters = nullptr;
        void generate_compressed_model_tag_array();
    };
}

#endif
