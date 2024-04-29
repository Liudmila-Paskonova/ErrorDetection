#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

STYPE
bit_reverse(STYPE value)
{
    UTYPE answer = 0;
    UTYPE v = value;
    for (int i = 0; i < sizeof(STYPE) * CHAR_BIT; ++i) {
        answer |= (v & 1) << (31 - i);
        v >>= 1;
    }
    value = answer;
    return value;
}
