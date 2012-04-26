import numpy as np


def rms(audio, n=512, m=3):
    '''
    Root Mean Square amplitude envelope
    '''
    rms = np.zeros(len(audio))
    i = 0
    while i <= (len(audio) - n):
        start = rms[i - 1 if i else 0]
        num_values = min(i, m - 1)
        prev_values = rms[i - num_values:i]
        current = np.sqrt(np.sum(audio[i:i + n] ** 2) / n)
        values = np.hstack((prev_values, np.array([current])))
        end = np.mean(values)
        rms[i:i + n] = np.linspace(start, end, n)
        i += n
    return rms


def rms_frame(audio, n=512, m=3):
    '''
    Root Mean Square amplitude envelope
    '''
    rms = np.zeros((len(audio) / n) + 1)
    prev_values = np.zeros(m)
    i, frame = 0, 0
    while i <= len(audio):
        current = np.sqrt(np.sum(audio[i:i + n] ** 2) / n)
        prev_values = np.hstack((np.array([current]),
                                 np.roll(prev_values, 1)[1:]))
        rms[frame] = np.mean(prev_values)
        i += n
        frame += 1
    return rms


def _tae(audio, order=20):
    '''
    Calculate the True Amplitude Envelope of the given audio signal,
    with the given order.
    '''
    # extra zero padding to nearest power of 2
    if not len(audio) % 2 == 0:
        next_power_of_2 = 2 ** np.ceil(np.log2(len(audio)))
        audio = np.hstack((audio, np.zeros(next_power_of_2 - len(audio))))

    N = len(audio)
    fwr = np.abs(audio)
    threshold = 0.025 * np.max(fwr)

    go = True
    while go:
        ceps = np.fft.ifft(fwr)
        ceps[order:N - order] = 0.0
        env = np.fft.fft(ceps).real
        fwr = np.maximum(fwr, env)
        go = ((fwr - env) > threshold).any()

    return env


def tae(audio, frame_size=2048, hop_size=1024):
    '''
    Streaming version of the True Amplitude Envelope.
    '''
    env = np.zeros(len(audio))
    # TODO: should we calculate an optimal order here?
    #       this value seems to work well in practice
    order = 6
    p = 0

    while p <= len(audio) - frame_size:
        frame = audio[p:p + frame_size]
        env[p:p + frame_size] += _tae(frame, order) * np.hanning(len(frame))
        p += hop_size

    return env
