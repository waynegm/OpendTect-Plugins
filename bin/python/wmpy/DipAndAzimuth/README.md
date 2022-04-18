# Dip and Azimuth External Attribute Scripts
This a collection of [External Attribute](http://waynegm.github.io/WMPlugin-Docs/docs/plugins/externalattrib/) scripts for estimating
orientation, ie dip or dip azimuth.

| PLUGIN | DESCRIPTION |
|--------|-------------|
| [ex_dip.py] | True (Polar) dip and Dip Azimuth from inline and crossline dip |
| [ex_gradient_dip.py] | Orientation from inline, crossline and z gradients |
| [ex_vf_gradient3_dip.py] | Orientation using the vector filtered gradient with gradients by Kroon's 3 point operator |
| [ex_gradient3_st_dip.py] | Orientation using the gradient structure tensor with gradients by Kroon's 3 point operator|
| [ex_gradient5_st_dip.py] | Orientation using the gradient structure tensor with gradients by Farid's 5 point operator|
| [ex_phase3_dip.py] | Orientation using the 3D complex trace phase with gradients by Kroon's 3 point operator |
| [ex_vf_phase3_dip.py] | Orientation using the vector filtered 3D complex trace phase with gradients by Kroon's 3 point operator |
| [ex_weighted_phase3_st_dip.py] | Orientation using the 3D complex trace envelope weighted phase structure tensor with gradients by Kroon's 3 point operator |

All scripts will estimate at least the following attributes:

| OUTPUT     | DESCRIPTION
|------------|----------------|
| Inline Dip | Event dip observed on a crossline in microseconds per metre for time surveys and millimetres per metre for depth surveys. Output can be positive or negative with the convention that events dipping towards larger inline numbers producing positive dips. |
| Crossline Dip | Event dip observed on an inline in microseconds per metre for time surveys and millimetres per metre for depth surveys. Output can be positive or negative with the convention that events dipping towards larger crossline numbers producing positive dips. |
| True Dip | Event dip in microseconds per metre for time surveys and millimetres per metre for depth surveys. Output is always positive. |
| Dip Azimuth | Azimuth of the True Dip direction relative to the survey orientation. Output ranges from -180 to 180 degrees. Positive azimuth is defined from the inline in the direction of increasing crossline numbers. Azimuth = 0 indicates that the dip is dipping in the direction of increasing crossline numbers. Azimuth = 90 indicates that the dip is dipping in the direction of increasing inline numbers. |

Some scripts may offer additional outputs such as a measure of event coherency or planarity.

All of the scripts require the numba Python package.

For further details please see the [documentation](http://waynegm.github.io/WMPlugin-Docs/docs/externalattributes/dipazimuth/).
