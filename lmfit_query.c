#include <stdlib.h>
#include <stdio.h>
#include "lmfit.h"

int main(int narg, char ** args) {
    int ndbl, nint, m, npar, nfree;
    if (narg < 4) {
        return 0;
    }

    m = atoi(args[1]);
    npar = atoi(args[2]);
    nfree = atoi(args[3]);
    if (m <= 0) {
        return 1;
    }
    if (npar <= 0) {
        return 2;
    }
    if (nfree <= 0 || nfree > npar) {
        return 3;
    }

    mpfit_query(m, npar, nfree, &ndbl, &nint);

    printf("# doubles = %d, # ints = %d\n", ndbl, nint);

    return 0;
}