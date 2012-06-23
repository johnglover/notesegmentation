import os
import nose.tools as nt
import numpy as np
import scipy.io.wavfile as wavfile
import notesegmentation as ns


class TestSegmentation(object):
    float_precision = 5
    hop_size = 512
    frame_size = 2048
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
        metadata = {'sampling_rate': 44100}

        i = 0
        while i < len(self.audio):
            f = self.audio[i:i + self.hop_size]
            py_sc = ns.segmentation.spectral_centroid(f, metadata)[-1]
            c_sc = glt.spectral_centroid(f)
            nt.assert_almost_equals(py_sc, c_sc, self.float_precision)
            i += self.hop_size

    def test_segment(self):
        metadata = {'sampling_rate': 44100,
                    'spectral_centroid_frame_size': 512,
                    'spectral_centroid_hop_size': 512,
                    'odf_frame_size': 512,
                    'odf_hop_size': 512,
                    'env_hop_size': 512}
        py_segments = ns.segmentation.glt(self.audio, metadata)[0]

        c_segments = {}
        glt = ns.s.GLT()

        i = 0
        while i < len(self.audio):
            f = self.audio[i:i + self.frame_size]
            s = glt.segment(f)
            if not 'onset' in c_segments and s == glt.ONSET:
                c_segments['onset'] = i
            elif not 'sustain' in c_segments and s == glt.SUSTAIN:
                c_segments['sustain'] = i
            i += self.hop_size

        print py_segments
        print c_segments

        # expected_keys = ['onset', 'sustain', 'release', 'offset']
        # for k in expected_keys:
        #     assert py_segments[k] == c_segments[k]
