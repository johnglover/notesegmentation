import sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.io.wavfile as wavfile
import notesegmentation as ns

if len(sys.argv) != 2:
    print 'Usage: python', __file__, '<path to wav file>'
    sys.exit(1)

file_name = sys.argv[1]
sampling_rate, audio = wavfile.read(file_name)
audio = np.asarray(audio, dtype=np.double)
audio /= np.max(audio)
metadata = {'sampling_rate': sampling_rate}

fig = plt.figure(1, figsize=(14, 9))
plt.plot(np.abs(audio), '0.4')

g = ns.segmentation.glt(audio, metadata)
for note in g:
    for boundary_name, boundary in note.iteritems():
        plt.axvline(boundary, c='r', linestyle='--')
        print "Boundary (%s): %d" % (boundary_name, boundary)

plt.show()
