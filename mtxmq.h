/*
  MADNESS GPL (full license found in original file)
  Copyright (C) 2007,2010 Oak Ridge National Laboratory
  Copyright (C) 2014      Argonne National Laboratory
  $Id: mtxmq.h 62 2013-06-22 02:42:45Z jhammond $
*/

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef __INTEL_COMPILER
#define ASSALIGN(a) do { __assume_aligned((a),64); } while(0)
#define ASSALIGN2(a,b) do { __assume_aligned((a),64); __assume_aligned((b),64); } while(0)
#define ASSALIGN3(a,b,c) do { __assume_aligned((a),64); __assume_aligned((b),64); __assume_aligned((c),64); } while(0)
#endif

/// Matrix = Matrix transpose * matrix ... reference implementation
/// Does \c C=AT*B whereas mTxm does C=C+AT*B.  It also supposed
/// to be fast which it achieves thru restrictions
///   * All dimensions even
///   * All pointers aligned
/// \code
///    c(i,j) = sum(k) a(k,i)*b(k,j)  <------ does not accumulate into C
/// \endcode

namespace madness {
#if 1
    template <typename aT, typename bT, typename cT>
    void mTxmq(long dimi, long dimj, long dimk, cT* restrict c, const aT* restrict a, const bT* restrict b) {
        ASSALIGN3(a,b,c);
#pragma unroll_and_jam
        for (unsigned i=0; i<dimi; ++i) {
#pragma unroll_and_jam
            for (unsigned j=0; j<dimj; ++j) {
                c[i*dimj+j] = 0.0;
            }
#pragma unroll_and_jam
            for (unsigned k=0; k<dimk; ++k) {
                const aT aki = a[k*dimi+i];
// breaks and/or sucks with complex double 
//#pragma simd
#pragma vector nontemporal
#pragma unroll_and_jam
                for (long j=0; j<dimj; ++j) {
                    c[i*dimj+j] += aki*b[k*dimj+j];
                }
            }
        }
    }
#else
    template <typename aT, typename bT, typename cT>
    void mTxmq(long dimi, long dimj, long dimk, cT* restrict c, const aT* restrict a, const bT* restrict b) {
        ASSALIGN3(a,b,c);
#pragma loop_count min(1),max(1000),avg(400)
        for (unsigned i=0; i<dimi; ++i,c+=dimj,++a) {
            for (unsigned j=0; j<dimj; ++j) c[j] = 0.0;
            const aT *aik_ptr = a;
#pragma loop_count min(1),max(100),avg(20)
            for (unsigned k=0; k<dimk; ++k,aik_ptr+=dimi) {
                aT aki = *aik_ptr;
#pragma loop_count min(1),max(100),avg(20)
// breaks with complex double 
//#pragma simd
                for (long j=0; j<dimj; ++j) {
                    c[j] += aki*b[k*dimj+j];
                }
            }
        }
    }
#endif
}

