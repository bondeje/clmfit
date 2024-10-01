#include <stdio.h>
#include <math.h>

#ifdef TIMEIT
#ifdef _WIN32
#include <windows.h>
LONGLONG time_ = 0;
LARGE_INTEGER li = {0};
LARGE_INTEGER clock_freq = {0};
void mark_start(void) {
    QueryPerformanceFrequency(&clock_freq);
    QueryPerformanceCounter(&li);
    time_ = li.QuadPart;
}
void mark_end(void) {
    QueryPerformanceCounter(&li);
    time_ = (LONGLONG) ((li.QuadPart - time_) * 1.0e9 / clock_freq.QuadPart);
}
#else
// posix
#include <time.h>
long long time_ = 0;
struct timespec tspec = {0};
clockid_t clk = CLOCK_MONOTONIC;
void mark_start(void) {
    clock_gettime(clk, &tspec);
    time_ = tspec.tv_nsec;
}
void mark_end(void) {
    clock_gettime(clk, &tspec);
    time_ = tspec.tv_nsec - time_;
}
#endif // POSIX
long long get_execution_time(void) {
    return (long long) time_;
}
#else
long long get_execution_time(void) {
    return -1;
}
#endif // TIMEIT


#include "lmfit.h"

#define N (100)
#define NPAR (5)
#define X_START (-5.0)
#define X_END (5.0)
#define DX ((X_END - X_START) / (N - 1.0))

struct xy {
    double * x;
    double * y;
};

void gaussian(double x, double * pars, double * out) {
    double z = (x - pars[0]) / pars[1];
    *out = pars[4] + pars[3] * z + pars[2] * exp(-0.5 * z * z);
}

void gaussianv(int n, double * x, double * pars, double * out) {
    while (n--) {
        double z = (x[n] - pars[0]) / pars[1];
        out[n] = pars[4] + pars[3] * z + pars[2] * exp(-0.5 * z * z);
    }
}

void dgaussian(double x, double * pars, double * dvec) {
    if (!dvec) {
        return;
    }
    double z = (x - pars[0]) / pars[1];
    double z2 = z * z;
    double expz2 = exp(-0.5 * z2);
    dvec[0] = pars[2] * z / pars[1] * expz2;
    dvec[1] = pars[2] * z2 / pars[1] * expz2;
    dvec[2] = expz2;
    dvec[3] = z;
    dvec[4] = 1.0;
}

void dgaussianv(int n, double * x, double * pars, double * dvec) {
    if (!dvec) {
        return;
    }
    while (n--) {
        double z = (x[n] - pars[0]) / pars[1];
        double z2 = z * z;
        double expz2 = exp(-0.5 * z2);
        index_2D(dvec, n, 0, 5) = pars[2] * z / pars[1] * expz2;
        index_2D(dvec, n, 1, 5) = pars[2] * z2 / pars[1] * expz2;
        index_2D(dvec, n, 2, 5) = expz2;
        index_2D(dvec, n, 3, 5) = z;
        index_2D(dvec, n, 4, 5) = 1.0;
    }
}

int gaussian_cost(int m, /* Number of functions (elts of fvec) */
		       int n, /* Number of variables (elts of pars) */
		       double * pars,      /* I - Parameters */
		       double * fvec,   /* O - function values */
		       double * dvec,  /* O - function derivatives (optional)*/
		       void * data) {
    double * x = ((struct xy *)data)->x;
    double * y = ((struct xy *)data)->y;
    double ym = 0.0;
    if (dvec) {
        while (m--) {
            gaussian(x[m], pars, fvec + m);
            fvec[m] = y[m] - fvec[m];
            dgaussian(x[m], pars, dvec + m * n);
        }
    } else {
        while (m--) {
            gaussian(x[m], pars, &ym);
            fvec[m] = (y[m] - ym);
        }
    }  
    
    return 0;
}

int gaussianv_cost(int m, /* Number of functions (elts of fvec) */
		       int n, /* Number of variables (elts of pars) */
		       double * pars,      /* I - Parameters */
		       double * fvec,   /* O - function values */
		       double * dvec,  /* O - function derivatives (optional)*/
		       void * data) {
    double * x = ((struct xy *)data)->x;
    double * y = ((struct xy *)data)->y;
    if (dvec) {
        dgaussianv(m, x, pars, dvec);
    }  
    gaussianv(m, x, pars, fvec);
    while (m--) {
        fvec[m] = y[m] - fvec[m];
    }

    return 0;
}

int main(void) {

    double x[N];
    double y[N];
    double pars_in[NPAR] = {-2.0, 1.5, 2.0, 0.025, -0.3};
    double pars_guess[NPAR] = {-1.0, 1.25, 3.0, 0.005, 0.3};
    double dx = ((X_END - X_START) / (N - 1.0));
    int i = 0;
    x[i] = X_START;
    gaussian(x[i], pars_in, y + i);
    for (i = 1; i < N; i++) {
        x[i] = x[i - 1] + dx;
        gaussian(x[i], pars_in, y + i);
    }

    struct xy data = {&x[0], &y[0]};
    mp_result results = {0};
    mp_config config = {0};
    config.maxiter = 1000;
    int status = 0;

    printf("before:\n\tcenter: %f\n\twidth: %f\n\tamplitude: %f\n\tslope: %f\n\toffset: %f\n",
        pars_guess[0],
        pars_guess[1],
        pars_guess[2],
        pars_guess[3],
        pars_guess[4]);

#ifdef TIMEIT
    mark_start();
#endif
    status = mpfit(gaussian_cost, N, NPAR, pars_guess, NULL, &config, &data, &results);
#ifdef TIMEIT
    mark_end();
#endif
    printf("status = %d\n", status);
    printf("results:\n\tbestnorm: %f\n\torignorm: %f\n\tniter: %d\n\tnfev: %d\n\tstatus: %d\n\tnpar: %d\n\tnfree: %d\n\tnpegged: %d\n\tnfunc: %d\n\texec_time(ns): %lld\n",
                        results.bestnorm,
                        results.orignorm,
                        results.niter,
                        results.nfev,
                        results.status,
                        results.npar,
                        results.nfree,
                        results.npegged,
                        results.nfunc,
                        get_execution_time());
    printf("final:\n\tcenter: %f\n\twidth: %f\n\tamplitude: %f\n\tslope: %f\n\toffset: %f\n",
        pars_guess[0],
        pars_guess[1],
        pars_guess[2],
        pars_guess[3],
        pars_guess[4]);

    printf("expected:\n\tcenter: %f\n\twidth: %f\n\tamplitude: %f\n\tslope: %f\n\toffset: %f\n",
        pars_in[0],
        pars_in[1],
        pars_in[2],
        pars_in[3],
        pars_in[4]);

    pars_guess[0] = -1.0;
    pars_guess[1] = 1.25;
    pars_guess[2] = 3.0;
    pars_guess[3] = 0.005;
    pars_guess[4] = 0.3;

    printf("before:\n\tcenter: %f\n\twidth: %f\n\tamplitude: %f\n\tslope: %f\n\toffset: %f\n",
        pars_guess[0],
        pars_guess[1],
        pars_guess[2],
        pars_guess[3],
        pars_guess[4]);

#ifdef TIMEIT
    mark_start();
#endif
    status = mpfit(gaussianv_cost, N, NPAR, pars_guess, NULL, &config, &data, &results);
#ifdef TIMEIT
    mark_end();
#endif
    printf("status = %d\n", status);
    printf("results:\n\tbestnorm: %f\n\torignorm: %f\n\tniter: %d\n\tnfev: %d\n\tstatus: %d\n\tnpar: %d\n\tnfree: %d\n\tnpegged: %d\n\tnfunc: %d\n\texec_time(ns): %lld\n",
                        results.bestnorm,
                        results.orignorm,
                        results.niter,
                        results.nfev,
                        results.status,
                        results.npar,
                        results.nfree,
                        results.npegged,
                        results.nfunc,
                        get_execution_time());
    printf("final:\n\tcenter: %f\n\twidth: %f\n\tamplitude: %f\n\tslope: %f\n\toffset: %f\n",
        pars_guess[0],
        pars_guess[1],
        pars_guess[2],
        pars_guess[3],
        pars_guess[4]);

    printf("expected:\n\tcenter: %f\n\twidth: %f\n\tamplitude: %f\n\tslope: %f\n\toffset: %f\n",
        pars_in[0],
        pars_in[1],
        pars_in[2],
        pars_in[3],
        pars_in[4]);

    return 0;
}