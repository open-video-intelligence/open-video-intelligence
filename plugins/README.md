# Detect Type Plugin
Now, you can use the following detect type plugins in open-video-intelligence:
- [AudioDetect](#Audio-detect)
- [FaceDetect](#Face-detect)

## Audio Detect
Detecting audio

## Face detect
Detecting faces
### Build Requires
1) opencv (https://opencv.org/)</br>
Install opencv for using 'plugins/faceDetect'.</br>
Run the following command:
   ```console
   $ sudo apt install libopencv-dev
   ```
2) Model files</br>
This plugin requires pretrained models. (`haarcascade_eye.xml`, and `haarcascade_frontalface_alt2.xml`)</br>
The pretrained models are located in the data folder in the OpenCV installation or can be found [here](https://github.com/opencv/opencv).</br>
Add model files by referring to the following structure.
create a 'models' folder if necessary:</br>
   ```bash
   ├── faceDetect
   │   ├── models
   │   │   ├── haarcascade_eye.xml
   │   │   └── haarcascade_frontalface_alt2.xml
   │   ├── CMakeLists.txt
   │   └── faceDetect.cpp
   ├── __init__.py
   ├── CMakeLists.txt
   └── README.md
   ```

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
- [FaceDetectPy](#FaceDetectPy)
- [ObjectDetectPy](#ObjectDetectPy)

## AudioDetectPy
Provides the decibels of audio.

### Requires
Run the following command:
   ```console
   # Requires the latest pip
   $ pip install --upgrade pip

   $ pip install numpy
   ```

## FaceDetectPy
Provides face detection results using [MediaPipe](https://developers.google.com/mediapipe). </br>
You can also get the opposite result through the 'inverse' attribute value.
Please refer to the plugin description for more information.

### Requires
1) MediaPipe (https://developers.google.com/mediapipe) </br>
MediaPipe Framework is the low-level component used to build efficient on-device machine learning pipelines, similar to the premade MediaPipe Solutions.</br>
Run the following command:
   ```console
   # Requires the latest pip
   $ pip install --upgrade pip

   $ pip install mediapipe
   ```

2) Pretrained model</br>
This plugin requires the pretrained model. (`blaze_face_short_range.tflite`)</br>
The pretrained model can be found [here](https://developers.google.com/mediapipe/solutions/vision/face_detector#get_started).</br>
Add model files by referring to the following structure.
create a 'models' folder if necessary:</br>
   ```bash
   ├── faceDetectPy
   │   ├── models
   │   │   └── blaze_face_short_range.tflite
   │   ├── CMakeLists.txt
   │   └── faceDetect.py
   ├── __init__.py
   ├── CMakeLists.txt
   └── README.md
   ```

## ObjectDetectPy
Provides object detection results using [MediaPipe](https://developers.google.com/mediapipe). </br>
Finds objects that match the specified 'Label' attribute value.
Please refer to the plugin description for more information.

### Requires
1) MediaPipe (https://developers.google.com/mediapipe) </br>
MediaPipe Framework is the low-level component used to build efficient on-device machine learning pipelines, similar to the premade MediaPipe Solutions.</br>
Run the following command:
   ```console
   # Requires the latest pip
   $ pip install --upgrade pip

   $ pip install mediapipe
   ```

2) Pretrained model</br>
This plugin requires the pretrained model. (`efficientdet_lite0.tflite`)</br>
The pretrained model can be found [here](https://developers.google.com/mediapipe/solutions/vision/object_detector).</br>
Add model files by referring to the following structure.
create a 'models' folder if necessary:</br>
   ```bash
   ├── objectDetectPy
   │   ├── models
   │   │   └── efficientdet_lite0.tflite
   │   ├── CMakeLists.txt
   │   └── objectDetect.py
   ├── __init__.py
   ├── CMakeLists.txt
   └── README.md
   ```
