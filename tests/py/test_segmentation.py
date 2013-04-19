import os
import nose.tools as nt
import numpy as np
import scipy.io.wavfile as wavfile
import notesegmentation as ns


class TestSegmentation(object):
    float_precision = 5
    hop_size = 512
    frame_size = 512
    audio_path = os.path.join(
        os.path.dirname(__file__), '../audio/clarinet.wav'
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
        rt = ns.s.RTSegmentation()
        metadata = {'sampling_rate': 44100}

        i = 0
        while i < len(self.audio) - self.frame_size:
            f = self.audio[i:i + self.hop_size]
            py_sc = ns.segmentation.spectral_centroid(f, metadata)
            c_sc = rt.spectral_centroid(f)
            nt.assert_almost_equals(py_sc, c_sc, self.float_precision)
            i += self.hop_size

    def test_segment(self):
        metadata = {'sampling_rate': 44100,
                    'spectral_centroid_frame_size': 512,
                    'spectral_centroid_hop_size': 512,
                    'odf_frame_size': 512,
                    'odf_hop_size': 512,
                    'env_hop_size': 512}
        py_segments = ns.segmentation.rtsegmentation(self.audio, metadata)[0]

        c_segments = {}
        rt = ns.s.RTSegmentation()

        i = 0
        while i < len(self.audio):
            f = self.audio[i:i + self.frame_size]
            if len(f) < self.frame_size:
                f = np.hstack((f, np.zeros(self.frame_size - len(f))))

            s = rt.segment(f)

            if not 'onset' in c_segments and s == rt.ONSET:
                c_segments['onset'] = i
            elif not 'sustain' in c_segments and s == rt.SUSTAIN:
                c_segments['sustain'] = i
            elif not 'release' in c_segments and s == rt.RELEASE:
                c_segments['release'] = i
            elif not 'offset' in c_segments and s == rt.OFFSET:
                c_segments['offset'] = i
            i += self.hop_size

        expected_keys = ['onset', 'sustain', 'release', 'offset']
        for k in expected_keys:
            assert py_segments[k] == c_segments[k]
