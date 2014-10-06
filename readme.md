MASS: Motion-Aware Sensory Substitution
=======================================
![A screenshot of the MASS visualizer.](/screenshot.png)

MASS is an open-source, configurable sensory substitution system inspired by [the vOICe](http://www.seeingwithsound.com/). It features efficient resampling, motion detection, signal accumulation, and video -> audio encoding, with a visual feedback for experimenters.

Compile-Time Dependencies
-------------------------
- **A C Compiler:** Available [here](https://gcc.gnu.org/install/binaries.html).
- **Ruby:** Available [here](https://www.ruby-lang.org/en/downloads/).
- **Rake:** Enter `gem install rake` in a terminal.

On Ubuntu/Debian, you can install everything with the command `sudo apt-get install rake`.

Run-Time Dependencies
---------------------
- **GStreamer:** Available [here](http://gstreamer.freedesktop.org/download/).

On Ubuntu/Debian, you can install everything with the command `sudo apt-get install libgstreamer1.0-dev`.

Compilation
-----------
The default configuration can be built with `rake build`. An executable file will then be available at "bin/default". `rake run` will both build and run the application.

Configuration
-------------
A custom configuration can be built or run with `rake build_<config>` or `rake run_<config>`, where <config> is the name of a file in "conf" (excluding the extension). For a custom configuration, create a header file in the "conf" directory and define any combination of the symbols in the following list. The build script will automatically include this file in the build.

Configurable Symbols
--------------------
- `FRAME_HEIGHT`: The height of the video stream, in pixels (default: 36).
- `FRAME_WIDTH`: The width of the video stream, in pixels (default: 64).
- `SCAN_FREQUENCY`: The rate at which the sonifier traverses the width of the video stream (default: 1.0)
- `USE_MOTION_SENSOR`: Whether to transform the video stream into a representation its motion energy (default: true).
- `USE_ACCUMULATOR`: Whether to accumulate the value of each pixel over time (default: true).
- `ACCUMULATOR_THRESHOLD`: The minimum pixel intensity value the accumulator will accept; lower values will be disregarded (default: 0.01).
- `ACCUMULATOR_DECAY`: The factor that multiplies the accumulator contents at each frame (default: 0.95).
- `USE_HIGHLIGHTER`: Whether to visually highlight the current scan position (default: true).
- `HIGHLIGHT_WIDTH`: The width of the highlight, in pixels (default: 4).
- `HIGHLIGHT_COLOR`: The color of the highlight, as a red/green/blue hex code (default: 0x00bbff).
- `AUDIO_SAMPLE_RATE`: The rate at which the sound device should capture audio information, in samples-per-channel-per-second (default: 44100).
- `AUDIO_LATENCY`: The target latency of the audio output, in seconds (default: 0.005).
- `AUDIO_FREQUENCY`: A function or macro that maps a pixel position--ranging from 0.0 (top) to 1.0 (bottom)--to an audio frequency; `<math.h>` functions may be used as part of this definition.
- `AUDIO_AMPLITUDE`: A function or macro that maps a luminance--ranging from 0.0 to 1.0--and a frequency to an amplitude--ranging from 0.0 to 1.0; `<math.h>` functions may be used as part of this definition.

Predefined Configurations
-------------------------
- **Rapid Scan:** Increases `SCAN_FREQUENCY` and decreases `ACCUMULATOR_DECAY`.
- **High Resolution:** Increases the vertical and horizontal resolution of the video stream tenfold.
- **Pentatonic:** Assigns each row in the video stream a pitch in the pentatonic scale.
- **vOICe:** Emulates the vOICe sensory substitution system.