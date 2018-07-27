.. include:: ../common.rst

.. _float_upsampling_factor:

************************
Float upsampling factor:
************************

Omitting the potential artifacts discussed in previous chapters, the process of upsampling a signal by a factor
:math:`z | z \in \mathbb N^*` is rather simple. All it takes is adding the right number of zeros to the signal spectrum.

This being said, the *right number of zeros* might be hard to find (if at all) when :math:`z \in \mathbb R^{+*}`.
Assuming the input signal size is :math:`N`, and the user zooming factor is :math:`z_r, z_r \in \mathbb R^{+*}`, then
the padding spectrum size must be :math:`N.z_r`.

Chances are :math:`N.z_r \notin \mathbb N^{*}` so the input signal has to be padded up to a size
:math:`N' | N' > N, N'.z_r \in \mathbb N^{*}` assuming such a :math:`N'` exists.

If it does, then it is easy to figure out how this padding could imply overhead.

If it does not, and the spectrum is padded up to a size :math:`N_{up}` such that
:math:`N_{up} \in \mathbb N^* | N_{up} \approx N.z_r` then the zooming factor actually
applied is :math:`\overset{\sim}{z_r} = \frac{N_{up}}{N}`.

To make sure Sirius always applies the user required zooming factor, and not an approximation of it, Sirius requires the
user to set the input spacing, and the output one. Say the required zooming factor is :math:`z = 2.3333333333333335`
then the user must set input spacing to :math:`70` and output spacing to :math:`30` (no matter the unit here).

Sirius will then understand the zooming factor is :math:`7/3` and so will upsample the input data by a factor :math:`7`
before decimating the output to keep only :math:`1` pixel out of :math:`3`.

This process ensures the zooming factor is exactly the one required by the user. It shall be noted the decimation is a
safe process here and no aliasing will come from it. Indeed, the input spacing is :math:`70` (again, no matter the unit),
so decimating from :math:`10` to :math:`30` remains an aliasing free process since zooming from :math:`70` to :math:`10`
dit not add higher frequencies than the ones already contained in the original spectrum.

.. note:: If one has given Sirius a low-pass filter to control the way high frequencies are cut-off, Sirius assumes this
   :ref:`filter is sampled at the targeted resolution <SiriusFilterRules>` (:math:`7/3` here). Because the zooming
   factor will actually by :math:`7`, Sirius will firstly upsample the filter by a factor :math:`3`. This upsampling
   is obviously done using Fourier zero padding. This is equivalent to a sinc convolution which shall be the perfect
   way to upsample a continuous and rather smooth filter.


