# OpendTect v5+ Plugins 

This is the source code for a collection of plugins for the open source seismic interpretation system [OpendTect v5+](http://www.opendtect.org). All code is provided under the terms of the [GNU General Public License Version 3](./LICENSE.txt).

As of 24-Sep-2015 code development in the master branch is for plugin development for OpendTect 6. If you are interested in the latest source code for OpendTect 5 please refer to the v5-stable branch of this repository.

For details please see the [documentation](http://waynegm.github.io/OpendTect-Plugin-Docs/).

## Plugins

| PLUGIN | DESCRIPTION |
|--------|-------------|
| [AVOAttrib](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/AVOAttrib.html) | Six attributes derived from reflection intercept and gradient |
| [AVOPolarAttrib](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/AVOPolarAttrib.html) |Six attributes derived from AVO Polarization in the AVO Intercept-Gradient crossplot |
| [DataExtentHorizon](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/DataExtentHorizon.html) |Create a constant Z value 3D horizon which covers the extent of 2D and 3D seismic data. |
| [ExternalAttrib](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/ExternalAttrib.html) | Calculate attribute using an external process such as a python script |
| [GeopackageExport](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/GeopackageExport.html) | Export OpendTect data to a [GeoPackage](https://www.geopackage.org/) database |
| [GradientAttrib](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/GradientAttrib.html) | Calculate inline, crossline or z gradient using rotationally invariant operators |
| [Grid2D3DHorizon](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/Grid2D-3D.html) | Grid 2D and 3D horizon interpretation |
| [LTFAttrib](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/LTFAttrib.html) | Local time-frequency decomposition |
| [MLVFilter](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/MLVFilter.html) | Structure preserving filtering |
| [RSpecAttrib](http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/RSpecAttrib.html) | Recursive time-frequency decomposition |

## Building and Installation

* [Build from Source](http://waynegm.github.io/OpendTect-Plugin-Docs/installation/building-from-source.html)

* [Installation](http://waynegm.github.io/OpendTect-Plugin-Docs/installation/installation.html)
