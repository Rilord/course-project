

#include <cmath>
#include "PtexHalf.h"

PTEX_NAMESPACE_BEGIN

#include "PtexHalfTables.h"


uint16_t PtexHalf::fromFloat_except(uint32_t i)
{
    uint32_t s = ((i>>16) & 0x8000);
    int32_t e = ((i>>13) & 0x3fc00) - 0x1c000;

    if (e <= 0) {
        // denormalized
        union { uint32_t i; float f; } u;
        u.i = i;
        return (uint16_t)(s|int(fabs(u.f)*1.6777216e7 + .5));
    }

    if (e == 0x23c00)
        // inf/nan, preserve msb bits of m for nan code
        return (uint16_t)(s|0x7c00|((i&0x7fffff)>>13));
    else
        // overflow - convert to inf
        return (uint16_t)(s|0x7c00);
}

PTEX_NAMESPACE_END
