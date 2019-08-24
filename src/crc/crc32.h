#ifndef INVADER__CRC__CRC32_H
#define INVADER__CRC__CRC32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
uint32_t crc32(uint32_t crc, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif
