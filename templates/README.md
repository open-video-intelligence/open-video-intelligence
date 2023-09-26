# Plugin Template Generator for plugin developers
Generate buildable plugin templates with Open Video Intelligence.</br>
Run the following command:</br>
   ```console
   $ gen
   ```

For detailed usage, check the help (-h, or --help option).</br>
   ```console
   $ gen -h

   [Plugin Template Generator]

   (HOW TO)
      gen [plugin type] [plugin name] [language type]
         plugin type :
            -v, --video : video detector
            -a, --audio : audio detector

         plugin name (-n, --name) :
            Plugin name(Pascal case)

         language type (-l, --lang) :
            c : c/c++
            py : python

      e.g.
         $ gen -v -n FooBarPy -l py
         $ gen --audio --name FooBarC --lang c

   ```

# Generated plugin template
A new plugin is created under the plugins directory as follows:</br>
```bash
├── {plugin_name}
│   ├── models
│   ├── srcs
│   ├── CMakeLists.txt
│   └── {plugin_name}.cpp, or {plugin_name}.py
├── __init__.py
├── CMakeLists.txt # Add new plugin directory automatically here for building
└── README.md
```
The purpose of automatically created files and directories is as follows:</br>
 - models : Directory for data files used by plugin</br>
 - srcs : If you need additional files, use this directory.</br>
   In particular, in the case of Python, all additional files other than automatically generated plugin files must be added under this folder.
 - CMakeLists.txt : It is written to build and install only for auto-generated files and folders.</br>
   Therefore, additionally created files must be included here.</br>
 - .py / .cpp : Functions are automatically defined according to the OVI plugin interface.</br>
   For details, please refer to the comments of each function.