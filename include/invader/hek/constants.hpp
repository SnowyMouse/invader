// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__CONSTANTS_HPP
#define INVADER__HEK__CONSTANTS_HPP

/**
 * Number of times the game performs most of its logic (physics, etc.). This applies to everything (INCLUDING PAL - see comment*).
 * 
 * *While the game runs at 25 Hz on PAL, Bungie never changed tool.exe to account for this when building cache files. Instead, they changed a few select values and a few animations.
 */
#define TICK_RATE 30.0F
#define TICK_RATE_RECIPROCOL (1.0F / TICK_RATE)

/**
 * Number of world units per meter (1 world unit = 10 feet = 3.048 meters)
 */
#define WORLD_UNITS_PER_METER 3.048F

/**
 * Halo's gravity is equal to 9.78 meters per second squared, or approximately
 * 0.997 g (0.997 times Earth's gravity).
 */
#define GRAVITY ((9.78 / WORLD_UNITS_PER_METER) / TICK_RATE / TICK_RATE)

/**
 * From Guerilla's self-documentation, measured in grams per milliliter
 */
#define AIR_DENSITY 0.0011F
#define WATER_DENSITY 1.0F

/**
 * Pi to 100 digits
 */
#define HALO_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

#endif
