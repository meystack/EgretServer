#ifndef _WCRC32_RNG_H_
#define _WCRC32_RNG_H_



#ifdef __cplusplus
extern "C" {
#endif


unsigned long ucrc32   (unsigned long crc, const unsigned char *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif