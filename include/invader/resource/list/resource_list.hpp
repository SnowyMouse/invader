// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__RESOURCE__LIST__RESOURCE_LIST_HPP
#define INVADER__RESOURCE__LIST__RESOURCE_LIST_HPP

namespace Invader {
    /**
     * Get the default bitmap index that is hardcoded into the executable
     * @return bitmap index
     */
    const char * const *get_default_bitmap_resources() noexcept;

    /**
     * Get the number of bitmap resources in the bitmap index hardcoded into the executable
     * @return number of bitmap resources
     */
    unsigned long get_default_bitmap_resources_count() noexcept;

    /**
     * Get the default sound index that is hardcoded into the executable
     * @return sound index
     */
    const char * const *get_default_sound_resources() noexcept;

    /**
     * Get the number of sound resources in the sound index hardcoded into the executable
     * @return number of sound resources
     */
    unsigned long get_default_sound_resources_count() noexcept;

    /**
     * Get the default loc index that is hardcoded into the executable
     * @return loc index
     */
    const char * const *get_default_loc_resources() noexcept;

    /**
     * Get the number of loc resources in the loc index hardcoded into the executable
     * @return number of loc resources
     */
    unsigned long get_default_loc_resources_count() noexcept;
}

#endif
