/*
 * This is a refactoring of the mpfit library to remove some 
 * unsavory features or Fortran-isms in the implementation. Namely
 * 
 * 1) The Jacobian matrix is in column major order as if still copied from
 *    Fortran MINPACK. It is refactored into a linear array
 * 2) minimize or (goal) eliminate dependence on <stdlib.h>
 * 3) minimize or (goal) eliminate dependence on <string.h>
 * 4) use of static internal variables/global variables minimized or (goal) eliminated
 * 5) (goal) make optional compatibility with freestanding environments
 * 6) (goal) make typing for data sizes and data types configurable
 * 
 */

/*
 * par_index_t *:
 *  ipvt, pfixed, mpside, ddebug, ifree, qulim, qllim
 *
 */

// original mpfit header introduction string below
/* 
 * MINPACK-1 Least Squares Fitting Library
 *
 * Original public domain version by B. Garbow, K. Hillstrom, J. More'
 *   (Argonne National Laboratory, MINPACK project, March 1980)
 * 
 * Tranlation to C Language by S. Moshier (moshier.net)
 * 
 * Enhancements and packaging by C. Markwardt
 *   (comparable to IDL fitting routine MPFIT
 *    see http://cow.physics.wisc.edu/~craigm/idl/idl.html)
 */

/* Header file defining constants, data structures and functions of
   mpfit library 
   $Id$
*/

#ifndef CLMFIT_H
#define CLMFIT_H

#include <stddef.h>
#include <float.h>

#ifndef DATA_INDEX_TYPE
#define DATA_INDEX_TYPE int
#endif // DATA_INDEX_TYPE

typedef DATA_INDEX_TYPE data_index_t;

#ifndef PAR_INDEX_TYPE
#define PAR_INDEX_TYPE DATA_INDEX_TYPE
#endif // PAR_INDEX_TYPE

typedef PAR_INDEX_TYPE par_index_t;

#ifndef DATA_TYPE_MIN_SIZE
#define DATA_TYPE_MIN_SIZE 8
#endif // DATA_TYPE

// select data type based on the minimum size requested. This is currently
// hard-coded but can be modified based on architecture/floating type implementation
#if DATA_TYPE_MIN_SIZE > 8
    // TODO: should include an #error if too big for implementation
    typedef long double data_t;
    #define MP_MACHEP0 LDBL_EPSILON
    #define MP_DWARF LDBL_MIN
    #define MP_GIANT LDBL_MAX
    #include <tgmath.h>
    #define mp_sqrt(x) = sqrtl(x)
#elif DATA_TYPE_MIN_SIZE > 4
    typedef double data_t;
    #define MP_MACHEP0 DBL_EPSILON
    #define MP_DWARF DBL_MIN
    #define MP_GIANT DBL_MAX
    // requires <math.h>
    #define mp_sqrt(x) sqrt(x)
#else
    typedef float data_t;
    #define MP_MACHEP0 FLT_EPSILON
    #define MP_DWARF FLT_MIN
    #define MP_GIANT FLT_MAX
    #define mp_sqrt(x) sqrtf(x)
#endif

#define index_2D(arr, i, j, jmax) arr[(i) * (jmax) + j]

/* This is a C library.  Allow compilation with a C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif

/* modification to remove padding */
/* Definition of a parameter constraint structure */
struct mp_par_struct {
    data_t limits[2]; /* lower/upper limit boundary value */
    data_t step;      /* Step size for finite difference */
    data_t relstep;   /* Relative step size for finite difference */
    data_t deriv_reltol; /* Relative tolerance for derivative debug
                printout */
    data_t deriv_abstol; /* Absolute tolerance for derivative debug
                printout */
    char *parname;    /* Name of parameter, or 0 for none */
    par_index_t fixed;        /* 1 = fixed; 0 = free */
    int limited[2];   /* 1 = low/upper limit; 0 = no limit */
    int side;         /* Sidedness of finite difference derivative 
                    0 - one-sided derivative computed automatically
                    1 - one-sided derivative (f(x+h) - f(x)  )/h
                -1 - one-sided derivative (f(x)   - f(x-h))/h
                    2 - two-sided derivative (f(x+h) - f(x-h))/(2*h) 
                3 - user-computed analytical derivatives
                */
    int deriv_debug;  /* Derivative debug mode: 1 = Yes; 0 = No;

                        If yes, compute both analytical and numerical
                        derivatives and print them to the console for
                        comparison.

                NOTE: when debugging, do *not* set side = 3,
                but rather to the kind of numerical derivative
                you want to compare the user-analytical one to
                (0, 1, -1, or 2).
                */
};

/* Just a placeholder - do not use!! */
typedef void (*mp_iterproc)(void);

/* Definition of MPFIT configuration structure */
struct mp_config_struct {
    /* NOTE: the user may set the value explicitly; OR, if the passed
        value is zero, then the "Default" value will be substituted by
        mpfit(). */
    data_t ftol;    /* Relative chi-square convergence criterium Default: 1e-10 */
    data_t xtol;    /* Relative parameter convergence criterium  Default: 1e-10 */
    data_t gtol;    /* Orthogonality convergence criterium       Default: 1e-10 */
    data_t epsfcn;  /* Finite derivative step size               Default: MP_MACHEP0 */
    data_t stepfactor; /* Initial step bound                     Default: 100.0 */
    data_t covtol;  /* Range tolerance for covariance calculation Default: 1e-14 */
    int maxiter;    /* Maximum number of iterations.  If maxiter == MP_NO_ITER,
                        then basic error checking is done, and parameter
                        errors/covariances are estimated based on input
                        parameter values, but no fitting iterations are done. 
                Default: 200
            */
    #define MP_NO_ITER (-1) /* No iterations, just checking */
    data_index_t maxfev;     /* Maximum number of function evaluations, or 0 for no limit
                Default: 0 (no limit) */
    int nprint;     /* Default: 1 */
    int douserscale;/* Scale variables by user values?
                1 = yes, user scale values in diag;
                0 = no, variables scaled internally (Default) */
    int nofinitecheck; /* Disable check for infinite quantities from user?
                0 = do not perform check (Default)
                1 = perform check 
                */
    mp_iterproc iterproc; /* Placeholder pointer - must set to 0 */

};

/* Definition of results structure, for when fit completes */
struct mp_result_struct {
    data_t bestnorm;     /* Final chi^2 */
    data_t orignorm;     /* Starting value of chi^2 */
    data_t *resid;       /* Final residuals
                nfunc-vector, or 0 if not desired */
    data_t *xerror;      /* Final parameter uncertainties (1-sigma)
                npar-vector, or 0 if not desired */
    data_t *covar;       /* Final parameter covariance matrix
                npar x npar array, or 0 if not desired */
    data_index_t nfunc;           /* Number of residuals (= num. of data points) */
    int niter;           /* Number of iterations */
    data_index_t nfev;            /* Number of function evaluations */
    int status;          /* Fitting status code */
    par_index_t npar;            /* Total number of parameters */
    par_index_t nfree;           /* Number of free parameters */
    par_index_t npegged;         /* Number of pegged parameters */  
    char version[20];    /* CLMFIT version string */
  
};  

/* Convenience typedefs */  
typedef struct mp_par_struct mp_par;
typedef struct mp_config_struct mp_config;
typedef struct mp_result_struct mp_result;

/* Enforce type of fitting function */
typedef int (*mp_func)(data_index_t m, /* Number of functions (elts of fvec) */
		       par_index_t n, /* Number of variables (elts of x) */
		       data_t * x,      /* I - Parameters */
		       data_t * fvec,   /* O - function values */
		       data_t * dvec,  /* O - function derivatives (optional)*/
		       void * private_data); /* I/O - function private data*/

/* Error codes */
#define MP_ERR_INPUT (0)         /* General input parameter error */
#define MP_ERR_NAN (-16)         /* User function produced non-finite values */
#define MP_ERR_FUNC (-17)        /* No user function was supplied */
#define MP_ERR_NPOINTS (-18)     /* No user data points were supplied */
#define MP_ERR_NFREE (-19)       /* No free parameters */
#define MP_ERR_MEMORY (-20)      /* Memory allocation error */
#define MP_ERR_INITBOUNDS (-21)  /* Initial values inconsistent w constraints*/
#define MP_ERR_BOUNDS (-22)      /* Initial constraints inconsistent */
#define MP_ERR_PARAM (-23)       /* General input parameter error */
#define MP_ERR_DOF (-24)         /* Not enough degrees of freedom */

/* Potential success status codes */
#define MP_OK_CHI (1)            /* Convergence in chi-square value */
#define MP_OK_PAR (2)            /* Convergence in parameter value */
#define MP_OK_BOTH (3)           /* Both MP_OK_PAR and MP_OK_CHI hold */
#define MP_OK_DIR (4)            /* Convergence in orthogonality */
#define MP_MAXITER (5)           /* Maximum number of iterations reached */
#define MP_FTOL (6)              /* ftol is too small; no further improvement*/
#define MP_XTOL (7)              /* xtol is too small; no further improvement*/
#define MP_GTOL (8)              /* gtol is too small; no further improvement*/

// This should actually be a constant. You don't want to calculate const so much
//#define MP_RDWARF  (sqrt(MP_DWARF*1.5)*10)
//#define MP_RGIANT  (sqrt(MP_GIANT)*0.1)

/* External function prototype declarations */
// extern was unnecessary here
int mpfit(mp_func funct, data_index_t m, par_index_t npar, data_t *xall, 
          mp_par *pars, mp_config *config, void *private_data, 
          mp_result *result);

int mpfit_work(mp_func funct, data_index_t m, par_index_t npar,
		       data_t *xall, mp_par *pars, mp_config *config, 
		       void *private_data, mp_result *result, 
               par_index_t * index_work, data_t * data_work);

/* calculates the minimum sizes of workspace*/
void mpfit_query(data_index_t m, par_index_t npar, par_index_t nfree, 
                 size_t * n_data_t, size_t * n_data_index_t);


/* C99 uses isfinite() instead of finite() */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define mpfinite(x) isfinite(x)

/* Microsoft C uses _finite(x) instead of finite(x) */
#elif defined(_MSC_VER) && _MSC_VER
#define mpfinite(x) _finite(x)

/* Default is to assume that compiler/library has finite() function */
#else
#define mpfinite(x) finite(x)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MPFIT_H */
