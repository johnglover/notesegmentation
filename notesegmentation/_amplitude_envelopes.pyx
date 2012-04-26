import numpy as np
cimport numpy as np

np.import_array()

ctypedef np.double_t dtype_t

cdef extern from "../src/amplitude_envelopes.h":
    double c_rms "rms" (int n, double* audio)
    double c_rms_ma "rms" (int n, double* audio, int n_prev, double* prev)

def rms(np.ndarray[dtype_t, ndim=1] a): 
    return c_rms(len(a), <double*> a.data)

def rms_ma(np.ndarray[dtype_t, ndim=1] a, np.ndarray[dtype_t, ndim=1] prev): 
    return c_rms_ma(len(a), <double*> a.data, len(prev), <double*> prev.data)
