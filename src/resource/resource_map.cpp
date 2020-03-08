// SPDX-License-Identifier: GPL-3.0-only

#include <invader/resource/resource_map.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/file/file.hpp>
#include <invader/compress/ceaflate.hpp>
#include <invader/resource/hek/ipak.hpp>

namespace Invader {
    std::vector<Resource> load_resource_map(const std::byte *data, std::size_t size) {
        using namespace HEK;
        if(size < sizeof(ResourceMapHeader)) {
            throw OutOfBoundsException();
        }
        const auto &header = *reinterpret_cast<const ResourceMapHeader *>(data);
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

    std::vector<Resource> load_ipak(const std::byte *data, std::size_t size) {
        using namespace HEK;

        auto count = *reinterpret_cast<const std::uint32_t *>(data);
        const auto *elements = reinterpret_cast<const IPAKElement *>(data + 0x8);
        std::vector<Resource> resources(count);

        for(std::size_t i = 0; i < count; i++) {
            auto &element = elements[i];
            auto this_offset = static_cast<std::size_t>(element.offset);
            auto this_size = i + 1 == count ? (size - this_offset) : (elements[i+1].offset - this_offset);
            resources[i].path = element.name;
            resources[i].data = std::vector<std::byte>(data + this_offset, data + this_offset + this_size);
            resources[i].data_offset = static_cast<std::size_t>(element.offset);
            resources[i].path_offset = reinterpret_cast<const std::byte *>(&element) - data;
        }
        return resources;
    }

    std::vector<Resource> load_compressed_ipak(const std::byte *data, std::size_t size) {
        using namespace Compression::Ceaflate;
        std::size_t decompressed_size = find_decompressed_file_size(data, size);
        std::vector<std::byte> ipak(decompressed_size);
        decompress_file(data, size, ipak.data(), decompressed_size);
        return load_ipak(ipak.data(), decompressed_size);
    }
}
