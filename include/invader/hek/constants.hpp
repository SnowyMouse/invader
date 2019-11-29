// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__CONSTANTS_HPP
#define INVADER__HEK__CONSTANTS_HPP

/**
 * Number of times the game performs most of its logic (physics, etc.)
 */
#define TICK_RATE 30.0F

/**
 * Number of world units per meter (1 world unit = 10 feet = 3.048 meters)
 */
#define WORLD_UNITS_PER_METER 3.048F

/**
 * Halo's gravity is equal to 9.78 meters per second squared, or approximately
 * 0.997 g (0.997 times Earth's gravity).
 */
#define GRAVITY ((9.78F / WORLD_UNITS_PER_METER) / TICK_RATE / TICK_RATE)

/**
 * From Guerilla's self-documentation, measured in grams per milliliter
 */
#define AIR_DENSITY 0.0011F
#define WATER_DENSITY 1.0F

/**
 * I know M_PI is an option, but from looking at tool's output, this is
 * approximately the constant Halo uses
 */
#define HALO_PI 3.14159274101257F

#endif
