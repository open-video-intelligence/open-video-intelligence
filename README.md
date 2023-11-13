# open-video-intelligence
Open Video Intelligence is an Open Source project. The aim of the Open Video Intelligence is to develope a software framework that can easily connect various plugins to easily and efficiently edit video contents.

## Maintainers
- HaeJeong Kim
- JiYong Min
- MinJe Ahn
- SeungBae Shin

## Reviewers
- HaeJeong Kim
- JiYong Min
- MinJe Ahn
- SeungBae Shin

## Components
A Component is a unit of the source code that accomplishes a specific objective, a subsystem of Open Video Intelligence. Components are established by the development community on as-need basis.

All Components are led by Reviewers and/or Maintainers. New Component can be suggested by any member of the Community, but the final decision should be made by maintainers. Components that are no longer active and being archived, could not have assigned Reviewers and Maintainers.

| Component | Directory | Description |
| ------------- | ------------- | ------------- |
| Core | src, include | OVI core library |
| Plugins | plugins | Default plugins |
| Python | python | Python files to run the OVI core library |
| Tools | tools | The Command-Line Interface(CLI) using the OVI core library |
| Tests | tests | Unit test for OVI core |
| Template | templates | Template for plugin creation |
| Configuration | config | Settings used to run the OVI core library |

## Build Requires
- cmake (https://cmake.org/), ninja-build (https://ninja-build.org/)

The cmake is cross-platform family of tools designed to build and the ninja-build is a small build system with a focus on speed.
   ```console
   $ sudo apt install g++ cmake ninja-build
   ```
- OpenTimelineIO (https://github.com/AcademySoftwareFoundation/OpenTimelineIO)

OpenTimelineIO is an interchange format and API for editorial cut information.
Cmake needs 3.18.2 or higher.
Run the following command:
   ```console
   $ git clone https://github.com/AcademySoftwareFoundation/OpenTimelineIO.git
   $ cd OpenTimelineIO
   $ git checkout cxx17
   $ mkdir build
   $ cd build
   $ cmake .. { options }
   $ sudo make install
   ```
- FFmpeg (https://ffmpeg.org/)

FFmpeg is the multimedia framework, able to decode, encode and transcode contents.
Run the following command:
   ```console
   $ sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswresample-dev libswscale-dev ffmpeg
   ```
   **_NOTE:_** You need more requires to use plugins, please check [here](plugins/README.md).

## How to build
There are two ways to build Open Video Intelligence.
- Using a build script
- Using a cmake build

### Use a build script
Open Video Intelligence provides a build script.
To build and install, use the ovi_build.sh script as follows:
  ```console
  $ ./ovi_build.sh
  ```
This command builds and installs the OVI library with default plugins, tools and tests.

### Use a cmake build
The Open Video Intelligence provides a cmake build.
To build and install, use the command as follows:
  ```console
  $ cmake -B ./build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DENABLE_PYTHON=ON -G Ninja
  $ sudo ninja install
  ```

## How to use
The Open Video Intelligence provides the Command-Line Interface(CLI) and sample applications.
Please refer to [this page](tools/README.md) for more information.

### How to use Command-Line Interface
You can use cli tools as follow.

   ```console
   $ ovi_plugins --list-all
   $ ovi_session -i ./movie.mp4 -r FFMPEGRender'(path=./result.mp4)' -skv 3 -l 'AudioDetect' -verbose 4
   ```

## How to contribute
Contributions are welcome! Please see our [contribution guide](contribution.md) for more details.

## Tests
The Open Video Intelligence provides tools for testing functions.

### Unit test
`ovi_unittest.sh` will run various unit tests. However, Following extra environment are required.

   ```console
   $ sudo apt install libgtest-dev libgmock-dev
   $ cd /usr/src/gtest
   $ sudo cmake CMakeLists.txt
   $ sudo make
   $ sudo cp lib/*.a /usr/lib
   $ cd /usr/src/googletest/googlemock/
   $ sudo mkdir build
   $ sudo cmake ..
   $ sudo make
   $ sudo cp lib/*.a /usr/lib
   ```
It can also run a specific or one unit test as below.
   ```console
   $ ./ovi_unittest.sh SessionTest
   $ ./ovi_unittest.sh ${TEST_CLASS_NAME} ${TEST_NAME}
   ```

### Coverage
After unittest or testsuite run, you can check the code coverage
   ```console
   $ ./ovi_gcov.sh
   ....
   Overall coverage rate:
     lines......: 77.6% (919 of 1185 lines)
     functions..: 89.1% (155 of 174 functions)
   ```
   ![image](https://media.github.sec.samsung.net/user/33676/files/77387ad8-c457-4fce-b004-ba8d13234cc9)

## License
Apache 2.0, Please refer to [LICENSE.APLv2](LICENSE.APLv2) file.
