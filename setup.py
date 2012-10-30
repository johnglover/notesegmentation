"""Real-Time Segmentation of the Temporal Evolution of Musical Sounds

Implementations of the segmentation techniques proposed by Caetano et al.
and Glover et al. discussed in the paper:
    Real-Time Segmentation of the Temporal Evolution of Musical Sounds
    by Glover, Lazzarini and Timoney
    Proceedings of Acoustics 2012 Hong Kong Conference
"""
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext
import numpy

try:
    numpy_include = numpy.get_include()
except AttributeError:
    numpy_include = numpy.get_numpy_include()

doc_lines = __doc__.split("\n")
include_dirs = [numpy_include, '/usr/local/include']

amplitude_envelopes = Extension(
    "notesegmentation._amplitude_envelopes",
    sources=["notesegmentation/_amplitude_envelopes.pyx",
             "src/amplitude_envelopes.cpp"],
    include_dirs=["src"] + include_dirs,
    language="c++"
)

util = Extension(
    "notesegmentation._util",
    sources=["notesegmentation/_util.pyx",
             "src/util.cpp"],
    include_dirs=["src"] + include_dirs,
    language="c++"
)

segmentation = Extension(
    "notesegmentation._segmentation",
    sources=["notesegmentation/_segmentation.pyx",
             "src/window.cpp",
             "src/amplitude_envelopes.cpp",
             "src/util.cpp",
             "src/segmentation.cpp"],
    include_dirs=["src"] + include_dirs,
    libraries=['modal', 'm', 'fftw3'],
    language="c++"
)

setup(
    name='notesegmentation',
    description=doc_lines[0],
    long_description="\n".join(doc_lines[2:]),
    url='http://github.com/johnglover/notesegmentation',
    download_url='http://github.com/johnglover/notesegmentation',
    license='GPL',
    author='John Glover',
    author_email='john.c.glover@nuim.ie',
    platforms=["Linux", "Mac OS-X", "Unix"],
    version='1.0',
    packages=['notesegmentation'],
    ext_modules=[amplitude_envelopes, util, segmentation],
    cmdclass={'build_ext': build_ext}
)
