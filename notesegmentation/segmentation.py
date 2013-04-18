import numpy as np
import modal
import modal.onsetdetection as od
import amplitude_envelopes as ae
import util


class NoOnsetsFound(Exception):
    pass


def odf(audio, metadata, return_odf=False):
    frame_size = int(metadata.get('odf_frame_size', 2048))
    hop_size = int(metadata.get('odf_hop_size', 512))
    sampling_rate = int(metadata.get('sampling_rate', 44100))

    o = modal.PeakAmpDifferenceODF()
    o.set_frame_size(frame_size)
    o.set_hop_size(hop_size)
    o.set_sampling_rate(sampling_rate)

    onsets = []
    odf_values = []
    onset_det = od.RTOnsetDetection()

    p = 0
    while p <= len(audio) - frame_size:
        frame = audio[p:p + frame_size]
        odf_value = o.process_frame(frame)
        odf_values.append(odf_value)
        det_results = onset_det.is_onset(
            odf_value, o.max_odf_value(), return_threshold=True
        )
        if det_results[0]:
            onsets.append(p)
        p += hop_size

    if not return_odf:
        return onsets
    else:
        return (onsets, np.array(odf_values))


def spectral_centroid(audio, metadata):
    frame_size = int(metadata.get('spectral_centroid_frame_size', 512))
    hop_size = int(metadata.get('spectral_centroid_hop_size', 512))
    sampling_rate = int(metadata.get('sampling_rate', 44100))

    num_bins = (frame_size / 2) + 1
    freqs = np.linspace(0.0, float(sampling_rate) / 2, num_bins)
    sc = []

    p = 0
    while p <= len(audio) - frame_size:
        frame = audio[p:p + frame_size]
        centroid = 0.0

        window = np.hamming(frame_size)
        f = np.fft.rfft(frame * window)
        magnitudes = abs(f)

        if sum(magnitudes):
            centroid = sum(freqs * magnitudes) / sum(magnitudes)

        sc.append(centroid)
        p += hop_size

    return np.array(sc)


def _effort_times(s, thresholds):
    t = np.zeros(len(thresholds))
    current_threshold = 0
    for i in range(len(s)):
        if s[i] >= thresholds[current_threshold]:
            t[current_threshold] = i
            current_threshold += 1
            if current_threshold >= len(thresholds):
                break
    # this shouldn't happen, but just in case...
    if current_threshold < len(thresholds):
        t[current_threshold:len(thresholds)] = len(s)
    return t


def _efforts(s, thresholds):
    effort_times = _effort_times(s, thresholds)
    efforts = np.diff(effort_times)
    w = np.mean(efforts)
    M = len(efforts[efforts > w])  # no. efforts greater than mean
    return efforts, w, M


def cbr(audio, metadata, verbose=False):
    """
    Implementation of the automatic note segmentation given in:

        Caetano, M., Burred, J. J., Rodet, X.
        Automatic Segmentation of the Temporal Evolution of Isolated Acoustic
        Musical Instrument Sounds Using Spectro-Temporal Cues.
        In Proc. DAFx 2010

    Returns a list of dictionaries (one for each note) with the following keys:
    * onset
    * end_attack
    * sustain
    * release
    * offset
    """
    if verbose:
        print 'Segmenting %s with the Caetano, Burred, Rodet method' % \
            metadata.get('name', 'unknown audio file')

    notes = []

    onsets = odf(audio, metadata)
    if not onsets:
        raise NoOnsetsFound()
    onsets = [onsets[0]]

    env = ae.tae(np.abs(audio))
    sc = spectral_centroid(audio, metadata)

    for onset_number, onset in enumerate(onsets):
        # if time to prev onset is < 200 ms, ignore this onset
        note_duration = onset - onsets[onset_number - 1]
        max_note_duration = (metadata['sampling_rate'] * 200) / 1000
        if onset_number > 0 and note_duration < max_note_duration:
                if verbose:
                    print 'Warning: Detected note duration is too short',
                    print '(%d to %d, %d samples). Skipping.' % (
                        onsets[onset_number - 1], onset, note_duration
                    )
                continue

        boundaries = {
            'onset': onset,
            'end_attack': onset,
            'sustain': onset,
            'release': onset,
            'offset': len(audio)
        }

        # offset: last point TAE has same energy as onset
        n = 512
        onset_energy = np.sum(env[np.max(onset - n, 0):onset] ** 2)
        audio_pos = onset + (metadata['sampling_rate'] * 200 / 1000)
        while audio_pos <= len(audio) - n:
            frame = env[audio_pos:audio_pos + n]
            energy = np.sum(frame ** 2)
            if energy <= onset_energy:
                break
            audio_pos += n
        boundaries['offset'] = audio_pos

        # beginning of sustain / end of attack
        # using a modified version of Peeters' efforts method
        max_transient = min(onset + (metadata['sampling_rate'] * 500) / 1000,
                            len(env) - onset)
        if onset == max_transient:
            if verbose:
                print 'Warning: onset detected at end of signal, ignoring'
            continue
        max_env = np.max(env[onset:max_transient])
        max_env_loc = np.argmax(env[onset:max_transient]) + onset
        effort_thresholds = np.linspace(0, max_env, 10)
        effort_times = \
            _effort_times(env[onset:max_env_loc], effort_thresholds) + onset
        efforts = np.diff(effort_times)
        w = np.mean(efforts)
        M = len(efforts[efforts > w])  # no. efforts greater than mean

        # start of sustain: first point at which effort > M * w
        sustain = max_env_loc
        for i, effort in enumerate(efforts):
            if effort > M * w:
                sustain = effort_times[i]
        boundaries['sustain'] = sustain

        # end of attack: first local minima (in spectral centroid)
        # between onset and sustain
        sc_minima = util.next_minima(sc, onset / 512) * 512
        boundaries['end_attack'] = min(sustain, sc_minima)

        # start of release:  using a modified version of Peeters'
        # efforts method (in reverse)
        max_env = np.max(env[onset:boundaries['offset']])
        max_env_loc = np.argmax(env[onset:boundaries['offset']]) + onset
        effort_thresholds = np.linspace(0, max_env, 10)
        effort_times = _effort_times(env[boundaries['offset']:onset:-1],
                                     effort_thresholds)
        efforts = np.diff(effort_times)
        w = np.mean(efforts)
        M = len(efforts[efforts > w])

        # first point at which effort > M * w
        release = max_env_loc
        for i, effort in enumerate(efforts):
            if effort > M * w:
                release = boundaries['offset'] - effort_times[i]
        boundaries['release'] = release

        notes.append(boundaries)
    return notes


def rtsegmentation(audio, metadata, verbose=False):
    """
    Implementation of real-time automatic note segmentation, as described in:
        Glover, J., Lazzarini, V., and Timoney, J.
        Real- time segmentation of the temporal evolution of musical sounds
        Proceedings of the Acoustics 2012 Hong Kong Conference

    Returns a list of dictionaries (one for each note) with the following keys:
    * onset
    * sustain
    * release
    * offset
    """
    if verbose:
        print 'Segmenting %s with the Glover, Lazzarini and Timoney method' % \
            metadata.get('name', 'unknown audio file')

    notes = []

    onsets, o = odf(audio, metadata, return_odf=True)
    if not onsets:
        raise NoOnsetsFound()
    onsets = [onsets[0]]

    odf_hop_size = metadata.get('odf_hop_size', 512)
    env_hop_size = metadata.get('env_hop_size', 512)

    env = ae.rms_frame(audio, n=env_hop_size)
    env_no_ma = ae.rms_frame(audio, n=env_hop_size, m=1)
    sc = spectral_centroid(audio, metadata)
    centroid_cma = util.cumulative_moving_average(sc)

    for onset_number, onset in enumerate(onsets):
        # if time to prev onset is < 200 ms, ignore this onset
        note_duration = onset - onsets[onset_number - 1]
        max_note_duration = (metadata['sampling_rate'] * 200) / 1000
        if onset_number > 0 and note_duration < max_note_duration:
                if verbose:
                    print 'Warning: Detected note duration is too short',
                    print '(%d to %d, %d samples). Skipping.' % (
                        onsets[onset_number - 1], onset, note_duration
                    )
                continue

        boundaries = {'onset': onset,
                      'sustain': onset,
                      'release': len(audio),
                      'offset': len(audio)}

        onset_frame = onset / odf_hop_size

        # transient/attack: from onset until next minima in odf,
        #                   unless amp local max reached first
        boundaries['sustain'] = min(
            util.next_maxima_rt(env, onset_frame) * env_hop_size,
            util.next_minima_rt(o, onset_frame) * odf_hop_size
        )

        # release: 3 consecutive frames with decreasing energy and
        #          spectral centroid below average
        r = util.next_maxima(env, onset_frame)
        peak_amp = env[r]
        for i in range(r, len(env) - 1):
            peak_amp = max(peak_amp, env[i])
            is_release = ((env[i] <= (0.8 * peak_amp) and
                           util.decreasing(env, i, 5) and
                           sc[i] < centroid_cma[i]) or
                          env[i] <= 0.33 * peak_amp)
            if is_release:
                boundaries['release'] = i * env_hop_size
                break

        # offset: point (after sustain) at which envelope <=-60db
        #         below peak value
        peak_amp = np.max(np.abs(audio))
        peak_location = np.argmax(np.abs(audio)) / env_hop_size
        rt60 = (10 ** -3) * peak_amp
        offset = peak_location * env_hop_size
        for i in range(peak_location, len(env_no_ma)):
            if env_no_ma[i] <= rt60:
                offset = i * env_hop_size
                break
        boundaries['offset'] = offset

        notes.append(boundaries)
    return notes
