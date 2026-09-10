# OpendTect Plugins

This is the source code for a collection of plugins for the open source seismic interpretation system [OpendTect](http://www.opendtect.org). All code is provided under the terms of the [GNU General Public License Version 3](./LICENSE.txt).

<p align="center">
<a href="https://www.youtube.com/watch?v=nXIyODFinjw">
<img src="https://img.youtube.com/vi/nXIyODFinjw/0.jpg" width="50%" >
 </a>
</p>

Code development in the main branch is for OpendTect 2026. For earlier versions of OpendTect see the appropriate branch.

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

The [Pixi](https://pixi.prefix.dev/latest/) package manager can be used to obtain all required dependencies and build OpendTect plugins for OpendTect 2026. All build files are written to the **build-pixi** folder. The built plugins are written to the **inst/2026** folder.

**Note for Windows**: you will need to install msvc2022 64 bit (>= v17.8.4 - free community edition is sufficient).

**Note for macOS**: the supplied **pixi.toml** file is only set up for Windows and Linux. Extending it to **macOS** should be relatively simple if you have access to that hardware.

#### Install Pixi
<details>
  <summary><b>Linux and macOS</b></summary>

```bash
  curl -fsSL https://pixi.sh/install.sh | sh
```
</details>
<details>
  <summary><b>Windows</b></summary>

  <kbd>[**Download installer**](https://github.com/prefix-dev/pixi/releases/latest/download/pixi-x86_64-pc-windows-msvc.msi)</kbd>

  Or run:

```powershell
powershell -ExecutionPolicy ByPass -c "irm -useb https://pixi.sh/install.ps1 | iex"
```
</details>

#### Git Clone the OpendTect-Plugins Main Branch
Assuming **git** is installed, open a shell window and enter:
```bash
git clone -b main --depth 1 https://github.com/waynegm/OpendTect-Plugins.git
```
This will create a folder called **OpendTect-Plugins** with the source code.
#### Configure
To configure the build, open a shell:
- cd into the source code folder:
```bash
 cd OpendTect-Plugins
 ```
- Edit **pixi.toml** and change the **activation.env** table so that the `OpendTect_DIR` variable points at your OpendTect 2026 installation.
- In a shell enter:
```bash
pixi run config
```
#### Build
To build, open a shell, change to the source code folder (unless already there) and enter:
```bash
pixi run build
```
#### Run
To run OpendTect with the plugins loaded, open a shell, change to the source code folder (unless already there) and enter:
```bash
pixi run release
```
