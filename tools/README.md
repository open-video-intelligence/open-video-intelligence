# Tools
### ovi_plugins
   ```
   $ ovi_plugins
   Usage:
           ovi_plugins [Options] [Plugin Name]

   Options:
           --list-all                              show all available plugins
           --list-videodetectors                   show all available video detectors
           --list-audiodetectors                   show all available audio detectors
           --list-videoeffects                     show all available video ettects
           --list-audioeffects                     show all available audio ettects
           --list-renders                          show all available renders
           -v, --version                           version
           -h, --help                              help
   ```

### ovi_session
   ```
   $ ovi_session
   Usage:
           ovi_session [Session Launch Options] [Application Options]

   Session Launch Options:
           -i                      Media path to analyze and edit (mandatory)
           -r                      Render Plugin and its attrubutes (mandatory)
           -l                      Plugins to link (mandatory)
           -skv                    Video Frames count to skip analyze
           -v, -verbose            Logging level. Default 6. trace:0 debug:1 info:2 warn:3 error:4 critical:5 off:6
           -q                      Quit program

           Plugin Link Operators:
                   &               Link plugins with AND. cut the file according to the analysis result
                   |               Link plugins with OR. cut the file according to the analysis result

   Application Options:
           -version                Version
           -h, -help               help

   Example:
   ovi_session -i ./movie.mp4 -r FFMPEGRender'(path=./result.mp4)' -skv 3 -l 'AudioDetect(dbThreshold=50)' -verbose 4
   ovi_session -i ./movie.mp4 -skv 3 -l 'AudioDetect(dbThreshold=50)' -r OTIORender'(path=./result.otio)'
   ```

### py_import_tester
   ```
   $ ./py_import_tester
   Usage:
     ./py_import_tester {module_name}
   Example:
   ./py_import_tester audioDetect
   ```
