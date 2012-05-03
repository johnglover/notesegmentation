import numpy as np
cimport numpy as np

np.import_array()

dtype = np.float64
ctypedef np.double_t dtype_t


cdef extern from "../src/segmentation.h":
    cdef cppclass c_GLT "GLT":
        c_Frame()
        c_Frame(int frame_size)
        double spectral_centroid(int n, double* audio)

cdef class GLT:
    cdef c_GLT* thisptr

    def __cinit__(self): self.thisptr = new c_GLT()
    def __dealloc__(self): del self.thisptr

    def spectral_centroid(self, np.ndarray[dtype_t, ndim=1] a): 
        return self.thisptr.spectral_centroid(len(a), <double*> a.data)
