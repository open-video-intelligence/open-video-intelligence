# Detect Type Plugin
Now, you can use the following detect type plugins in open-video-intelligence:
- [AudioDetection](#Audio-detection)

## Audio Detection
Detecting audio

# Effect Type Plugin
## Audio effect maker

## Video effect maker

# Render Type Plugin
Now, you can use the following render type plugins in open-video-intelligence:
- [FFmpeg render](#FFmpeg-render)
- [Otio render](#Otio-render)

## FFmpeg Render
This plugin supports simple cut and merge.</br>
  support video effects : boxblur, Apply a boxblur algorithm to the input video. This effect has intensity. Range: 0 ~ 100. Default : 10</br>
  support video effects : avgblur, Apply average blur filter. This effect has intensity. Range: 0 ~ 100. Default : 10</br>
### Build Requires
1) FFmpeg (https://ffmpeg.org/)</br>
FFmpeg is the multimedia framework, able to decode, encode and transcode contents.</br>
Run the following command:
   ```console
   $ sudo apt install ffmpeg
   ```

## OTIO Render
This plugin saves the OTIO object to a given file.
### Build Requires
1) OpenTimelineIO (https://github.com/AcademySoftwareFoundation/OpenTimelineIO)</br>
OpenTimelineIO is an interchange format and API for editorial cut information.</br>
Cmake needs 3.18.2 or higher.</br>
Run the following command:
   ```console
   $ git clone https://github.com/AcademySoftwareFoundation/OpenTimelineIO.git
   $ git checkout cxx17
   $ mkdir build
   $ cd build
   $ cmake .. { options }
   $ sudo make install
   ```

# Python plugin
Now, you can use the following detect type python plugins.
- [AudioDetectPy](#AudioDetectPy)

## AudioDetectPy
Provides the decibels of audio.

### Requires
Run the following command:
   ```console
   # Requires the latest pip
   $ pip install --upgrade pip

   $ pip install numpy
   ```
