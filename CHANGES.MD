# OpendTect Plugins Release Notes

## Release 6.4.2.1

### GeopackageExport plugin
- GH-26: fix to ensure default file extension for output file is "gpkg"
- GH-27: fix for failure to trap lack of Horizon output layer name on Windows
- GH-28: fix for survey box export toggle
- GH-29: fix detection of anchor point based x-y coordinate system - only support projected CRS
 
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

