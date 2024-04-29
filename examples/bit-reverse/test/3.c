#include <limits.h>

STYPE
bit_reverse(STYPE value)
{
    UTYPE nmb = 0;
    UTYPE size = CHAR_BIT * sizeof(STYPE);
    for (UTYPE i = 0; i < size; i++) {
        nmb <<= (UTYPE) 1;
        nmb += ((UTYPE) value & (1 << i)) >> i;
    }
    return (STYPE) nmb;
}
