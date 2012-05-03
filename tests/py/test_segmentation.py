import os
import nose.tools as nt
import numpy as np
import scipy.io.wavfile as wavfile
import notesegmentation as ns


class TestSegmentation(object):
    float_precision = 5
    frame_size = 512
    audio_path = os.path.join(
        os.path.dirname(__file__), '../clarinet-C-octave0.wav'
    )

    @classmethod
    def setup_class(cls):
        cls.audio = wavfile.read(cls.audio_path)[1]
        cls.audio = np.asarray(cls.audio, dtype=np.double)
        cls.audio /= np.max(cls.audio)

    def test_effort_times(self):
        a = np.linspace(10, 100, 10)
        t = np.array([0, 25, 50, 75])
        effort_times = ns.segmentation._effort_times(a, t)
        assert np.all(effort_times == np.array([0, 2, 4, 7]))

    def test_efforts(self):
        a = np.linspace(10, 100, 10)
        t = np.array([0, 25, 50, 75])
        efforts, w, M = ns.segmentation._efforts(a, t)
        assert np.all(efforts == np.array([2, 2, 3]))
        nt.assert_almost_equals(w, 2.333, places=3)  # mean of [2, 2, 3]
        assert M == 1

    def test_spectral_centroid(self):
        glt = ns.s.GLT()
        audio_metadata = {'sampling_rate': 44100}

        i = 0
        while i < len(self.audio):
            f = self.audio[i:i + self.frame_size]
            py_sc = ns.segmentation.spectral_centroid(f, audio_metadata)[-1]
            c_sc = glt.spectral_centroid(f)
            nt.assert_almost_equals(py_sc, c_sc, self.float_precision)
            i += self.frame_size
