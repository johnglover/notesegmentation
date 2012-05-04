import numpy as np
import notesegmentation as ns


class TestUtils(object):
    def test_moving_average(self):
        a = ns.util.moving_average(np.array([3.0]), 1)
        assert a == np.array([3.0]), a
        a = ns.util.moving_average(np.array([1.0, 2.0, 3.0]), 2)
        assert np.all(a == np.array([1.0, 1.5, 2.5])), a
        a = ns.util.moving_average(np.array([1.0, 2.0, 3.0, 4.0, 5.0]), 2)
        assert np.all(a == np.array([1.0, 1.5, 2.5, 3.5, 4.5])), a

    def test_cumulative_moving_average(self):
        a = ns.util.cumulative_moving_average(np.array([3.0]))
        assert a == np.array([3.0]), a
        a = ns.util.cumulative_moving_average(np.array([1.0, 2.0, 3.0]))
        assert np.all(a == np.array([1.0, 1.5, 2.0])), a
        a = ns.util.cumulative_moving_average(np.array([1.0, 2.0, 3.0, 4.0, 5.0]))
        assert np.all(a == np.array([1.0, 1.5, 2.0, 2.5, 3])), a

    def test_c_cumulative_moving_average(self):
        a = ns.u.cumulative_moving_average(np.array([3.0]))
        assert a == np.array([3.0]), a
        a = ns.u.cumulative_moving_average(np.array([1.0, 2.0, 3.0]))
        assert np.all(a == np.array([1.0, 1.5, 2.0])), a
        a = ns.u.cumulative_moving_average(np.array([1.0, 2.0, 3.0, 4.0, 5.0]))
        assert np.all(a == np.array([1.0, 1.5, 2.0, 2.5, 3])), a

    def test_decreasing(self):
        a = np.array([1, 2, 3, 4, 5, 4, 3, 2, 1])
        assert not ns.util.decreasing(a, 3, 2)
        assert not ns.util.decreasing(a, 4, 2, 2)
        assert ns.util.decreasing(a, 8, 4)
        assert ns.util.decreasing(a, 8, 2, 2)

    def test_next_maxima(self):
        a = np.array([1, 2, 3, 4, 5, 4, 3, 4, 1])
        assert ns.util.next_maxima(a, 0, 1) == 4
        assert ns.util.next_maxima(a, 1, 3) == 4
        assert ns.util.next_maxima(a, 4, 1) == 7
        assert ns.util.next_maxima(a, 4, 4) == len(a)

    def test_next_minima(self):
        a = np.array([1, 2, 0, 4, 5, 4, 3, 4, 1])
        assert ns.util.next_minima(a, 0, 1) == 2
        assert ns.util.next_minima(a, 2, 1) == 6
        assert ns.util.next_minima(a, 2, 2) == len(a)
