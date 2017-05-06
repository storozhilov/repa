Repa - ALSA multitrack recorder
===============================

Multitrack recorder, which reads directly from ALSA and does
it's write operations in a separate thread. That allows to
get rid of X-runs (data losses) even on poor hardware.
