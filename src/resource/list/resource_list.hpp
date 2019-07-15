/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__RESOURCE__LIST__RESOURCE_LIST_HPP
#define INVADER__RESOURCE__LIST__RESOURCE_LIST_HPP

/**
 * Get the default bitmap index that is hardcoded into the executable
 * @return bitmap index
 */
const char **get_default_bitmap_resources();

/**
 * Get the default sound index that is hardcoded into the executable
 * @return sound index
 */
const char **get_default_sound_resources();

/**
 * Get the default loc index that is hardcoded into the executable
 * @return loc index
 */
const char **get_default_loc_resources();

#endif
