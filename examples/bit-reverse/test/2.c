// test 0
// 501 contest
// id = 34682
// ERROR: result is declared as STYPE -> <<= causes UB
#include <stdio.h>

STYPE
bit_reverse(STYPE value)
{
    UTYPE abs_val = value;
    UTYPE flag = 0;
    flag -= 1;
    STYPE result = 0;
    while (flag) {
        flag >>= 1;
        result <<= 1;
        result |= abs_val & 1;
        abs_val >>= 1;
    }
    return result;
}

/* CORRECT:
#include <stdio.h>

STYPE
bit_reverse(STYPE value)
{
    UTYPE abs_val = value;
    UTYPE flag = 0;
    flag -= 1;
    UTYPE result = 0;
    while (flag) {
        flag >>= 1;
        result <<= 1;
        result |= abs_val & 1;
        abs_val >>= 1;
    }
    return (STYPE) result;
}
*/
