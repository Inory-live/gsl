/* specfunc/bessel_amp_phase.h
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
#ifndef _BESSEL_AMP_PHASE_H_
#define _BESSEL_AMP_PHASE_H_

#include "gsl_sf_chebyshev.h"

extern const gsl_sf_cheb_series _gsl_sf_bessel_amp_phase_bm0_cs;
extern const gsl_sf_cheb_series _gsl_sf_bessel_amp_phase_bth0_cs;

extern const gsl_sf_cheb_series _gsl_sf_bessel_amp_phase_bm1_cs;
extern const gsl_sf_cheb_series _gsl_sf_bessel_amp_phase_bth1_cs;


/* large argument expansions [Abramowitz+Stegun, 9.2.28-29]
 *
 * thetanu_corr = thetanu - x + 1/2 nu Pi
 *
 * assumes x > 0
 */
int gsl_sf_bessel_asymp_Mnu_e(const double nu, const double x, double * result);
int gsl_sf_bessel_asymp_thetanu_corr_e(const double nu, const double x, double * result); /* w/o x term */


#endif /* !_BESSEL_AMP_PHASE_H_ */
