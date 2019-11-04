// SPDX-License-Identifier: GPL-3.0-only

#include <tiffio.h>
#include <invader/bitmap/image_loader.hpp>
#include <invader/printf.hpp>
#include "stb/stb_image.h"

namespace Invader {
    #define ALLOCATE_PIXELS(count) reinterpret_cast<Invader::ColorPlatePixel *>(calloc(sizeof(Invader::ColorPlatePixel) * pixel_count, 1))

    static Invader::ColorPlatePixel *rgba_to_pixel(const std::uint8_t *data, std::size_t pixel_count) {
        auto *pixel_data = ALLOCATE_PIXELS(pixel_count);

        for(std::size_t i = 0; i < pixel_count; i++) {
            pixel_data[i].alpha = data[3];
            pixel_data[i].red = data[0];
            pixel_data[i].green = data[1];
            pixel_data[i].blue = data[2];
            data += 4;
        }

        return pixel_data;
    }

    #undef ALLOCATE_PIXELS

    Invader::ColorPlatePixel *load_image(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
        // Load it
        int x = 0, y = 0, channels = 0;
        auto *image_buffer = stbi_load(path, &x, &y, &channels, 4);
        if(!image_buffer) {
            eprintf("Failed to load %s. Error was: %s\n", path, stbi_failure_reason());
            exit(EXIT_FAILURE);
        }

        // Get the width and height
        image_width = static_cast<std::uint32_t>(x);
        image_height = static_cast<std::uint32_t>(y);
        image_size = image_width * image_height * sizeof(Invader::ColorPlatePixel);

        // Do the thing
        Invader::ColorPlatePixel *return_value = rgba_to_pixel(reinterpret_cast<std::uint8_t *>(image_buffer), image_width * image_height);

        // Free the buffer
        stbi_image_free(image_buffer);

        return return_value;
    }

    Invader::ColorPlatePixel *load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
        TIFF *image_tiff = TIFFOpen(path, "r");
        if(!image_tiff) {
            eprintf("Cannot open %s\n", path);
            exit(EXIT_FAILURE);
        }
        TIFFGetField(image_tiff, TIFFTAG_IMAGEWIDTH, &image_width);
        TIFFGetField(image_tiff, TIFFTAG_IMAGELENGTH, &image_height);

        // Force associated alpha so alpha doesn't get multiplied in TIFFReadRGBAImageOriented
        uint16_t ua[] = { EXTRASAMPLE_ASSOCALPHA };
        TIFFSetField(image_tiff, TIFFTAG_EXTRASAMPLES, 1, ua);

        // Read it all
        image_size = image_width * image_height * sizeof(Invader::ColorPlatePixel);
        auto *image_pixels = reinterpret_cast<Invader::ColorPlatePixel *>(std::calloc(image_size, 1));
        TIFFReadRGBAImageOriented(image_tiff, image_width, image_height, reinterpret_cast<std::uint32_t *>(image_pixels), ORIENTATION_TOPLEFT);

        // Close the TIFF
        TIFFClose(image_tiff);

        // Swap red and blue channels
        for(std::size_t i = 0; i < image_size / 4; i++) {
            Invader::ColorPlatePixel swapped = image_pixels[i];
            swapped.red = image_pixels[i].blue;
            swapped.blue = image_pixels[i].red;
            image_pixels[i] = swapped;
        }

        return image_pixels;
    }
}
