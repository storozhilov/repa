# Repa - multichannel media recording and broadcasting platform

This solution was created by musicians for musicians to be used at
reheasals or any kind of live performance. It consists of two main
components: for live video and audio processing respectively.

## Multitrack live video recording and broadcasting subsystem

`vrepa` is a multitrack video recording, mixing and broadcasting solution to
be used either in professional or amateur environments. Just connect
all available video sources (webcams, action cams, mobile devices, etc.)
and do a TV-like broadcast.

## Multitrack live audio recording subsystem

`arepa-cli` & `arepa-gtk2` are multitrack audio recorders, which read directly from
ALSA and do their write operations in a separate thread using a lock-free ring-buffer
for the incoming audio data. That allows to get rid of X-runs (data losses) even on
poor hardware having a lot of input audio-channels to be recorded.

I decided to write this small program after fighting with X-runs faced either
on Jack or on Ardour side, when we tried to make two 8-channels audio-cards to
work fine on our Linux audio workstation. ALSA's **arecord** suffered from the
X-runs as well.

There was no low latency requirement, but we had just to record our live
sessions as WAV-files. So the idea was to get rid of that stuff at all.
It's always possible to import created audio-files into Ardour later on.

### Project's name explaination

*"Repa"* translates into *"turnip"* from Russian. Russian musicians frequently
use this slang name for the *"rehearsal"* or *"repetition"*.
