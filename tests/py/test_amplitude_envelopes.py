import os
import numpy as np
import scipy.io.wavfile as wavfile
from nose.tools import assert_almost_equals
import notesegmentation as ns


class TestAmplitudeEnvelopes(object):
    float_precision = 5
    frame_size = 512
    audio_path = os.path.join(
        os.path.dirname(__file__), '../audio/clarinet.wav'
    )

    @classmethod
    def setup_class(cls):
        cls.audio = wavfile.read(cls.audio_path)[1]
        cls.audio = np.asarray(cls.audio, dtype=np.double)
        cls.audio /= np.max(cls.audio)

    def test_rms(self):
        py_rms = ns.amplitude_envelopes.rms_frame(
            self.audio, self.frame_size, 1
        )

        i, frame = 0, 0
        while i < len(self.audio):
            rms = ns.ae.rms(self.audio[i:i + self.frame_size])
            assert_almost_equals(py_rms[frame], rms, self.float_precision)
            i += self.frame_size
            frame += 1

    def test_rms_ma(self):
        py_rms = ns.amplitude_envelopes.rms_frame(
            self.audio, self.frame_size, 5
        )

        i, frame = 0, 0
        prev = np.zeros(5, dtype=np.double)
        while i < len(self.audio):
            rms = ns.ae.rms_ma(self.audio[i:i + self.frame_size], prev)
            assert_almost_equals(py_rms[frame], rms, self.float_precision)
            i += self.frame_size
            frame += 1
