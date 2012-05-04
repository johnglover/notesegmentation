import numpy as np
cimport numpy as np

np.import_array()

ctypedef np.double_t dtype_t

cdef extern from "../src/util.h" namespace "util":
    void cma "util::cumulative_moving_average_frame" (int n, double* audio, double* cma,
                                                      int n_prev, double prev)

def cumulative_moving_average(np.ndarray[dtype_t, ndim=1] a, n_prev=0, prev=0.0): 
    cdef np.ndarray result = np.zeros(len(a), dtype=np.double)
    cma(len(a), <double*> a.data, <double*> result.data, n_prev, prev)
    return result
