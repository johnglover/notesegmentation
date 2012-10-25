import numpy as np
cimport numpy as np

np.import_array()

dtype = np.float64
ctypedef np.double_t dtype_t


cdef extern from "../src/segmentation.h":
    cdef cppclass c_RTSegmentation "RTSegmentation":
        c_Frame()
        c_Frame(int frame_size)
        double spectral_centroid(int n, double* audio)
        int segment(int n, double* audio)


cdef class RTSegmentation:
    cdef c_RTSegmentation* thisptr

    # note regions
    NONE = 0
    ONSET = 1
    ATTACK = 2
    SUSTAIN = 3
    RELEASE = 4
    OFFSET = 5

    def __cinit__(self): self.thisptr = new c_RTSegmentation()
    def __dealloc__(self): del self.thisptr

    def spectral_centroid(self, np.ndarray[dtype_t, ndim=1] a):
        return self.thisptr.spectral_centroid(len(a), <double*> a.data)

    def segment(self, np.ndarray[dtype_t, ndim=1] a):
        return self.thisptr.segment(len(a), <double*> a.data)
