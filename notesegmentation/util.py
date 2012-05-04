import numpy as np


def next_minima(a, current, n=1):
    """
    Find the next local minima in a.
    Must be less than n adjacent values in each direction.
    """
    for i in range(current + 1, len(a) - 1):
        forward_neighbours = min(n, len(a) - (i + 1))
        backward_neighbours = min(n, i)
        minima = True
        # search all forward neighbours (up to a max of n), testing to see
        # if the current sample is less than all of them
        for p in range(forward_neighbours):
            if a[i] > a[i + p + 1]:
                minima = False
                break
            elif a[i] == a[i + p + 1]:
                # see if the next change is up or down
                p2 = p
                while p2 < (len(a) - i - 2) and a[i] == a[i + p2 + 1]:
                    p2 += 1
                if a[i] > a[i + p2 + 1]:
                    minima = False
                    break
        # if it is greater than 1 of the forward neighbours,
        # no need to check backwards
        if not minima:
            continue
        # now test the backwards neighbours
        for p in range(backward_neighbours):
            if a[i] > a[i - (p + 1)]:
                minima = False
                break
            elif a[i] == a[i - (p + 1)]:
                # see if the next change is up or down
                p2 = p
                while i - (p2 + 2) > 0 and a[i] == a[i - (p2 + 1)]:
                    p2 += 1
                if a[i] > a[i - (p2 + 1)]:
                    minima = False
                    break
        if minima:
            return i
    # if no minima found, return the last sample position
    return len(a)


def next_maxima(a, current, n=1):
    """
    Find the next local maxima in a.
    Must be less than n adjacent values in each direction.
    """
    for i in range(current + 1, len(a) - 1):
        forward_neighbours = min(n, len(a) - (i + 1))
        backward_neighbours = min(n, i)
        m = True
        # search all forward neighbours (up to a max of n), testing to see
        # if the current sample is greater than all of them
        for p in range(forward_neighbours):
            if a[i] < a[i + p + 1]:
                m = False
                break
            elif a[i] == a[i + p + 1]:
                # see if the next change is up or down
                p2 = p
                while p2 < (len(a) - i - 2) and a[i] == a[i + p2 + 1]:
                    p2 += 1
                if a[i] < a[i + p2 + 1]:
                    m = False
                    break
        if not m:
            continue
        # now test the backwards neighbours
        for p in range(backward_neighbours):
            if a[i] < a[i - (p + 1)]:
                m = False
                break
            elif a[i] == a[i - (p + 1)]:
                # see if the next change is up or down
                p2 = p
                while i - (p2 + 2) > 0 and a[i] == a[i - (p2 + 1)]:
                    p2 += 1
                if a[i] < a[i - (p2 + 1)]:
                    m = False
                    break
        if m:
            return i
    # if no minima found, return the last sample position
    return len(a)


def find_peaks(a, threshold, n=3):
    """
    Return all peaks (local maxima) in array a.
    """
    peaks = []
    for i in range(len(a)):
        if a[i] < threshold:
            continue
        forward_neighbours = min(n, len(a) - (i + 1))
        backward_neighbours = min(n, i)
        maxima = True
        # search all forward neighbours (up to a max of n), testing to see
        # if the current sample is bigger than all of them
        for p in range(forward_neighbours):
            if a[i] < a[i + p + 1]:
                maxima = False
                break
        # if it is less than 1 of the forward neighbours,
        # no need to check backwards
        if not maxima:
            continue
        # now test the backwards neighbours
        for p in range(backward_neighbours):
            if a[i] < a[i - (p + 1)]:
                maxima = False
                break
        if maxima:
            peaks.append(i)
    return peaks


def moving_average(s, n=5000):
    """
    For each point in numpy array s, return the moving average of
    the previous n points.
    """
    avg = np.zeros(len(s))
    for i in range(len(s)):
        avg[i] = np.abs(np.mean(s[max(0, (i - n) + 1):i + 1]))
    return avg


def cumulative_moving_average(s):
    """
    For each point in numpy array s, return the cumulative moving average.
    """
    avg = np.zeros(len(s))
    avg[0] = s[0]
    for i in range(1, len(s)):
        avg[i] = avg[i - 1] + ((s[i] - avg[i - 1]) / (i + 1))
    return avg


def decreasing(s, current, n, hop=1):
    """
    Return true if s is decreasing for n values from current backwards.
    """
    x = np.arange(current, current - (n * hop), -hop)
    for i in range(len(x) - 1):
        if s[x[i]] >= s[x[i + 1]]:
            return False
    return True
