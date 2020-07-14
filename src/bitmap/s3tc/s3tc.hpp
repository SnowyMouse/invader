#ifndef S3TC_H
#define S3TC_H

#include <cstdint>

namespace S3TCH {

std::uint32_t PackRGBA(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
void DecompressBlockDXT1(std::uint32_t x, std::uint32_t y, std::uint32_t width, const std::uint8_t *blockStorage, std::uint32_t *image);
void BlockDecompressImageDXT1(std::uint32_t width, std::uint32_t height, const std::uint8_t *blockStorage, std::uint32_t *image);
void DecompressBlockDXT5(std::uint32_t x, std::uint32_t y, std::uint32_t width, const std::uint8_t *blockStorage, std::uint32_t *image);
void BlockDecompressImageDXT5(std::uint32_t width, std::uint32_t height, const std::uint8_t *blockStorage, std::uint32_t *image);

}

#endif // S3TC_H
