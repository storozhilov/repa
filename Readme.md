Repa - xruns-free ALSA multitrack recorder
==========================================

Multitrack recorder, which reads directly from ALSA and does it's write
operations in a separate thread. That allows to get rid of X-runs (data losses)
even on poor hardware having a lot of input audio-channels to be recorded.

*"Repa"* translates into *"turnip"* from Russian. Russian musicians frequently
use this slang name for the *"rehearsal"* or *"repetition"*.

I decided to write this small program after fighting with X-runs faced either
on Jack or on Ardour side, when we tried to make two 8-channles audio-cards to
work fine on our Linux audio workstation. ALSA's **arecord** suffered from the
X-runs as well.

There was no low latency requirement, but we had to just to record our live
sessions as WAV-files. So the idea was just to get rid of that stuff at all.
It's always possible to import created audio-files into Ardour later on.
