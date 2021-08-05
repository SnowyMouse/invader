// SPDX-License-Identifier: GPL-3.0-only

#include <invader/resource/resource_map.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/file/file.hpp>

using namespace Invader::Parser;

namespace Invader {
    std::vector<Resource> load_resource_map(const std::byte *data, std::size_t size) {
        if(size < sizeof(ResourceMapHeader)) {
            throw OutOfBoundsException();
        }
        const auto &header = *reinterpret_cast<const ResourceMapHeader *>(data);
        
        // Check if it's valid
        switch(header.type) {
            case ResourceMapType::RESOURCE_MAP_BITMAP:
            case ResourceMapType::RESOURCE_MAP_SOUND:
            case ResourceMapType::RESOURCE_MAP_LOC:
                break;
            default:
                throw InvalidMapException();
        }
        
        std::size_t resource_count = header.resource_count;
        std::size_t resource_offset = header.resources;
        std::size_t path_offset = header.paths;
        if(resource_offset > size || resource_offset + sizeof(ResourceMapResource) * resource_count > size) {
            throw OutOfBoundsException();
        }
        const auto *resources = reinterpret_cast<const ResourceMapResource *>(data + resource_offset);

        std::vector<Resource> returned_resources;

        for(std::size_t r = 0; r < resource_count; r++) {
            std::size_t resource_data_offset = resources[r].data_offset;
            std::size_t resource_path_offset = resources[r].path_offset + path_offset;
            std::size_t resource_data_size = resources[r].size;

            const auto *resource_data = data + resource_data_offset;
            if(resource_data_offset >= size || resource_data_offset + resource_data_size > size) {
                throw OutOfBoundsException();
            }

            std::size_t resource_path_length = 0;
            if(resource_path_offset >= size) {
                throw OutOfBoundsException();
            }
            const auto *resource_path = reinterpret_cast<const char *>(data + resource_path_offset);
            for(;;resource_path_length++) {
                if(resource_path_length >= size) {
                    throw OutOfBoundsException();
                }
                else if(resource_path[resource_path_length] == 0) {
                    break;
                }
            }

            Resource resource;
            resource.path = Invader::File::remove_duplicate_slashes(resource_path);
            resource.data = std::vector<std::byte>(resource_data, resource_data + resource_data_size);
            resource.path_offset = resource_path_offset;
            resource.data_offset = resource_data_offset;

            returned_resources.push_back(resource);
        }

        return returned_resources;
    }
}
