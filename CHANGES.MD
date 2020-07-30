# OpendTect Plugins Release Notes

## Release 6.4.15

### Mistie
- Fix bug when no correlation quality filter set
- If filter is set show in filter dialog
- Add option to calculate only Z misties


## Release 6.4.14

### uiGrid2D3DHorizon
- Don't write number of fault polygons to par file if no faults selected

### Mistie
- Add filter by correlation quality

### External Attribute Scripts
- Add Misc/ex_angle_stacks_3_to_AVOIG.py and Misc/ex_angle_stacks_5_to_AVOIG.py
- Misc/ex_correlation.py - clip coreelation values to range 0-1

### External Attribute Plugin
- Prefer %OD_USER_PLUGIN_DIR%/bin/python/wmpy ahead of "OpendTect Install Folder"/bin/python/wmpy for location of external attribute scripts
- Fix for spaces in python environment path on Windows

## Release 6.4.13
- Make PluginInfo fields consistent across all plugins
- Use plugin version in PluginInfo field
- Update help keys

### External Attribute
- If OD_EX_DIR environment variable not set look in some other sensible places for external attribute scripts such as:
    - "OpendTect Install Folder"/bin/python/wmpy
    - %OD_USER_PLUGIN_DIR%/bin/python/wmpy
- Set default Python interpreter using the OpendTect Python settings
- Use a table to display the UI fields
- For Windows activate environment before starting Python script
- Add refresh/reset button beside interpreter field to force update against OpendTect Python settings

### External Attribute Scripts
- Put Python external attribute scripts into repository
- Update ex_spatial_filter_circular and ex_spatial_filter_rectangular for change in scipy dependency

### uiGrid2D3DHorizon
- ensure faults are only used if fault selection field is displayed
- properly set polyline/polygon fault open/closed status and honor during fault masking

### uiWMTools
- fix crash on hitting cancel button of fault polygon generation dialog
- use fault stick intersections with horizon to generate fault polygons

## Release 6.4.12
- Place all attributes in wmPlugins group
- Fixes for handling survey change

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

