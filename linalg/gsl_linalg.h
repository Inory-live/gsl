/* linalg/gsl_linalg.h
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000 Gerard Jungman, Brian Gough
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
#ifndef __GSL_LINALG_H__
#define __GSL_LINALG_H__

#include <gsl/gsl_permutation.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS		/* empty */
#define __END_DECLS		/* empty */
#endif

__BEGIN_DECLS

typedef enum
  {
    GSL_LINALG_MOD_NONE = 0,
    GSL_LINALG_MOD_TRANSPOSE = 1,
    GSL_LINALG_MOD_CONJUGATE = 2
  }
gsl_linalg_matrix_mod_t;


/* Note: You can now use the gsl_blas_dgemm function instead of matmult */

/* Simple implementation of matrix multiply.
 * Calculates C = A.B
 *
 * exceptions: GSL_EBADLEN
 */
int gsl_linalg_matmult (const gsl_matrix * A,
			const gsl_matrix * B,
			gsl_matrix * C);


/* Simple implementation of matrix multiply.
 * Allows transposition of either matrix, so it
 * can compute A.B or Trans(A).B or A.Trans(B) or Trans(A).Trans(B)
 *
 * exceptions: GSL_EBADLEN
 */
int gsl_linalg_matmult_mod (const gsl_matrix * A,
			    gsl_linalg_matrix_mod_t modA,
			    const gsl_matrix * B,
			    gsl_linalg_matrix_mod_t modB,
			    gsl_matrix * C);


/* Householder Transformations */

double gsl_linalg_householder_transform (gsl_vector * v);

int gsl_linalg_householder_hm (double tau, 
                               const gsl_vector * v, 
                               gsl_matrix * A, 
                               gsl_vector * work);

int gsl_linalg_householder_hv (double tau, 
                               const gsl_vector * v, 
                               gsl_vector * w);

/* Singular Value Decomposition

 * exceptions: 
 */
int gsl_linalg_SV_decomp (gsl_matrix * A,
			  gsl_matrix * Q,
			  gsl_vector * S);

int
gsl_linalg_SV_solve (const gsl_matrix * U,
                     const gsl_matrix * Q,
                     const gsl_vector * S,
                     const gsl_vector * b,
                     gsl_vector * x);


/* LU Decomposition, Gaussian elimination with partial pivoting
 */

int gsl_linalg_LU_decomp (gsl_matrix * A, gsl_permutation * p, int *signum);

int gsl_linalg_LU_solve (const gsl_matrix * LU,
			 const gsl_permutation * p,
			 const gsl_vector * b,
                         gsl_vector * x);

int gsl_linalg_LU_svx (const gsl_matrix * LU,
                       const gsl_permutation * p,
                       gsl_vector * x);

int gsl_linalg_LU_refine (const gsl_matrix * A,
			  const gsl_matrix * LU,
			  const gsl_permutation * p,
			  const gsl_vector * b,
			  gsl_vector * x,
			  gsl_vector * residual);

int gsl_linalg_LU_invert (const gsl_matrix * LU,
			  const gsl_permutation * p,
			  gsl_matrix * inverse);

double gsl_linalg_LU_det (gsl_matrix * LU, int signum);
double gsl_linalg_LU_lndet (gsl_matrix * LU);
int gsl_linalg_LU_sgndet (gsl_matrix * lu, int signum);

/* QR decomposition */

int gsl_linalg_QR_decomp (gsl_matrix * A,
			  gsl_vector * tau);

int gsl_linalg_QR_solve (const gsl_matrix * QR,
			 const gsl_vector * tau,
			 const gsl_vector * b,
			 gsl_vector * x);

int gsl_linalg_QR_svx (const gsl_matrix * QR,
                       const gsl_vector * tau,
                       gsl_vector * x);

int gsl_linalg_QR_QRsolve (gsl_matrix * Q,
			   gsl_matrix * R,
			   const gsl_vector * b,
			   gsl_vector * x);

int gsl_linalg_QR_Rsolve (const gsl_matrix * QR,
                          const gsl_vector * b,
			  gsl_vector * x);

int gsl_linalg_QR_Rsvx (const gsl_matrix * QR,
                        gsl_vector * x);

int gsl_linalg_QR_update (gsl_matrix * Q,
			  gsl_matrix * R,
			  gsl_vector * w,
			  const gsl_vector * v);

int gsl_linalg_QR_QTvec (const gsl_matrix * QR,
			 const gsl_vector * tau,
			 gsl_vector * v);

int gsl_linalg_QR_unpack (const gsl_matrix * QR,
			  const gsl_vector * tau,
			  gsl_matrix * Q,
			  gsl_matrix * R);

int gsl_linalg_R_solve (const gsl_matrix * R,
                        const gsl_vector * b,
                        gsl_vector * x);

int gsl_linalg_R_svx (const gsl_matrix * R,
                      gsl_vector * x);


/* Q R P^T decomposition */

int gsl_linalg_QRPT_decomp (gsl_matrix * A,
			    gsl_vector * tau,
			    gsl_permutation * p,
			    int *signum);

int gsl_linalg_QRPT_decomp2 (const gsl_matrix * A, 
                             gsl_matrix * q, gsl_matrix * r, 
                             gsl_vector * tau, 
                             gsl_permutation * p, 
                             int *signum);

int gsl_linalg_QRPT_solve (const gsl_matrix * QR,
			   const gsl_vector * tau,
			   const gsl_permutation * p,
                           const gsl_vector * b,
			   gsl_vector * x);


int gsl_linalg_QRPT_svx (const gsl_matrix * QR,
                         const gsl_vector * tau,
                         const gsl_permutation * p,
                         gsl_vector * x);

int gsl_linalg_QRPT_QRsolve (const gsl_matrix * Q,
			     const gsl_matrix * R,
			     const gsl_permutation * p,
			     const gsl_vector * b,
			     gsl_vector * x);

int gsl_linalg_QRPT_Rsolve (const gsl_matrix * QR,
                             const gsl_permutation * p,
			     const gsl_vector * b,
                             gsl_vector * x);

int gsl_linalg_QRPT_Rsvx (const gsl_matrix * QR,
                           const gsl_permutation * p,
                           gsl_vector * x);

int gsl_linalg_QRPT_update (gsl_matrix * Q,
			    gsl_matrix * R,
			    const gsl_permutation * p,
			    gsl_vector * u,
			    const gsl_vector * v);

/* Cholesky Decomposition */

int gsl_linalg_cholesky_decomp (gsl_matrix * A);

int gsl_linalg_cholesky_solve (const gsl_matrix * cholesky,
                               const gsl_vector * b,
                               gsl_vector * x);

int gsl_linalg_cholesky_svx (const gsl_matrix * cholesky,
                             gsl_vector * x);

/* Linear Solve Using Householder Transformations

 * exceptions: 
 */

int gsl_linalg_HH_solve (gsl_matrix * A, const gsl_vector * b, gsl_vector * x);
int gsl_linalg_HH_svx (gsl_matrix * A, gsl_vector * x);

/* Linear solve for a symmetric tridiagonal system.

 * The input vectors represent the NxN matrix as follows:
 *
 *     diag[0]  offdiag[0]             0    ...
 *  offdiag[0]     diag[1]    offdiag[1]    ...
 *           0  offdiag[1]       diag[2]    ...
 *           0           0    offdiag[2]    ...
 *         ...         ...           ...    ...
 */
int gsl_linalg_solve_symm_tridiag (const gsl_vector * diag,
				   const gsl_vector * offdiag,
				   const gsl_vector * b,
				   gsl_vector * x);


/* Linear solve for a symmetric cyclic tridiagonal system.

 * The input vectors represent the NxN matrix as follows:
 *
 *      diag[0]  offdiag[0]             0   .....  offdiag[N-1]
 *   offdiag[0]     diag[1]    offdiag[1]   .....
 *            0  offdiag[1]       diag[2]   .....
 *            0           0    offdiag[2]   .....
 *          ...         ...
 * offdiag[N-1]         ...
 */
int gsl_linalg_solve_symm_cyc_tridiag (const gsl_vector * diag,
				       const gsl_vector * offdiag,
				       const gsl_vector * b,
				       gsl_vector * x);


__END_DECLS

#endif /* __GSL_LINALG_H__ */
