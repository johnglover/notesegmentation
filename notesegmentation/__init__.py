import amplitude_envelopes
import segmentation
import partial_stability
import util

try:
    import _amplitude_envelopes as ae
except ImportError:
    pass

try:
    import _segmentation as s
except ImportError:
    pass
