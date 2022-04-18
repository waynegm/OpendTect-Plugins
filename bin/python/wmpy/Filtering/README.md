# Filtering External Attribute Scripts
This a collection of [External Attribute](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/externalattrib/) scripts for filtering seismic
data and attributes.

| PLUGIN | DESCRIPTION |
|--------|-------------|
| [ex_lpa_smooth.py](http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/LPA_Smooth) | Structure preserving filter by local polynomial approximation |
| [ex_spatial_filter_circular.py](http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/Spatial_Filter_Circular) | Apply lowpass, highpass, bandpass or band reject spatial (k-k) filter with circular symmetry |
| [ex_spatial_filter_rectangular.py](http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/Spatial_Filter_Rectangular) | Apply lowpass, highpass, bandpass or band reject spatial (k-k) filter with rectangular symmetry or by setting a stepout of 0 in one direction apply a 1D spatiall filter.|
| [ex_vector_filter_dip.py](http://waynegm.github.io/OpendTect-Plugin-Docs/External_Attributes/Vector_Filters) | Applies a vector median filter to Cross-line and Inline dips. Outputs filtered orientation (inline dip, crossline dip, true dip and dip azimuth) |

Some scripts require the numba Python package.

