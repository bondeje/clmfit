This is a refactoring of the source code for [mpfit v1.5](https://pages.physics.wisc.edu/~craigm/idl/cmpfit.html)

Original code for comparison and to fulfill license/DISCLAIMER requirements of source located in `cmpfit/` folder

Changes made:
1) Indentations realigned and compound statement blocks added to be consistent
   - Justification: The mpfit.c file was unnecessarily difficult to read
2) Jacobian derivative vector is changed from column-major `double **` to row major `double *`.
   - Justification: This makes memory access consistent with the memory layout (row-major) in C as opposed to Fortran (column-major).
     This might allow larger datasets to get the advantage of localization/caching and reduces number of memory allocations.
     The original was only allocating "number of parameters" extra pointers to doubles so memory was not really an issue, but there
     was already a memory allocation for the jacobian: `double * fjac`. The `double ** dvec` was an extra allocation presumably done
     as syntactic sugar so it could be indexed like a 2D array, but they kept Fortran column-major orientation. This also makes the
     interface more in line with other similar fortran translations, e.g. LINPACK
   - TODO 1: I would prefer to do a pointer to array `double * [n_parameters]`, which would allow 2D array indexing while still
     maintaining minimum, linear memory, but MSVC in a target environment I made this for does not allow Variably Modified Types from
     C89/C99 and C11 they are optional. This is just going to be a preprocessor check to see if VMT is available. The availability of
     VMTs is guaranteed in C23 but who knows when MSVC will be compliant.
   - TODO 2: I'm still checking the math to see how many more column-major/row-major potential issues can safely be resolved. To
     keep impact minimal, I re-use the `fjac` variable allocation, but transpose back to column-major after it is passed to
     user-supplied function. Despite the added transpose and still using column-major calculations downstream, I'm seeing a 5-10%
     speedup in very small datasets (O(100 x 5))
3) WIP: adding interface to allow caller to supply workspace
   - Justification: This would make the interface more in line with e.g. LINPACK where user provides extra workspace memory
     allocations to do computations. It would also allow the library to exist in an environment where `<stdlib.h>` is not available
4) WIP: Reduce number of memory allocations.
   - Justification: reduce syscalls. Every time `mpfit(...)` from `cmpfit` is run, it performs 23 `malloc(...)` calls, but after the
     `dvec` one above is removed, there are only arrays of 2 different data types. This can be reduced to 2 calls
5) Allow configurable compilation for the different floating types: `float`, `double`, or `long double`
   - Justification: allow users who have more or less constrained memory needs to use library
6) Allow configurable index types for both parameter arrays and data array indices
   - Justification: `mpfit` uses `int` for both parameter and data array index types, but typically we have number of parameters <<
     number of data points. It is a micro-optimization to allow for different types to potentially trade-off memory. The bigger option
     this provides is using unsigned types. There was only a single line of code that used a negative value for either of these types
     and it was for a use case where the negative value was not used for an index but rather a flag. This was broken off and now it
     is safe to use unsigned types for these values, default is still `int`.
7) Eliminate/minimize use of static function variables
   - Justification: There's nothing really wrong with the use of static function variables in general, but the way `mpfit` was using
     them was kind of like global variables with restricted scope, except the values just end up being constants. They have been
     replaced by `#define`s. I have been burned before by Fortran's equivalent of static variables, which make function calls that
     one might rightfully expect to be idempotent be not idempotent (I'm looking at you FITPACK). That's what this looked like, so I
     wanted to get rid of it.

Wishlist:
1) Make compatible with freestanding implementations
   - It would be nice to have an option for a sophisticated optimizer available in a freestanding environment. It would not take much. The dependencies that are not fully freestanding conformant are:
     - `<math.h>` - `fabs`/`fabsf`/`fabsl`, `isfinite`, `sqrtf`/`sqrt`/`sqrtl` (the ****f, ****l additions being because of my inclusion of `<tgmath.h>` for (5) above.)
     - `<stdlib.h>` - `malloc`, `free`
     - `<stdio.h>` - the many arguably unnecessary uses of `printf`
2) Make data type-generic
   - Justification: don't want 3+ compilations to do `float`, `double`, `long double`, etc. 
   - Instead of re-compiling the library for each data type, it would be nice to just make it type generic for the different floating types
