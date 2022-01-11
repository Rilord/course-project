#ifndef PtexHalf_h
#define PtexHalf_h





#ifndef PTEXAPI
#  if defined(_WIN32) || defined(_WINDOWS) || defined(_MSC_VER)
#    ifndef PTEX_STATIC
#      ifdef PTEX_EXPORTS
#         define PTEXAPI __declspec(dllexport)
#      else
#         define PTEXAPI __declspec(dllimport)
#      endif
#    else
#      define PTEXAPI
#    endif
#  else
#    define PTEXAPI
#  endif
#endif

#include "PtexInt.h"

#include "PtexVersion.h"

PTEX_NAMESPACE_BEGIN



struct PtexHalf {
    uint16_t bits;

    /// Default constructor, value is undefined
    PtexHalf() {}
    PtexHalf(float val) : bits(fromFloat(val)) {}
    PtexHalf(double val) : bits(fromFloat(float(val))) {}
    operator float() const { return toFloat(bits); }
    PtexHalf& operator=(float val) { bits = fromFloat(val); return *this; }

    static float toFloat(uint16_t h)
    {
        union { uint32_t i; float f; } u;
        u.i = h2fTable[h];
        return u.f;
    }

    static uint16_t fromFloat(float val)
    {
        if (val==0) return 0;
        union { uint32_t i; float f; } u;
        u.f = val;
        int e = f2hTable[(u.i>>23)&0x1ff];
        if (e) return (uint16_t)(e + (((u.i&0x7fffff) + 0x1000) >> 13));
        return fromFloat_except(u.i);
    }

 private:
    PTEXAPI static uint16_t fromFloat_except(uint32_t val);
#ifndef DOXYGEN
     public:
#endif
    PTEXAPI static uint32_t h2fTable[65536];
    PTEXAPI static uint16_t f2hTable[512];
};

PTEX_NAMESPACE_END

using Ptex::PtexHalf;

#endif
