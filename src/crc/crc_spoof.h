#ifndef INVADER__CRC__CRC_SPOOF_H
#define INVADER__CRC__CRC_SPOOF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct FakeFileHandle {
    uint8_t *data;
    uint64_t size;
    uint64_t offset;
} FakeFileHandle;

void   crc_spoof_fake_fclose(FakeFileHandle *f);
int    crc_spoof_fake_fseek(FakeFileHandle *f, long offset, int type);
void   crc_spoof_fake_rewind(FakeFileHandle *f);
int    crc_spoof_fake_fflush(FakeFileHandle *f);
int    crc_spoof_fake_fgetc(FakeFileHandle *f);
int    crc_spoof_fake_fputc(int c, FakeFileHandle *f);
int    crc_spoof_fake_feof(FakeFileHandle *f);
size_t crc_spoof_fake_fread(void *ptr, size_t size, size_t count, FakeFileHandle *f);

const char *crc_spoof_modify_file_crc32(FakeFileHandle *f, uint64_t offset, uint32_t newcrc, bool printstatus);
uint32_t crc_spoof_reverse_bits(uint32_t x);

#ifdef __cplusplus
}
#endif

#endif
