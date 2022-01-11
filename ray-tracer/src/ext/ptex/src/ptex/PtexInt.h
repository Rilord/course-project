#ifndef PtexInt_h
#define PtexInt_h





#if defined(_WIN32) || defined(_WINDOWS) || defined(_MSC_VER)

#if defined(_MSC_VER) && _MSC_VER >= 1600
#include <stdint.h>
#else
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#endif

#else
#include <stdint.h>
#endif

#endif
