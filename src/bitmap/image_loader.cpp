// SPDX-License-Identifier: GPL-3.0-only

#include <tiffio.h>
#include "image_loader.hpp"
#include <invader/printf.hpp>
#include "stb/stb_image.h"

namespace Invader {
    static std::vector<Pixel> rgba_to_pixel(const std::uint8_t *data, std::size_t pixel_count) {
        auto pixel_data = std::vector<Pixel>(pixel_count);

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

    std::vector<Pixel> load_image(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
        // Load it
        int x = 0, y = 0, channels = 0;
        auto *image_buffer = stbi_load(path, &x, &y, &channels, 4);
        if(!image_buffer) {
            eprintf_error("Failed to load %s. Error was: %s", path, stbi_failure_reason());
            exit(EXIT_FAILURE);
        }

        // Get the width and height
        image_width = static_cast<std::uint32_t>(x);
        image_height = static_cast<std::uint32_t>(y);
        image_size = image_width * image_height * sizeof(Invader::Pixel);

        // Do the thing
        auto return_value = rgba_to_pixel(reinterpret_cast<std::uint8_t *>(image_buffer), image_width * image_height);

        // Free the buffer
        stbi_image_free(image_buffer);

        return return_value;
    }

    std::vector<Pixel> load_tiff(const char *path, std::uint32_t &image_width, std::uint32_t &image_height, std::size_t &image_size) {
        TIFF *image_tiff = TIFFOpen(path, "r");
        if(!image_tiff) {
            eprintf_error("Cannot open %s", path);
            exit(EXIT_FAILURE);
        }
        TIFFGetField(image_tiff, TIFFTAG_IMAGEWIDTH, &image_width);
        TIFFGetField(image_tiff, TIFFTAG_IMAGELENGTH, &image_height);

        // Force associated alpha if we have alpha so alpha doesn't get multiplied in TIFFReadRGBAImageOriented
        std::uint16_t count;
        std::uint16_t *attributes;
        int defined = TIFFGetField(image_tiff, TIFFTAG_EXTRASAMPLES, &count, &attributes);
        if(defined && count == 1) {
            if(*attributes == EXTRASAMPLE_UNASSALPHA) {
                std::uint16_t new_value = EXTRASAMPLE_ASSOCALPHA;
                TIFFSetField(image_tiff, TIFFTAG_EXTRASAMPLES, 1, &new_value);
            }
        }

        // Read it all
        image_size = image_width * image_height * sizeof(Invader::Pixel);
        auto image_pixels = std::vector<Invader::Pixel>(image_size);
        TIFFReadRGBAImageOriented(image_tiff, image_width, image_height, reinterpret_cast<std::uint32_t *>(image_pixels.data()), ORIENTATION_TOPLEFT);

        // Close the TIFF
        TIFFClose(image_tiff);

        // Swap red and blue channels
        for(std::size_t i = 0; i < image_size / 4; i++) {
            Invader::Pixel swapped = image_pixels[i];
            swapped.red = image_pixels[i].blue;
            swapped.blue = image_pixels[i].red;
            image_pixels[i] = swapped;
        }

        return image_pixels;
    }
}
