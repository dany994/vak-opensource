/*
 * MRF24WG Universal Driver Endian control
 *
 * Summary: This module contains endian defintions
 */
#ifndef __WF_ENDIAN_H
#define __WF_ENDIAN_H

// if host is little-endian then need to convert as MRF24WG is big-endian
#if defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
    #define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                      (((uint16_t)(A) & 0x00ff) << 8))

    #define htonl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
                      (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                      (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                      (((uint32_t)(A) & 0x000000ff) << 24))
    #define ntohs  htons

#define ntohl  htonl

// else if host is big-endian, no need to convert
#elif defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
    #define htons(A) (A)
    #define htonl(A) (A)
    #define ntohs(A) (A)
    #define ntohl(A) (A)
#else
    #error "Must define either BIG_ENDIAN or LITTLE_ENDIAN"
#endif

#endif /* __WF_ENDIAN_H */
