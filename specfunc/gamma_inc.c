/* specfunc/gamma_inc.c
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000 Gerard Jungman
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Author:  G. Jungman
 * RCS:     $Id$
 */
#include <config.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_errno.h>
#include "gsl_sf_erf.h"
#include "gsl_sf_exp.h"
#include "gsl_sf_log.h"
#include "gsl_sf_gamma.h"


/* The dominant part,
 * D(a,x) := x^a e^(-x) / Gamma(a+1)
 */
static
int
gamma_inc_D(const double a, const double x, gsl_sf_result * result)
{
  if(a < 10.0) {
    double lnr;
    gsl_sf_result lg;
    gsl_sf_lngamma_e(a+1.0, &lg);
    lnr = a * log(x) - x - lg.val;
    result->val = exp(lnr);
    result->err = 2.0 * GSL_DBL_EPSILON * (fabs(lnr) + 1.0) * fabs(result->val);
    return GSL_SUCCESS;
  }
  else {
    double mu = (x-a)/a;
    double term1;
    gsl_sf_result gstar;
    gsl_sf_result ln_term;
    gsl_sf_log_1plusx_mx_e(mu, &ln_term);  /* log(1+mu) - mu */
    gsl_sf_gammastar_e(a, &gstar);
    term1 = exp(a*ln_term.val)/sqrt(2.0*M_PI*a);
    result->val  = term1/gstar.val;
    result->err  = 2.0 * GSL_DBL_EPSILON * (fabs(a*ln_term.val) + 1.0) * fabs(result->val);
    result->err += gstar.err/fabs(gstar.val) * fabs(result->val);
    return GSL_SUCCESS;
  }
}


/* P series representation.
 */
static
int
gamma_inc_P_series(const double a, const double x, gsl_sf_result * result)
{
  const int nmax = 5000;

  gsl_sf_result D;
  int stat_D = gamma_inc_D(a, x, &D);

  double sum  = 1.0;
  double term = 1.0;
  int n;
  for(n=1; n<nmax; n++) {
    term *= x/(a+n);
    sum  += term;
    if(fabs(term/sum) < GSL_DBL_EPSILON) break;
  }

  result->val  = D.val * sum;
  result->err  = D.err * fabs(sum);
  result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);

  if(n == nmax)
    GSL_ERROR ("error", GSL_EMAXITER);
  else
    return stat_D;
}


/* Q large x asymptotic
 */
static
int
gamma_inc_Q_large_x(const double a, const double x, gsl_sf_result * result)
{
  const int nmax = 5000;

  gsl_sf_result D;
  const int stat_D = gamma_inc_D(a, x, &D);

  double sum  = 1.0;
  double term = 1.0;
  double last = 1.0;
  int n;
  for(n=1; n<nmax; n++) {
    term *= (a-n)/x;
    if(fabs(term/last) > 1.0) break;
    if(fabs(term/sum)  < GSL_DBL_EPSILON) break;
    sum  += term;
    last  = term;
  }

  result->val  = D.val * (a/x) * sum;
  result->err  = D.err * fabs((a/x) * sum);
  result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);

  if(n == nmax)
    GSL_ERROR ("error", GSL_EMAXITER);
  else
    return stat_D;
}


/* Uniform asymptotic for x near a, a and x large.
 * See [Temme, p. 285]
 * FIXME: need c1 coefficient
 */
static
int
gamma_inc_Q_asymp_unif(const double a, const double x, gsl_sf_result * result)
{
  const double rta = sqrt(a);
  const double eps = (x-a)/a;

  gsl_sf_result ln_term;
  const int stat_ln = gsl_sf_log_1plusx_mx_e(eps, &ln_term);  /* log(1+eps) - eps */
  const double eta  = eps * sqrt(-2.0*ln_term.val/(eps*eps));

  gsl_sf_result erfc;

  double R;
  double c0, c1;

  gsl_sf_erfc_e(eta*M_SQRT2*rta, &erfc);

  if(fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
    c0 = -1.0/3.0 + eps*(1.0/12.0 - eps*(23.0/540.0 - eps*(353.0/12960.0 - eps*589.0/30240.0)));
    c1 = 0.0;
  }
  else {
    double rt_term;
    rt_term = sqrt(-2.0 * ln_term.val/(eps*eps));
    c0 = (1.0 - 1.0/rt_term)/eps;
    c1 = 0.0;
  }

  R = exp(-0.5*a*eta*eta)/(M_SQRT2*M_SQRTPI*rta) * (c0 + c1/a);

  result->val  = 0.5 * erfc.val + R;
  result->err  = GSL_DBL_EPSILON * fabs(R * 0.5 * a*eta*eta) + 0.5 * erfc.err;
  result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);

  return stat_ln;
}


/* Continued fraction for Q.
 *
 * Q(a,x) = D(a,x) a/x F(a,x)
 *             1   (1-a)/x  1/x  (2-a)/x   2/x  (3-a)/x
 *  F(a.x) =  ---- ------- ----- -------- ----- -------- ...
 *            1 +   1 +     1 +   1 +      1 +   1 +
 *
 * Uses Gautschi equivalent series method for the CF evaluation.
 *
 * Assumes a != x + 1, so that the first term of the
 * CF recursion is not undefined. This is why we need
 * gamma_inc_Q_CF_protected() below. Based on a problem
 * report by Teemu Ikonen [Tue Oct 10 12:17:19 MDT 2000].
 */
static
int
gamma_inc_Q_CF(const double a, const double x, gsl_sf_result * result)
{
  const int kmax = 5000;

  gsl_sf_result D;
  const int stat_D = gamma_inc_D(a, x, &D);

  double sum  = 1.0;
  double tk   = 1.0;
  double rhok = 0.0;
  int k;

  for(k=1; k<kmax; k++) {
    double ak;
    if(GSL_IS_ODD(k))
      ak = (0.5*(k+1.0)-a)/x;
    else
      ak = 0.5*k/x;
    rhok  = -ak*(1.0 + rhok)/(1.0 + ak*(1.0 + rhok));
    tk   *= rhok;
    sum  += tk;
    if(fabs(tk/sum) < GSL_DBL_EPSILON) break;
  }

  result->val  = D.val * (a/x) * sum;
  result->err  = D.err * fabs((a/x) * sum);
  result->err += GSL_DBL_EPSILON * (2.0 + 0.5*k) * fabs(result->val);

  if(k == kmax)
    GSL_ERROR ("error", GSL_EMAXITER);
  else
    return stat_D;
}


/* See note above for  gamma_inc_Q_CF(). */
static
int
gamma_inc_Q_CF_protected(const double a, const double x, gsl_sf_result * result)
{
  if(fabs(1.0 - a + x) < 2.0*GSL_DBL_EPSILON) {
    /*
     * This is a problem region because when
     * 1.0 - a + x = 0 the first term of the
     * CF recursion is undefined.
     *
     * I missed this condition when I first
     * implemented gamma_inc_Q_CF() function,
     * so now I have to fix it by side-stepping
     * this point, using the recursion relation
     *   Q(a,x) = Q(a-1,x) + x^(a-1) e^(-z) / Gamma(a)
     *          = Q(a-1,x) + D(a-1,x)
     * to lower 'a' by one, giving an a=x point,
     * which is fine.
     */

    gsl_sf_result D;
    gsl_sf_result tmp_CF;

    const int stat_tmp_CF = gamma_inc_Q_CF(a-1.0, x, &tmp_CF);
    const int stat_D = gamma_inc_D(a-1.0, x, &D);
    result->val  = tmp_CF.val + D.val;
    result->err  = tmp_CF.err + D.err;
    result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
    return GSL_ERROR_SELECT_2(stat_tmp_CF, stat_D);
  }
  else {
    return gamma_inc_Q_CF(a, x, result);
  }
}


/* Useful for small a and x. Handles the subtraction analytically.
 */
static
int
gamma_inc_Q_series(const double a, const double x, gsl_sf_result * result)
{
  double term1;  /* 1 - x^a/Gamma(a+1) */
  double sum;    /* 1 + (a+1)/(a+2)(-x)/2! + (a+1)/(a+3)(-x)^2/3! + ... */
  int stat_sum;
  double term2;  /* a temporary variable used at the end */

  {
    /* Evaluate series for 1 - x^a/Gamma(a+1), small a
     */
    const double pg21 = -2.404113806319188570799476;  /* PolyGamma[2,1] */
    const double lnx  = log(x);
    const double el   = M_EULER+lnx;
    const double c1 = -el;
    const double c2 = M_PI*M_PI/12.0 - 0.5*el*el;
    const double c3 = el*(M_PI*M_PI/12.0 - el*el/6.0) + pg21/6.0;
    const double c4 = -0.04166666666666666667
                       * (-1.758243446661483480 + lnx)
                       * (-0.764428657272716373 + lnx)
		       * ( 0.723980571623507657 + lnx)
		       * ( 4.107554191916823640 + lnx);
    const double c5 = -0.0083333333333333333
                       * (-2.06563396085715900 + lnx)
		       * (-1.28459889470864700 + lnx)
		       * (-0.27583535756454143 + lnx)
		       * ( 1.33677371336239618 + lnx)
		       * ( 5.17537282427561550 + lnx);
    const double c6 = -0.0013888888888888889
                       * (-2.30814336454783200 + lnx)
                       * (-1.65846557706987300 + lnx)
                       * (-0.88768082560020400 + lnx)
                       * ( 0.17043847751371778 + lnx)
                       * ( 1.92135970115863890 + lnx)
                       * ( 6.22578557795474900 + lnx);
    const double c7 = -0.00019841269841269841
                       * (-2.5078657901291800 + lnx)
                       * (-1.9478900888958200 + lnx)
                       * (-1.3194837322612730 + lnx)
                       * (-0.5281322700249279 + lnx)
                       * ( 0.5913834939078759 + lnx)
                       * ( 2.4876819633378140 + lnx)
                       * ( 7.2648160783762400 + lnx);
    const double c8 = -0.00002480158730158730
                       * (-2.677341544966400 + lnx)
                       * (-2.182810448271700 + lnx)
                       * (-1.649350342277400 + lnx)
                       * (-1.014099048290790 + lnx)
                       * (-0.191366955370652 + lnx)
                       * ( 0.995403817918724 + lnx)
                       * ( 3.041323283529310 + lnx)
                       * ( 8.295966556941250 + lnx);
    const double c9 = -2.75573192239859e-6
                       * (-2.8243487670469080 + lnx)
                       * (-2.3798494322701120 + lnx)
                       * (-1.9143674728689960 + lnx)
                       * (-1.3814529102920370 + lnx)
                       * (-0.7294312810261694 + lnx)
                       * ( 0.1299079285269565 + lnx)
                       * ( 1.3873333251885240 + lnx)
                       * ( 3.5857258865210760 + lnx)
                       * ( 9.3214237073814600 + lnx);
    const double c10 = -2.75573192239859e-7
                       * (-2.9540329644556910 + lnx)
                       * (-2.5491366926991850 + lnx)
                       * (-2.1348279229279880 + lnx)
                       * (-1.6741881076349450 + lnx)
                       * (-1.1325949616098420 + lnx)
                       * (-0.4590034650618494 + lnx)
                       * ( 0.4399352987435699 + lnx)
                       * ( 1.7702236517651670 + lnx)
                       * ( 4.1231539047474080 + lnx)
                       * ( 10.342627908148680 + lnx);

    term1 = a*(c1+a*(c2+a*(c3+a*(c4+a*(c5+a*(c6+a*(c7+a*(c8+a*(c9+a*c10)))))))));
  }

  {
    /* Evaluate the sum.
     */
    const int nmax = 5000;
    double t = 1.0;
    int n;
    sum = 1.0;

    for(n=1; n<nmax; n++) {
      t *= -x/(n+1.0);
      sum += (a+1.0)/(a+n+1.0)*t;
      if(fabs(t/sum) < GSL_DBL_EPSILON) break;
    }
    
    if(n == nmax)
      stat_sum = GSL_EMAXITER;
    else
      stat_sum = GSL_SUCCESS;
  }

  term2 = (1.0 - term1) * a/(a+1.0) * x * sum;
  result->val  = term1 + term2;
  result->err  = GSL_DBL_EPSILON * (fabs(term1) + 2.0*fabs(term2));
  result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
  return stat_sum;
}


/*-*-*-*-*-*-*-*-*-*-*-* Functions with Error Codes *-*-*-*-*-*-*-*-*-*-*-*/

int
gsl_sf_gamma_inc_Q_e(const double a, const double x, gsl_sf_result * result)
{
  if(a <= 0.0 || x < 0.0) {
    result->val = 0.0;
    result->err = 0.0;
    GSL_ERROR ("error", GSL_EDOM);
  }
  else if(x == 0.0) {
    result->val = 1.0;
    result->err = 0.0;
    return GSL_SUCCESS;
  }
  else if(x <= 0.5*a) {
    /* If the series is quick, do that. It is
     * robust and simple.
     */
    gsl_sf_result P;
    int stat_P = gamma_inc_P_series(a, x, &P);
    result->val  = 1.0 - P.val;
    result->err  = P.err;
    result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
    return stat_P;
  }
  else if(a >= 1.0e+06 && (x-a)*(x-a) < a) {
    /* Then try the difficult asymptotic regime.
     * This is the only way to do this region.
     */
    return gamma_inc_Q_asymp_unif(a, x, result);
  }
  else if(a < 0.2 && x < 5.0) {
    /* Cancellations at small a must be handled
     * analytically; x should not be too big
     * either since the series terms grow
     * with x and log(x).
     */
    return gamma_inc_Q_series(a, x, result);
  }
  else if(a <= x) {
    if(x <= 1.0e+06) {
      /* Continued fraction is excellent for x >~ a.
       * We do not let x be too large when x > a since
       * it is somewhat pointless to try this there;
       * the function is rapidly decreasing for
       * x large and x > a, and it will just
       * underflow in that region anyway. We 
       * catch that case in the standard
       * large-x method.
       */
      return gamma_inc_Q_CF(a, x, result);
    }
    else {
      return gamma_inc_Q_large_x(a, x, result);
    }
  }
  else {
    if(a < 0.8*x) {
      /* Continued fraction again. The convergence
       * is a little slower here, but that is fine.
       * We have to trade that off against the slow
       * convergence of the series, which is the
       * only other option.
       */
      return gamma_inc_Q_CF(a, x, result);
    }
    else {
      gsl_sf_result P;
      int stat_P = gamma_inc_P_series(a, x, &P);
      result->val  = 1.0 - P.val;
      result->err  = P.err;
      result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
      return stat_P;
    }
  }
}


int
gsl_sf_gamma_inc_P_e(const double a, const double x, gsl_sf_result * result)
{
  if(a <= 0.0 || x < 0.0) {
    result->val = 0.0;
    result->err = 0.0;
    GSL_ERROR ("error", GSL_EDOM);
  }
  else if(x == 0.0) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_SUCCESS;
  }
  else if(x < 20.0 || x < 0.5*a) {
    /* Do the easy series cases. Robust and quick.
     */
    return gamma_inc_P_series(a, x, result);
  }
  else if(a > 1.0e+06 && (x-a)*(x-a) < a) {
    /* Crossover region. Note that Q and P are
     * roughly the same order of magnitude here,
     * so the subtraction is stable.
     */
    gsl_sf_result Q;
    int stat_Q = gamma_inc_Q_asymp_unif(a, x, &Q);
    result->val  = 1.0 - Q.val;
    result->err  = Q.err;
    result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
    return stat_Q;
  }
  else if(a <= x) {
    /* Q <~ P in this area, so the
     * subtractions are stable.
     */
    gsl_sf_result Q;
    int stat_Q;
    if(a > 0.2*x) {
      stat_Q = gamma_inc_Q_CF(a, x, &Q);
    }
    else {
      stat_Q = gamma_inc_Q_large_x(a, x, &Q);
    }
    result->val  = 1.0 - Q.val;
    result->err  = Q.err;
    result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
    return stat_Q;
  }
  else {
    if((x-a)*(x-a) < a) {
      /* This condition is meant to insure
       * that Q is not very close to 1,
       * so the subtraction is stable.
       */
      gsl_sf_result Q;
      int stat_Q = gamma_inc_Q_CF_protected(a, x, &Q);
      result->val  = 1.0 - Q.val;
      result->err  = Q.err;
      result->err += 2.0 * GSL_DBL_EPSILON * fabs(result->val);
      return stat_Q;
    }
    else {
      return gamma_inc_P_series(a, x, result);
    }
  }
}


/*-*-*-*-*-*-*-*-*-* Functions w/ Natural Prototypes *-*-*-*-*-*-*-*-*-*-*/

#include "eval.h"

double gsl_sf_gamma_inc_P(const double a, const double x)
{
  EVAL_RESULT(gsl_sf_gamma_inc_P_e(a, x, &result));
}

double gsl_sf_gamma_inc_Q(const double a, const double x)
{
  EVAL_RESULT(gsl_sf_gamma_inc_Q_e(a, x, &result));
}
