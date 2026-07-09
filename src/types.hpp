// Copyright 2024 OpenJE

#ifndef TYPES_HPP_
#define TYPES_HPP_

typedef signed char        int8;
typedef signed short       int16;
typedef signed int         int32;

#ifdef _MSC_VER
typedef signed __int64     int64;
typedef unsigned __int64   uint64;
#else
typedef signed long long   int64;
typedef unsigned long long uint64;
#endif

typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;

typedef uint8              _BYTE;
typedef uint16             _WORD;
typedef uint32             _DWORD;
typedef uint64             _QWORD;

typedef uint8              BYTE;
typedef uint16             WORD;
typedef uint32             DWORD;
typedef uint64             QWORD;

typedef uint8              byte;
typedef uint16             word;
typedef uint32             dword;
typedef uint64             qword;

typedef uint8              undefined;
typedef uint16             undefined2;
typedef uint32             undefined4;
typedef uint64             undefined8;

typedef void               code;
typedef long double        float10;

typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef uint64             ulonglong;

//
// IDA partial-access helpers
//

#define BYTEn(x, n)   (*((_BYTE  *)&(x) + (n)))
#define WORDn(x, n)   (*((_WORD  *)&(x) + (n)))
#define DWORDn(x, n)  (*((_DWORD *)&(x) + (n)))

#define SBYTEn(x, n)  (*((int8  *)&(x) + (n)))
#define SWORDn(x, n)  (*((int16 *)&(x) + (n)))
#define SDWORDn(x, n) (*((int32 *)&(x) + (n)))

#define BYTE0(x)      BYTEn(x, 0)
#define BYTE1(x)      BYTEn(x, 1)
#define BYTE2(x)      BYTEn(x, 2)
#define BYTE3(x)      BYTEn(x, 3)
#define BYTE4(x)      BYTEn(x, 4)
#define BYTE5(x)      BYTEn(x, 5)
#define BYTE6(x)      BYTEn(x, 6)
#define BYTE7(x)      BYTEn(x, 7)

#define WORD0(x)      WORDn(x, 0)
#define WORD1(x)      WORDn(x, 1)
#define WORD2(x)      WORDn(x, 2)
#define WORD3(x)      WORDn(x, 3)

#define DWORD0(x)     DWORDn(x, 0)
#define DWORD1(x)     DWORDn(x, 1)

#define SBYTE0(x)     SBYTEn(x, 0)
#define SBYTE1(x)     SBYTEn(x, 1)
#define SBYTE2(x)     SBYTEn(x, 2)
#define SBYTE3(x)     SBYTEn(x, 3)
#define SBYTE4(x)     SBYTEn(x, 4)
#define SBYTE5(x)     SBYTEn(x, 5)
#define SBYTE6(x)     SBYTEn(x, 6)
#define SBYTE7(x)     SBYTEn(x, 7)

#define SWORD0(x)     SWORDn(x, 0)
#define SWORD1(x)     SWORDn(x, 1)
#define SWORD2(x)     SWORDn(x, 2)
#define SWORD3(x)     SWORDn(x, 3)

#define LOBYTE(x)     BYTE0(x)
#define HIBYTE(x)     BYTE1(x)
#define LOWORD(x)     WORD0(x)
#define HIWORD(x)     WORD1(x)
#define LODWORD(x)    DWORD0(x)
#define HIDWORD(x)    DWORD1(x)

#define SLOBYTE(x)    SBYTE0(x)
#define SHIBYTE(x)    SBYTE1(x)
#define SLOWORD(x)    SWORD0(x)
#define SHIWORD(x)    SWORD1(x)


//
// IDA pair helpers
//
// These replace Ghidra CONCAT-style operations.
//

#define __PAIR16__(high, low) \
    ((uint16)(((uint16)(uint8)(high) << 8) | (uint8)(low)))

#define __PAIR32__(high, low) \
    ((uint32)(((uint32)(uint16)(high) << 16) | (uint16)(low)))

#define __PAIR64__(high, low) \
    ((uint64)(((uint64)(uint32)(high) << 32) | (uint32)(low)))

#define __SPAIR16__(high, low) \
    ((int16)(((int16)(int8)(high) << 8) | (uint8)(low)))

#define __SPAIR32__(high, low) \
    ((int32)(((int32)(int16)(high) << 16) | (uint16)(low)))

#define __SPAIR64__(high, low) \
    ((int64)(((int64)(int32)(high) << 32) | (uint32)(low)))


//
// IDA rotate helpers
//

#define __ROL1__(x, count) \
    ((uint8)(((uint8)(x) << ((count) & 7)) | ((uint8)(x) >> ((8 - ((count) & 7)) & 7))))

#define __ROR1__(x, count) \
    ((uint8)(((uint8)(x) >> ((count) & 7)) | ((uint8)(x) << ((8 - ((count) & 7)) & 7))))

#define __ROL2__(x, count) \
    ((uint16)(((uint16)(x) << ((count) & 15)) | ((uint16)(x) >> ((16 - ((count) & 15)) & 15))))

#define __ROR2__(x, count) \
    ((uint16)(((uint16)(x) >> ((count) & 15)) | ((uint16)(x) << ((16 - ((count) & 15)) & 15))))

#define __ROL4__(x, count) \
    ((uint32)(((uint32)(x) << ((count) & 31)) | ((uint32)(x) >> ((32 - ((count) & 31)) & 31))))

#define __ROR4__(x, count) \
    ((uint32)(((uint32)(x) >> ((count) & 31)) | ((uint32)(x) << ((32 - ((count) & 31)) & 31))))

#define __ROL8__(x, count) \
    ((uint64)(((uint64)(x) << ((count) & 63)) | ((uint64)(x) >> ((64 - ((count) & 63)) & 63))))

#define __ROR8__(x, count) \
    ((uint64)(((uint64)(x) >> ((count) & 63)) | ((uint64)(x) << ((64 - ((count) & 63)) & 63))))

#define __ROL__(x, count) __ROL4__((x), (count))
#define __ROR__(x, count) __ROR4__((x), (count))

#define __RCL__(x, y) invalid_operation
#define __RCR__(x, y) invalid_operation


//
// IDA flag helpers
//

#define __MKCSHL__(x, count) \
    (((x) >> (8 * sizeof(x) - (count))) & 1)

#define __MKCSHR__(x, count) \
    (((x) >> ((count) - 1)) & 1)

#define __SETS__(x) \
    ((int8)((x) >> (8 * sizeof(x) - 1)) < 0)

#define __CFADD__(x, y) \
    ((uint64)(x) + (uint64)(y) < (uint64)(x))

#define __CFSUB__(x, y) \
    ((uint64)(x) < (uint64)(y))

#define __OFADD__(x, y) \
    (((~((x) ^ (y))) & ((x) ^ ((x) + (y)))) >> (8 * sizeof(x) - 1))

#define __OFSUB__(x, y) \
    ((((x) ^ (y)) & ((x) ^ ((x) - (y)))) >> (8 * sizeof(x) - 1))

#define __SETP__(x, y) invalid_operation


//
// IDA convenience helpers
//

#define __BYTE_SWAP16__(x) \
    ((uint16)((((uint16)(x) & 0x00FF) << 8) | (((uint16)(x) & 0xFF00) >> 8)))

#define __BYTE_SWAP32__(x) \
    ((uint32)((((uint32)(x) & 0x000000FF) << 24) | \
              (((uint32)(x) & 0x0000FF00) << 8)  | \
              (((uint32)(x) & 0x00FF0000) >> 8)  | \
              (((uint32)(x) & 0xFF000000) >> 24)))

#define __pure          /* nothing */
#define __noreturn      /* nothing */
#define __usercall      /* nothing */
#define __userpurge     /* nothing */
#define __spoils    /* nothing */

#define __hidden        /* nothing */
#define __return_ptr    /* nothing */

#define _UNKNOWN        char

#endif // TYPES_HPP_
