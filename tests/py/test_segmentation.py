from nose.tools import assert_almost_equals
import numpy as np
import notesegmentation as ns


class TestSegmentation(object):
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
        assert_almost_equals(w, 2.333, places=3)  # mean of [2, 2, 3]
        assert M == 1
