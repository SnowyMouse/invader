// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__RESOURCE__RESOURCE_MAP_HPP
#define INVADER__RESOURCE__RESOURCE_MAP_HPP

#include <cstddef>
#include <string>
#include <vector>

namespace Invader {
    struct Resource {
        std::string path;
        std::vector<std::byte> data;
        std::size_t path_offset;
        std::size_t data_offset;
    };

    /**
     * Return an array of containers for the given resource map
     * @param  data pointer to resource data
     * @param  size size of resource data
     * @return      array of containers
     * @throws      if failed
     */
    std::vector<Resource> load_resource_map(const std::byte *data, std::size_t size);

    /**
     * Return an array of containers for the given ipak
     * @param  data data pointer to ipak
     * @param  size size of ipak
     * @return      array of containers
     * @throws      if failed
     */
    std::vector<Resource> load_ipak(const std::byte *data, std::size_t size);

    /**
     * Return an array of containers for the given compressed ipak
     * @param  data data pointer to ipak
     * @param  size size of ipak
     * @return      array of containers
     * @throws      if failed
     */
    std::vector<Resource> load_compressed_ipak(const std::byte *data, std::size_t size);
}
#endif
