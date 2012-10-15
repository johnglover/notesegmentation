import numpy as np
import simpl
import util


def _is_active(partial):
    '''
    Return True if partial is active.
    '''
    return partial.amplitude > 0


def _deviation(partial1, partial2):
    '''
    Return the deviation between two partials.
    This is the difference in the log (to base 2) of their frequency values,
    multiplied by their amplitudes.
    '''
    if partial1.frequency < 1e-6:
        deviation = np.log2(partial2.frequency)
    elif partial2.frequency < 1e-6:
        deviation = np.log2(partial1.frequency)
    else:
        deviation = np.log2(partial2.frequency) - np.log2(partial1.frequency)
    deviation *= np.log10(np.max([partial1.amplitude, partial2.amplitude]))
    return np.abs(deviation)


def get_stability(audio, metadata, frame_size=512, hop_size=512,
                  num_partials=60, stretched=False):
    '''
    Return a partial stability signal (as a numpy array) for a
    given audio signal.
    '''
    pd = simpl.SMSPeakDetection()
    pd.max_peaks = num_partials
    pd.frame_size = frame_size
    pd.hop_size = hop_size
    peaks = pd.find_peaks(audio)
    pt = simpl.SMSPartialTracking()
    pt.max_frame_delay = 145
    pt.analysis_delay = 100
    pt.min_good_frames = 3
    pt.clean_tracks = 1
    pt.harmonic = True
    pt.max_partials = num_partials
    frames = pt.find_partials(peaks)

    deviations = np.zeros(len(audio))
    non_stretched_deviations = []

    p = 0
    for frame_number in range(1, len(frames)):
        f1 = frames[frame_number - 1]
        f2 = frames[frame_number]
        avg = 0.0
        partials1 = f1.partials
        partials2 = f2.partials
        for i in range(num_partials):
            if _is_active(partials1[i]) or _is_active(partials2[i]):
                avg += _deviation(partials2[i], partials1[i])
        avg /= num_partials
        non_stretched_deviations.append(avg)
        start = deviations[frame_number - 1 if frame_number else 0]
        deviations[p:p + hop_size] = np.linspace(start, avg, hop_size)
        p += hop_size

    if stretched:
        return deviations
    else:
        return np.array(non_stretched_deviations)


def get_transients(audio, metadata, frame_size=256,
                   hop_size=128, num_partials=10):
    '''
    Return a list of transient regions in a given audio signal.

    Transient start: onset location
    Transient end:
        * find largest peak following onset
        * if no other large peaks (value > 25% of largest peak)
          within 10 hop sizes of next minima, just get up to next minima.
        * if larger peaks found, get up to minima after last peak

    Returns a dictionary of the form:
        {'start': <int>, 'end': <int>}
    '''
    sampling_rate = int(metadata.get('sampling_rate', 44100))
    transients = []
    stability = get_stability(audio, metadata, frame_size,
                              hop_size, num_partials, False)
    peaks = util.find_peaks(stability, np.mean(stability))

    for onset in metadata['onsets']:
        transient = {'start': onset, 'end': onset}

        # ignore if not within 200ms of a reference onset
        candidate_peaks = [
            p for p in peaks if (p >= (onset / hop_size)) and
            ((p * hop_size) - onset <= ((sampling_rate / 1000) * 200))
        ]
        if not candidate_peaks:
            continue

        # find the largest peak after each onset
        transient_peak = 0
        peak_value = stability[candidate_peaks[0]]
        for p in range(1, len(candidate_peaks)):
            if stability[candidate_peaks[p]] > peak_value:
                transient_peak = p
                peak_value = stability[candidate_peaks[p]]

        # find minima after last peak in the cluster, if any
        transient_peak = candidate_peaks[transient_peak]
        close_peaks = []
        last_peak = transient_peak
        for p in candidate_peaks:
            if (np.abs(p - last_peak) <= 10) and \
                stability[p] >= (0.25 * peak_value):
                close_peaks.append(p)
                last_peak = p
        last_peak = transient_peak if not close_peaks else close_peaks[-1]
        transient['end'] = min(
            onset + ((sampling_rate / 1000) * 200),
            util.next_minima(stability, last_peak) * hop_size
        )

        transients.append(transient)

    return transients
