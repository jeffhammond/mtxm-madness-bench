/*
  This file is part of MADNESS.

  Copyright (C) 2007,2010 Oak Ridge National Laboratory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  For more information please contact:

  Robert J. Harrison
  email: rjharrison@gmail.com

  $Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex>

#include <omp.h>

#include <cblas.h>
#include <mtxmq.h>

#define TIME_DGEMM
#define ALIGNMENT 64

typedef std::complex<double> double_complex;

using namespace madness;

#ifdef TIME_DGEMM
void mTxm_zgemm(long ni, long nj, long nk, double_complex* c, const double_complex* a, const double_complex*b ) {
  double_complex one=1.0;
  madness::cblas::gemm(madness::cblas::NoTrans,madness::cblas::Trans,nj,ni,nk,one,b,nj,a,ni,one,c,nj);
}
#endif

double_complex ran()
{
  static unsigned long seed = 76521;

  seed = seed*1812433253 + 12345;
  double d1 = double(seed & 0x7fffffff)*4.6566128752458e-10;
  seed = seed*1812433253 + 12345;
  double d2 = double(seed & 0x7fffffff)*4.6566128752458e-10;

  return double_complex(d1,d2);
}

void ran_fill(int n, double_complex *a) {
    while (n--) *a++ = ran();
}

void mTxm(long dimi, long dimj, long dimk,
          double_complex* c, const double_complex* a, const double_complex* b) {
    int i, j, k;
    for (k=0; k<dimk; ++k) {
        for (j=0; j<dimj; ++j) {
            for (i=0; i<dimi; ++i) {
                c[i*dimj+j] += a[k*dimi+i]*b[k*dimj+j];
            }
        }
    }
}

void crap(double rate, double fastest, double start) {
    if (rate == 0) printf("darn compiler bug %e %e %lf\n",rate,fastest,start);
}


void timer(const char* s, long ni, long nj, long nk, double_complex *a, double_complex *b, double_complex *c) {

  double fastest=0.0, fastest_zgemm=-1.0;
  double nflop = 2.0*ni*nj*nk;
  long loop;
  for (loop=0; loop<30; ++loop) {
    double rate;
    double start = omp_get_wtime();
    mTxmq(ni,nj,nk,c,a,b);
    start = omp_get_wtime() - start;
    rate = 1.e-9*nflop/start;
    crap(rate,fastest,start);
    if (rate > fastest) fastest = rate;
  }
#ifdef TIME_DGEMM
  for (loop=0; loop<30; ++loop) {
    double rate;
    double start = omp_get_wtime();
    mTxm_zgemm(ni,nj,nk,c,a,b);
    start = omp_get_wtime() - start;
    rate = 1.e-9*nflop/start;
    crap(rate,fastest_zgemm,start);
    if (rate > fastest_zgemm) fastest_zgemm = rate;
  }
#endif
  printf("%20s %3ld %3ld %3ld %8.2f %8.2f\n",s, ni,nj,nk, fastest, fastest_zgemm);
}

void trantimer(const char* s, long ni, long nj, long nk, double_complex *a, double_complex *b, double_complex *c) {
  double fastest=0.0, fastest_zgemm=-1.0;
  double nflop = 3.0*2.0*ni*nj*nk;
  long loop;
  for (loop=0; loop<30; ++loop) {
    double rate;
    double start = omp_get_wtime();
    mTxmq(ni,nj,nk,c,a,b);
    mTxmq(ni,nj,nk,a,c,b);
    mTxmq(ni,nj,nk,c,a,b);
    start = omp_get_wtime() - start;
    rate = 1.e-9*nflop/start;
    crap(rate,fastest,start);
    if (rate > fastest) fastest = rate;
  }
#ifdef TIME_DGEMM
  for (loop=0; loop<30; ++loop) {
    double rate;
    double start = omp_get_wtime();
    mTxm_zgemm(ni,nj,nk,c,a,b);
    mTxm_zgemm(ni,nj,nk,a,c,b);
    mTxm_zgemm(ni,nj,nk,c,a,b);
    start = omp_get_wtime() - start;
    rate = 1.e-9*nflop/start;
    crap(rate,fastest_zgemm,start);
    if (rate > fastest_zgemm) fastest_zgemm = rate;
  }
#endif
  printf("%20s %3ld %3ld %3ld %8.2f %8.2f\n",s, ni,nj,nk, fastest, fastest_zgemm);
}

int main(int argc, char * argv[]) {
    const long nimax=30*30;
    const long njmax=100;
    const long nkmax=100;
    long ni, nj, nk, i, m;
    double_complex *a, *b, *c, *d;

    printf("Running MADNESS mTxm test \n");

    posix_memalign((void **) &a, ALIGNMENT, nkmax*nimax*sizeof(double_complex));
    posix_memalign((void **) &b, ALIGNMENT, nkmax*njmax*sizeof(double_complex));
    posix_memalign((void **) &c, ALIGNMENT, nimax*njmax*sizeof(double_complex));
    posix_memalign((void **) &d, ALIGNMENT, nimax*njmax*sizeof(double_complex));

    ran_fill(nkmax*nimax, a);
    ran_fill(nkmax*njmax, b);


/*     ni = nj = nk = 2; */
/*     for (i=0; i<ni*nj; ++i) d[i] = c[i] = 0.0; */
/*     mTxm (ni,nj,nk,c,a,b); */
/*     mTxmq(ni,nj,nk,d,a,b); */
/*     for (i=0; i<ni; ++i) { */
/*       long j; */
/*       for (j=0; j<nj; ++j) { */
/* 	printf("%2ld %2ld %.6f %.6f\n", i, j, c[i*nj+j], d[i*nj+j]); */
/*       } */
/*     } */
/*     return 0; */

    printf("Starting to test ... \n");
    for (ni=1; ni<12; ni+=1) {
        for (nj=1; nj<12; nj+=1) {
            for (nk=1; nk<12; nk+=1) {
                for (i=0; i<ni*nj; ++i) d[i] = c[i] = 0.0;
                mTxm (ni,nj,nk,c,a,b);
                mTxmq(ni,nj,nk,d,a,b);
                for (i=0; i<ni*nj; ++i) {
                    double err = std::abs(d[i]-c[i]);
                    /* This test is sensitive to the compilation options.
                       Be sure to have the reference code above compiled
                       -msse2 -fpmath=sse if using GCC.  Otherwise, to
                       pass the test you may need to change the threshold
                       to circa 1e-13.
                    */
                    if (err > 2e-14) {
                        printf("test_mtxmq: error %ld %ld %ld %e\n",ni,nj,nk,err);
                        exit(1);
                    }
                }
            }
        }
    }
    printf("... OK!\n");

    printf("%20s %3s %3s %3s %8s %8s (GF/s)\n", "type", "M", "N", "K", "LOOP", "BLAS");
    for (ni=2; ni<60; ni+=2) timer("(m*m)T*(m*m)", ni,ni,ni,a,b,c);
    for (m=1; m<=30; m+=1) timer("(m*m,m)T*(m*m)", m*m,m,m,a,b,c);
    for (m=1; m<=30; m+=1) trantimer("tran(m,m,m)", m*m,m,m,a,b,c);
    for (m=1; m<=20; m+=1) timer("(20*20,20)T*(20,m)", 20*20,m,20,a,b,c);

    return 0;
}
