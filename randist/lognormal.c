#include <math.h>
#include <gsl_rng.h>
#include <gsl_randist.h>

/* Lognormal random numbers are the exponentials of gaussian random numbers */

double
gsl_ran_lognormal (const gsl_rng * r)
{
  double u, v, r2, normal, z;

  do
    {
      /* choose x,y in uniform square (-1,-1) to (+1,+1) */

      u = -1 + 2 * gsl_rng_uniform (r);
      v = -1 + 2 * gsl_rng_uniform (r);

      /* see if it is in the unit circle */
      r2 = u * u + v * v;
    }
  while (r2 > 1.0 || r2 == 0);

  normal = u * sqrt (-2.0 * log (r2) / r2) ;

  z = exp(normal) ;

  return z ;
}
