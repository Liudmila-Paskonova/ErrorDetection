// test 2
// 401 contest
// id = 33924
// ERROR: error in formula nmb+=...

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

/* CORRECT

#include <limits.h>

STYPE bit_reverse(STYPE value)
{
    UTYPE nmb = 0;
    for (UTYPE i = 0; i < CHAR_BIT * sizeof(value); i++) {
        nmb <<= 1;
        nmb |= (((UTYPE) value) >> i) & 1;
    }
    return (STYPE) nmb;
}

*/
