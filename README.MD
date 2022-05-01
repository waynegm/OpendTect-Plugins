# OpendTect Plugins 

This is the source code for a collection of plugins for the open source seismic interpretation system [OpendTect](http://www.opendtect.org). All code is provided under the terms of the [GNU General Public License Version 3](./LICENSE.txt).

<p align="center">
<a href="https://www.youtube.com/watch?v=nXIyODFinjw">
<img src="https://img.youtube.com/vi/nXIyODFinjw/0.jpg" width="50%" >
 </a>
</p>

As of 5-Jan-2020 code development in the master branch is for OpendTect 6.6. If you are interested in the latest source code for:

-  OpendTect v6.4 please refer to the 6.4 branch
-  OpendTect v5 please refer to the v5-stable branch

For details please see the [documentation](http://waynegm.github.io/WMPlugin-Docs/) or click one of the links below.

## Plugins

| PLUGIN | DESCRIPTION |
|--------|-------------|
| [AVOAttrib](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/avoattrib) | Six attributes derived from reflection intercept and gradient |
| [AVOPolarAttrib](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/avopolarattrib) |Six attributes derived from AVO Polarization in the AVO Intercept-Gradient crossplot |
| [ExternalAttrib](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/externalattrib) | Calculate attribute using an external process such as a python script |
| [GeopackageExport](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/geopackageexport) | Export OpendTect data to a [GeoPackage](https://www.geopackage.org/) database |
| [GradientAttrib](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/gradientattrib) | Calculate inline, crossline or z gradient using rotationally invariant operators |
| [Grid2D3DHorizon](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/grid2d-3d) | Grid 2D and 3D horizon interpretation |
| [LocalAttrib](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/localattrib) | Implementation of Fomel's local attribute concept, only Local time-frequency decomposition for now |
| [Mistie](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/mistie) | Estimate/edit/apply Z shift, phase rotation and amplitude corrections to 2D and 3D seismic data |
| [MLVFilter](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/mlvfilter) | Structure preserving filtering |
| [RSpecAttrib](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/rspecattrib) | Recursive time-frequency decomposition |
| [WMTools](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/wmtools) | Miscellaneous tools for OpendTect, including generation of data extent horizon, convex hull polygons |

## Building and Installation

* [Build from Source](http://waynegm.github.io/WMPlugin-Docs/docs/gettingstarted/building-from-source/)

* [Installation](http://waynegm.github.io/WMPlugin-Docs/docs/gettingstarted/installation/)
