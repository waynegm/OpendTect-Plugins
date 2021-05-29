# PyLops External Attribute Scripts
This a collection of [External Attribute](http://waynegm.github.io/OpendTect-Plugin-Docs/Attributes/ExternalAttrib/) scripts that
use the [PyLops](https://pylops.readthedocs.io/en/latest/index.html) linear operator library.

| PLUGIN | DESCRIPTION |
|--------|-------------|
| ex_prestack_modelling.py | Generate a synthetic pre-stack angle volume from well data |
| ex_poststack_relative_inversion.py | Calculate a post-stack relative seismic inversion |

All scripts require the PyLops Python package and it's dependencies. These can be installed in an active conda environment using:

```
conda install -c conda-forge pylops
```
or

```
pip install pylops
```
See the [PyLops documentation](https://pylops.readthedocs.io/en/latest/installation.html) for more information.