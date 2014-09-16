===========
RSpecAttrib
===========
|

RSpec Attribute
---------------

Recursive time-frequency decomposition attribute for the open source seismic interpretation platform `OpendTect v5+ <http://www.opendtect.org/>`_.

.. contents:: :local:
   :backlinks: top

Description
***********

This plugin can be used as an alternative to the `OpendTect FFT spectral decomposition attribute <http://opendtect.org/rel/doc/User/base/appendix_spectral-decomposition.htm>`_.

It does spectral decomposition using `Nilsen's (2007) <https://bora.uib.no/bitstream/handle/1956/3036/42162315.pdf?sequence=1>`_ time-frequency analysis algorithm. This algortihm is a recursive filter approximation to a special case of the short time fourier transform (STFT). 

The primary advantage of this plugin over the standard OpendTect FFT spectral decomposition is that can be evaluated significantly faster. As an example for a 2000 samples per trace dataset under Linux on an Intel Core i5 this attribute can generate a single frequency cube at 4000 traces per second which is considerably faster than the 144 traces per second of the OpendTect FFT spectral decomposition attribute. The processing speed advantage of this attribute reduces as the number of frequencies output increases but it still remains substantially faster for output of up to 30 frequencies in this test case.

Examples
********

.. figure:: RSpecAttrib_1.jpg
   :width: 100%
   :align: center

   OpendTect FFT Spectral Decomposition (30Hz +/-28ms window)

.. figure:: RSpecAttrib_2.jpg
   :width: 100%
   :align: center
   
   Recursive time-frequency attribute (30Hz 56ms effective window)

.. figure:: RSpecAttrib_3.jpg
   :width: 100%
   :align: center
   
   Crossplot of RSpecAttrib vs FFT Spectral Decomposition

The output of the RSpec attribute (rfreq30) is visually identical and also highly correlated to the OpendTect FFT spectral decomposition (sdfreq30) as shown in the following crossplot of the two attributes.   

Input Parameters
****************

This attribute has 4 parameters:

* Input Volume
 
  The attribute volume to be analysed.

* Effective window

  This determines the time resolution. Recommend setting it equal to or less than the FFT window length you would used for the standard OpendTect FFT spectral decomposition.

* Output frequency

  When displaying the attribute in the tree this is the frequency slice that will be generated

* Step

  This determines the set of frequencies that can be chosen when generating a frequency volume.
  
.. image:: RSpecAttrib_input.jpg
   :width: 100%


