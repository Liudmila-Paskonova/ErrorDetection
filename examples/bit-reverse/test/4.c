// test 3
// 501 contest
// id = 34684
// ERROR: ...

STYPE
bit_reverse(STYPE value)
{
    UTYPE tmp = ~0;
    int cnt = 0, left_bit = 0, right_bit = 0;
    while (tmp) {
        tmp >>= 1;
        cnt++;
    }
    for (int i = 0; i < cnt >> 1; i++) {
        tmp = 1;
        left_bit = (tmp << (cnt - 1 - i)) & value;
        right_bit = (tmp << i) & value;
        if ((left_bit && !right_bit) || (!left_bit && right_bit)) {
            value = value ^ ((tmp << (cnt - 1 - i)) | (tmp << i));
        }
    }
    return value;
}

/*
CORRECT

STYPE
bit_reverse(STYPE value)
{
    UTYPE abs_val = value;
    UTYPE flag = -1;
    UTYPE abs_res = 0;
    while (flag) {
        flag >>= 1;
        abs_res <<= 1;
        abs_res |= abs_val & 1;
        abs_val >>= 1;
    }
    return (STYPE) abs_res;
}
*/
