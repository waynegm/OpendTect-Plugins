# OpendTect Plugins Release Notes

## Release 7.0.7

### DataExtentHorizon
- fix 3D area not included after adding 2D lines to scope

### ConvexHull
- fix hull includes 3D even if not selected

## Release 7.0.6
- update to C++ 17
### EFDAttrib
- fix display of modes and spectral decomposition test panel
### External Attributes
- fix usage of OD_USER_PLUGIN_DIR for locating scripts
- add wmpy/AVO folder
    - add ex_avoig_angle3.py, ex_avoig_angle4.py and ex_avoig_angle5.py for intercept and gradient
 calculation from angle stacks
    - add ex_fatti3_angle3.py and ex_fatti3_angle4.py for fatti 3 term Rp, Rs and Rd calculation
    from angle stacks
### LocalAttrib
- fix display of spectral decomposition test panel
### Mistie
- fix to ensure analysis window is used for mistie calculation
### RSpecAttrib
- fix display of spectral decomposition test panel

## Release 7.0.5
### External Attributes
- add xa.params MinSamplesPerThread setting, default is 40
- ensure OpendTect application bin/python is in the PYTHONPATH of the attribute
 script environment
- if the environment variable OD_USER_PLUGIN_DIR exists add
OD_USER_PLUGIN_DIR/bin/python to the PYTHONPATH of the attribute script
environment
- if the environment variable OD_APPL_PLUGIN_DIR exists add
OD_APPL_PLUGIN_DIR/bin/python to the PYTHONPATH of the attribute script
environment
- move NAN handling in output from Python to C++ side
- add xa.params metadata (Survey, SurveyDiskLocation, InputNames)
- fix up help urls
- add Text UI input type
- add mode to File type, allowed values are "FileIn", "FileOut" and "Dir" for
selecting an input file, output file or folder

## Release 7.0.4.1
### External Attributes
- fix for Output UI field not being displayed

## Release 7.0.4
- Compile against OpendTect 7.0.4 with new compilers (GCC 10.5 and MSVS 2022)

## Release 7.0.3
- Compile against new versions dependencies for OpendTect 7.0.3

### External Attributes
- fix for UI fields not being displayed

### uiGeopackageExport
- fix for crash when exporting horizon data without a layer name
- add export of Z slices to geotiff

### GradientAttrib
- fix documentation link

### Mistie Analysis
- fix crash when no 2D horizons in survey

## Release 7.0.0
- Adapt to OpendTect 7.0 API changes

### uiGeopackageExport
- remove dependency on gdal

## Release 6.6.12
### uiGrid2D3DHorizon
- revamp ui to fix reported crashes

## Release 6.6.11
### uiGrid2D3DHorizon
- try and fix reported crashes

## Release 6.6.10
### AVOPolarAttrib
-  Replace Nans and Infs with Undefined

### EFDAttrib
- add EFD (Emprirical Fourier Decomposition) modes and spectral decomposition attributes

### External Attributes
- fix crash when loading attribute set with non existent interpreter or attribute script
- add UI error reporting when loading the script into the UI
- add ex_test_gradient.py to generate a synthetic gradient volume for testing AVO attributes
- fix various issues with scripts related to Python module updates
- move common functions into extlib.py
- add Jupyter notebook to demo/test extlib.py functions

### Mistie Analysis
- fix crash when estimating misties for depth surveys

### wmodpy - OpendTect Python Bindings
A decision was made to discontinue development of these within WMPlugins in favour of moving the code
into the dGB managed OpendTect open source code repository system on Github. The new repository will be
called "odpybind".

## Release 6.6.9
### AVOPolarAttrib
-  Fix attribute calculation on time slices and horizons

### GradientAttrib
-  Fix attribute calculation on time slices and horizons

### MLVFilterAttrib
-  Fix attribute calculation on time slices and horizons

### RSpecAttrib
-  Add UI option to display time frequency panel for a user selected trace
-  Fix attribute calculation on time slices and horizons

### LocalAttrib
-  LTFAttrib - Add UI option to display time frequency panel for a user selected trace

### External Attributes
-  Fix z window sample size ex_vf_gradient3_dip.py, ex_vf_phase3_dip.py and ex_vector_filter_dip.py

### wmodpy - OpendTect Python Bindings
-  revamp notebook examples
-  add pytest test script

## Release 6.6.8
### External Attributes
- remove frame around data entry UI
- add new "params" format for attribute UI definition (legacy style still supported)
- add input file selection new style "param"
- allow "param" keys with spaces
- add Pylops/ex_prestack_modelling.py external attribute script
- add Pylops/ex_poststack_inversion.py external attribute script
- fix noise added by ex_addnoise.py
- replace NaN's in output with OpendTect undefined (1e30)
- fix for read/write errors for large data

### wmodpy - OpendTect Python Bindings
- improve Windows build support
- standardise python command naming
- add methods to python Wells object to get well log data
- add Horizon3D python object for working with OpendTect 3D horizons (names, info and z values)
- add Horizon2D python object for working with OpendTect 2D horizons (names only)
- remove dependency on GeoPandas
- add methods to generate GeoJSON Feature Collections

## Release 6.6.7
### uiWMTools
- Ensure DataExtentHorizons conforms to survey definition

### uiGrid2D3DHorizon
- Ensure BoundingBox and ConvexHull horizon extents conform to survey definition

### OpendTect Python Bindings
- First release of some experimental python bindings for accessing OpendTect data

## Release 6.6.6
- Fix some depreciated OpendTect API calls

### Mistie
- Add calculation of Z misties from horizon interpretation
- Add tool to apply Z corrections to horizon interpretation

## Release 6.6.5
- Rebuild to accommodate API changes in OpendTect 6.6beta9

## Release 6.6.4
- Rebuild to accommodate API changes in OpendTect 6.6beta8

### Mistie
- Fix bug when no correlation quality filter set
- If filter is set show in filter dialog
- Add option to calculate only Z misties

## Release 6.6.3
- Rebuild to accommodate API changes in OpendTect 6.6beta5

## Release 6.6.2
- Use plugin version in PluginInfo field

### Mistie
- Add filter by correlation quality

### uiGrid2D3DHorizon
- Don't write number of fault polygons to par file if no faults selected

### External Attributes
- Prefer %OD_USER_PLUGIN_DIR%/bin/python/wmpy ahead of "OpendTect Install Folder"/bin/python/wmpy for location of external attribute scripts
- Fix for spaces in python environment path on Windows

### External Attribute Scripts
- Update ex_spatial_filter_circular and ex_spatial_filter_rectangular for change in scipy dependency
- Add Misc/ex_angle_stacks_3_to_AVOIG.py and Misc/ex_angle_stacks_5_to_AVOIG.py
- Misc/ex_correlation.py - clip coreelation values to range 0-1

## Release 6.6.1
- Make PluginInfo fields consistent across all plugins
- Update help keys

### External Attributes
- If OD_EX_DIR environment variable not set look in some other sensible places for external attribute scripts such as:
    - "OpendTect Install Folder"/bin/python/wmpy
    - %OD_USER_PLUGIN_DIR%/bin/python/wmpy
- Set default Python interpreter using the OpendTect Python settings
- Use a table to display the UI fields
- Put Python external attribute scripts into repository
- For Windows activate environment before starting Python script
- Add refresh/reset button beside interpreter field to force update against OpendTect Python settings

### uiGrid2D3DHorizon
- ensure faults are only used if fault selection field is displayed
- properly set polyline/polygon fault open/closed status and honor during fault masking

### uiWMTools
- fix crash on hitting cancel button of fault polygon generation dialog
- use fault stick intersections with horizon to generate fault polygons

## Release 6.6.0
- Adapt to OpendTect v6.6 API changes
- Fixes for handling survey change


## Release 6.4.12
- Place all attributes in wmPlugins group

### Mistie
- Fix build issues on Windows (declare classes as mExpClass)

## Release 6.4.11
### Mistie
- Fix crash during mistie estimation (add locks around variable update)

## Release 6.4.10
### uiWMTools
- Create plugin for miscellaneous OpendTect tools
- Move DataExtentHorizon tool into this plugin
- Add Convex Hull polygon generator tool
- Add fault polyline generator tool
- Add constant Z polyline generator tool

### DataExtentHorizon
- No longer a separate plugin, now included in uiWMTools plugin

### uiGrid2D3DHorizon
- fixed 3D survey data used even if 3D survey toggle is unchecked
- add Multi-level B-Spline gridding option
- revise implementation of Inverse Distance Weighted gridding, include option to just use specified number of nearest neighbour points
- revise thin plate spline option
- add polylines as a z value zource
- add convex hull and range scope options

### uiGeopackageExport plugin
- add support for session save/restore to GeopackageDisplay layers

## Release 6.4.8

### RSpecAttrib
- add option to perform time-frequency reassignment
- fix issues with offset gates

### LocalAttrib
- port code to support Madagascar style local attributes in OpendTect under both Windows and Linux
- LTFAttrib moved into this plugin
- fix issues with offset gates

### LTFAttrib
- plugin removed and code moved to LocalAttrib plugin

## Release 6.4.7

### AVOPolarAttrib
- move gradient intput field below intercept field to reduce width of UI

### GeopackageExport plugin
- add display of Geopackage lines and polylines on 3D horizons

### Mistie plugin
- include a 3D volume in the mistie analysis
- close dialogs when survey/project is changed

## Release 6.4.6

### Mistie plugin
- Add Mistie Analysis, Estimation and Correction computation tools

### MLVFilterAttrib
- Add trimmed mean output option

## Release 6.4.5

### Mistie plugin
- First release

### GeopackageExport plugin
- GH-36: add export of 3D horizons and attributes to Geotiff image
- Fix help key for Geotiff export dialog

## Release 6.4.4

### Grid2D3D Horizon plugin
- First release

### DataExtentHorizon plugin
- First release

## Release 6.4.3

### GeopackageExport plugin
- GH-26: fix to ensure default file extension for output file is "gpkg"
- GH-27: fix for failure to trap lack of Horizon output layer name on Windows
- GH-28: fix for survey box export toggle
- GH-29: fix detection of anchor point based x-y coordinate system - only support projected CRS
- GH-30: fix spatial reference system table issues by adding EPSG code to CRS information
- GH-31: explicitly initialise pointers to nullptr for Windows to fix crash on Horizon tab
- GH-32: add export of well tracks and markers
- GH-33: add option to export well marker depths in feet

## Release 6.4.2

###  GeopackageExport plugin
- First release

## Release 6.4.1
- Allow all attributes to have documentation displayed from the help icon in the attribute description editor [#23]
- Add attribute plugin skeleton code
- Add test of Eigen and ArrayFire libraries

### AVOPolarAttrib
- First release

## Release 6.2.4

### MLVFilterAttrib
- Fill missing traces to reduce edge effects [#20]

### ExternalAttrib
- Fill missing traces to reduce edge effects [#21]

### GradientAttrib
- Fill missing traces to reduce edge effects [#22]

## Release 6.0.4-1
### ExternalAttrib
- Add "Same" (boolean) option to StepOut parameter [#19]

## Release 6.0.4
### ExternalAttrib
- Capture external attribute script errors in OpendTect log file

## Release 6.0.2
#### ExternalAttrib
- Fix for multi-trace multi-attribute input - no longer restricted to input from same multi-attribute volume [#17]

## Release 6.0.1
#### ExternalAttrib
- Add Minimum option to ZSampMargin and StepOut parameters [#15]
- Fix for error message for multi-attribute input [#16]

## Release 6.0.0
### GradientAttrib
- New attribute for calculating inline, crossline or z gradient using various derivative operators

## Release 6.0.0pre7-1
#### ExternalAttrib
- Allow %ENVVAR% construct in interpreter field where ENVVAR is a system environment variable with the interpreter path [#12]
- Use environment variable, OD_EX_DIR, as the root of the folder structure containing the external attribute files [#13]
- Export ExternalAttrib and ExtProc classes to fix link errors during Windows build

## Release 6.0.0pre7
- Port all attributes for compatibility with OpendTect v6

#### ExternalAttrib
- Hide input ui controls on change of attribute [#11]

## Release 5.0.7
#### ExternalAttrib
- Support multi-threaded execution by starting multiple copies of the external application
- Support multi-attribute multi-trace input (upto 6 input attributes)
- Add "Parallel" (boolean) tag to the JSON parameter string to control single vs multi-threaded computation
- Add "Inputs" (Array of Strings) tag to the JSON parameter string to indicate the input attributes
- Add simple python examples of multi-attribute single trace input and multi-attribute multi-trace input

## Release 5.0.6-1
#### AVOAttrib
- Add example color tables

#### ExternalAttrib
- Add local polynomial approximation external attribute examples
- Add dipFactor and zFactor items to the SeismicInfo block exported to external attribute scripts (note changes to extattrib.py)
- Add "Symmetric" tag to the "ZSampMargin' JSON parameter string tag. The value of the tag is a boolean constant (True/False). If set True the UI will show a single entry box. Input sets the 'ZSampMargin.Value' array to [-Input, Input]. Default is 'ZSampMargin.Symmetric': False for which the UI displays entry boxes for both the window start and stop.
- Bugfix: ensure number of samples provided to external attribute includes ZSampMargin - fixes calculation failure on horizons and slices

#### MLVFilterttrib
- Add example color table for mlv_elements

## Release 5.0.6
#### AVOAttrib
- Fix typo in UI

#### ExternalAttrib
- Add "Help" tag to JSON parameter string. The value of the tag is a url string pointing to a help page for the attribute. This is optional.
- Add "Selection" tag to JSON parameter string. The value of the tag is an object with a 'Name' (string), 'Values' (array of strings) and 'Select' (number) tags. Displays a list box labeled 'Name' with options specified in 'Values' and default selection being item number 'Select'. This is optional.

#### RSpecAttrib
- Replace window parameter with Z gate. Window parameter set from Z gate width and attribute input taken at centre of Z gate. This allows attribute extraction offset from an horizon.

