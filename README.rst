=====================
OpendTect-v5-Plugins 
=====================
|

OpendTect v5+ Plugins
---------------------

This is a collection of plugins for the open source seismic interpretation system `OpendTect v5+ <http://www.opendtect.org>`_. All code is provided under the terms of the `GNU General Public License Version 3 <./LICENSE.rst>`_.

.. contents:: :local:
   :backlinks: top

Plugins
*******

Filtering
+++++++++

Frequency
+++++++++
* `Recursive time-frequency decomposition <doc/RSpectAttrib.html>`_

Binary Distribution
*******************


Building the Plugins
********************
These instructions are for Linux. The plugins may compile and run on Windows but that has not been tested.

#. Download the source for the attribute from the `GitHub repository <https://github.com/waynegm/OpendTect-5-plugins>`_.

#. Use the OpendTect installation manager to install the OpendTect developer packages and install any other packages required for compiling and building code for your operating environment as per the `OpendTect Programmer's Manual <http://www.opendtect.org/rel/doc/Programmer/>`_.

#. Start OpendTect

#. Select the Utilities-Tools-Create Plugin Devel. Env. menu item to create a development work folder (eg /home/user/ODWork).

#. Unzip the attribute source zip archive downloaded in step 1 in the development work folder. This will overwrite the CMakeLists.txt in the development work folder and add the plugin source folders to the plugin folder.

#. Optionally edit CMakeCache.txt in the development work folder and change Debug to Release.

#. Open a terminal, cd to the development work folder and type::

    cmake .
    make

#. This should create the binary files for each plugin, lib\*.so and libui\*.so, in the bin folder (eg in ODWork/bin/lux64/Release/) and four \*.alo files for each plugin in the root of the development work folder.

Installation Instructions
*************************
For installation there are 2 alternatives.

Sitewide Installation
+++++++++++++++++++++

#. Copy the lib\*.so and libui\*.so files to the appropriate platform sub folder in the OpendTect installation bin folder (eg copy to /opt/seismic/OpendTect/5.0/bin/lux64/Release).

#. Copy the \*.alo files to the appropriate platform sub folder in the OpendTect installation bin folder (eg copy to /opt/seismic/OpendTect/5.0/bin/lux64/Release).

Per-user Installation
+++++++++++++++++++++

On Linux it is also possible to install the plugin files in a users .od folder

#. Copy the lib\*.so and libui\*.so files to the appropriate platform sub folder in the users ~/.od/bin folder (eg copy to /home/user/.od/bin/lux64/Release).

#. Copy the \*.alo files to the appropriate platform sub folder in the users ~/.od/plugins folder (eg copy to /home/user/.od/plugins/lux64).

Common Issues
*************

Plugins not loading
+++++++++++++++++++
#. Try manually loading the plugin.

#. Check the OpendTect log file for error messages. 

libstdc++.so.6: version 'GLIBCXX_3.4.??' not found
++++++++++++++++++++++++++++++++++++++++++++++++++

This happens when the plugin is built with a gcc version different to the version used to build OpendTect. Solutions are: 

#. (Easy and seems to work ok but could break something) Rename the libstdc++.so.6 file in the OpendTect installation bin/lux64 folder to say old_libstdc++.so.6 and restart OpendTect.

#. (Hard) Install the same version of gcc that OpendTect was built with and rebuild the plugin.

#. (Hardest) Build OpendTect from source using your installed gcc.
