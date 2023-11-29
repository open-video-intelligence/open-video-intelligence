# Python Interface
Interface is provided through [ovi.py](ovi.py). </br>
See the docstring for more information. </br>

# Python Tool
[ovi_launcher](ovi_launcher) is command-based tool using python interface. </br>
Using this file, you can run simple operation or get development hints. </br>
For more information, run the following command: </br>
   ```console
   $ ./ovi_launcher

   # For more information, use '-h' options
   $ ./ovi_launcher -h

   usage: ./ovi_launcher [options]

   optional arguments:
   -h, --help            show this help message and exit
   -p {all,render,video,audio}, --plugins {all,render,video,audio}
                           Show the plugin list
   -i PATH, --input PATH
                           Input media path
   -r NAME, --render NAME
                           Render plugin name (default: FFMPEGRender)
   -o PATH, --output PATH
                           Output file path (default: result.mp4)
   -l EXPRESSIONS, --link EXPRESSIONS
                           Plugins to link (default: FaceDetect with default attributes)

                           Plugin Link Operators:
                              &	Link plugins with AND. cut the file according to the analysis result
                              |	Link plugins with OR. cut the file according to the analysis result
                              :	Apply effects to the plugin. It must be behind the plugin
                              ~	Analyze video without cutting file. It must be used to apply effects only

                           Plugin's Attributes Formatting:
                              {Plugin}({Attribute1}={Value1};{Attribute2}={Value2};...)
                              Do not use space in the parenthesis
                              e.g. FaceDetect(scale=1.2;minNeighbors=3)
   -s COUNT, --skip COUNT
                           Video Frames count to skip analyze (default: 0)


   ```

# Python Unit Test
[ut.py](ut.py) is unittest for OVI python APIs. </br>
See the following command for testing: </br>
   ```console
   $ python ut.py

   ...
   (Print the log here)
   ...

   Ran 28 tests in 11.176s

   OK
   ```
