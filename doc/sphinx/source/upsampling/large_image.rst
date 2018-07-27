.. include:: ../common.rst

.. _large_data:

***********************************
Tile processing of the large image:
***********************************

The implicit edges discontinuities and their associated artifacts
=================================================================

In order to process satellite images, Sirius divides it in tiles (blocks). Those tiles are then executed independently
from each other.

Though there is no particular reason for a natural signal / image to be periodic, Fourier Transform implicitly
assumes it is. This leaves us with a periodized signal / image with probably strong discontinuities across its edges.
This involves artifacts in the frequency domain that penalises the resampling process.

The tiling process that divides the images with arbitrary borders can only emphasized those artifacts.

Below is an example with a triangular 1D signal (periodic) and a linear signal (half the triangle) with strong edges
discontinuities. Both signals are zero padded :

.. ipython:: python
    :okwarning:

    n=50
    z=2
    tri = create_1D_triangle_signal(n)
    tri_zpd = zoom_freq_zpd( tri, zoom_factor=z)
    plt.plot(tri_zpd, label='triangular signal upsampled by Fourier zero padding');
    @savefig triangular_zpd.png
    plt.plot(np.arange(0,n*z,z),tri,'.',label='orignal triangular signal samples');plt.legend()
    plt.close()

    halftri = create_1D_ramp(n)
    halftri_zpd = zoom_freq_zpd( halftri, zoom_factor=z)
    plt.plot(halftri_zpd, label='ramp signal upsampled by Fourier zero padding');
    @savefig ramp_zpd.png
    plt.plot(np.arange(0,n*z,z), halftri,'.',label='orignal ramp signal samples');plt.legend()


Those artifacts come from the fact that only a finite number of terms of the Fourier series can
actually be used for numerical processing. Then, even though their limits do not overshoot the signal and will allow for
a discontinuous signal to be approximated by infinite series of continuous sinusoids, every partial Fourier series can
overshoot it. And it can actually be shown that the overshoot near a discontinuity will always be about 9%
(|GibbsWilbraham|).


The possible solutions
======================

Margin the original data
------------------------

One way to get rid of these artifacts is to marge the original data by propagating the first and the last value of the signal,
before and after it.

.. ipython:: python

    marge_factor=4
    halftri_zpd_spatially = zero_pad(halftri, marge_factor)
    halftri_zpd_spatially[halftri_zpd_spatially.size/2+halftri.size/2:halftri_zpd_spatially.size] = halftri[-1]
    halftri_zpd_spatially_zpd = zoom_freq_zpd( halftri_zpd_spatially, zoom_factor=z)

    plt.title('It is possible to marge enough the data to \'move\' the \n artifacts far away from the signal samples');
    plt.plot(halftri_zpd, label='ramp signal upsampled by Fourier zero padding');
    plt.plot(halftri_zpd_spatially_zpd[halftri.size*(marge_factor-1)*z/2:halftri.size*(marge_factor-1)*z/2+halftri_zpd.size], label='ramp signal upsampled by Fourier zpd after adding spatial margin');
    @savefig Margin_before_fzpd.png
    plt.plot(np.arange(0,n*z,z), halftri,'.',label='orignal ramp signal samples');plt.legend()


Note that the margin factor must be high enough :

.. ipython:: python

    marge_factor=2
    halftri_zpd_spatially_mf2 = zero_pad(halftri, marge_factor)
    halftri_zpd_spatially_mf2[halftri_zpd_spatially_mf2.size/2+halftri.size/2:halftri_zpd_spatially_mf2.size] = halftri[-1]
    halftri_zpd_spatially_mf2_zpd = zoom_freq_zpd( halftri_zpd_spatially_mf2, zoom_factor=z)

    plt.suptitle('The margin factor must be high enough');
    plt.subplot(211)
    plt.plot(halftri_zpd_spatially_zpd, label='ramp signal spatially marged (x4) before Fourier zpd');plt.legend();
    plt.plot(np.arange(100, 100+halftri_zpd_spatially_mf2_zpd.size), halftri_zpd_spatially_mf2_zpd, label='ramp signal spatially marged (x2) before Fourier zpd');plt.legend();
    plt.subplot(212)
    plt.plot(halftri_zpd_spatially_zpd, label='ramp signal spatially marged (x4) before Fourier zpd');plt.legend();
    plt.plot(np.arange(100, 100+halftri_zpd_spatially_mf2_zpd.size), halftri_zpd_spatially_mf2_zpd, label='ramp signal spatially marged (x2) before Fourier zpd');plt.legend();
    plt.xlim(130,160)
    @savefig Margin_factor_before_fzpd.png
    plt.ylim(-1,6)


Mirroring the original data
---------------------------

One other way to look at it is to simply suppress the discontinuity by mirroring the signal.

.. ipython:: python

    marge_factor=2
    halftri_mirrored = mirror(halftri, margin_factor=marge_factor)
    halftri_mirrored_zpd = zoom_freq_zpd( halftri_mirrored, zoom_factor=z)

    plt.title('The ramp signal mirrored is periodic');
    @savefig Mirrored_ramp.png
    plt.plot(halftri_mirrored, label='mirrored ramp signal');
    plt.close()

    plt.title('The mirroring makes the signal periodic and avoids artifacts');
    plt.plot(halftri_zpd, label='ramp signal upsampled by Fourier zero padding');
    plt.plot(halftri_mirrored_zpd[halftri.size*(marge_factor-1)*z/2:halftri.size*(marge_factor-1)*z/2+halftri_zpd.size], label='ramp signal upsampled by Fourier zpd after being mirrored');
    @savefig Mirroring_before_fzpd.png
    plt.plot(np.arange(0,n*z,z), halftri,'.',label='orignal ramp signal samples');plt.legend()


The periodic + smooth decomposition
-----------------------------------

The two previous methods implies adding 'fake' samples to the input data, which might be very time consuming if
done for every tile of a satellite image. The solution proposed by |ps_| however can deal with discontinuous signal with
only a little overhead. It consists in decomposing the input signal into two sub-signals, one being periodic, and
the other being smooth. The periodic signal can then be upsampled in Fourier domain without concerns about the artifacts
discussed in this chapter. The smooth one, built from the differences located at the data edges, is upsampled spatially
with a simple bilinear filter.

.. ipython:: python

    n = halftri.size
    #1 : Compute smooth part
    x = np.arange(n)
    s = np.zeros(halftri.shape)
    s = ((halftri[n-1] - halftri[0])/np.float(n)) * (x - (n-1)/2)

    # 2 : Compute periodic part
    p = halftri - s

    # 3 : Zero padds the periodic part
    tf_p = np.fft.fft(p)
    z_tf_p = fft1D_zero_pad(tf_p, zoom_factor=2)

    # 4 : Interpolate the smooth part spatially with a blinear interpolator
    bln_interp_s = interp1d(s, zoom_factor=2)

    # 5 : Merged p & s zoomed and FT-1
    halftri_p_fzpd_and_s_bln = np.fft.ifft(z_tf_p) + bln_interp_s

    # plot the decomposition
    plt.suptitle('Periodic and smooth decomposition : spatial domain')
    plt.subplot(411)
    plt.plot(halftri, label='Input signal')
    plt.legend()
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
    plt.subplot(412)
    plt.plot(p, label='periodic')
    plt.legend()
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
    plt.subplot(413)
    plt.plot(s, label='smooth')
    plt.legend()
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
    plt.subplot(414)
    plt.plot(p+s, label='periodic + smooth')
    plt.legend()
    @savefig p_plus_s_triangle_decomposition.png
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
    plt.close()

    # plot the result
    plt.title('Using the p+s decomposition avoids artifacts.\n This solution adds little overhead');
    plt.plot(halftri_zpd, label='ramp signal upsampled by Fourier zero padding');
    plt.plot(halftri_p_fzpd_and_s_bln, label='ramp signal upsampled by Fourier zero padding with p+s decomposition');
    @savefig p_plus_s_before_fzpd.png
    plt.plot(np.arange(0,n*z,z), halftri,'.',label='orignal ramp signal samples');plt.legend()


Conclusion : Sirius uses the p+s decomposition
==============================================

Because it adds little overhead the p+s decomposition is the solution processed by Sirius to Fourier Transform each
tile.

Below is a little comparison of the three methods presented above :

.. ipython:: python

    halftri_analytically_zoomed = create_1D_ramp(50, step=2)[0:50*2-1]
    plt.suptitle('Interpolation error against the input signal analytically upsampled')
    plt.subplot(311)
    marge_factor = 2
    plt.plot(halftri_analytically_zoomed-halftri_zpd_spatially_mf2_zpd[halftri.size*(marge_factor-1)*z/2:halftri.size*(marge_factor-1)*z/2+halftri_zpd.size-1], label='For Fourier zero padding with marged (x2) signal')
    plt.legend()
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
    plt.subplot(312)
    plt.plot(halftri_analytically_zoomed-halftri_mirrored_zpd[halftri.size*(marge_factor-1)*z/2:halftri.size*(marge_factor-1)*z/2+halftri_zpd.size-1], label='For Fourier zero padding with mirrored (x2) signal')
    plt.legend()
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)
    plt.subplot(313)
    plt.plot(halftri_analytically_zoomed-halftri_p_fzpd_and_s_bln[0:50*2-1], label='For Fourier zero padding with p+s decomposition')
    plt.legend()
    @savefig comparing_solution_to_FT_tiles.png
    plt.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)


.. note:: The use of an apodized low-pass filter such as the :ref:`lanczos one <window_lanczos>` can lesser the artifacts since it smooths the high frequencies.
   However, this is only considered a side effect. First because Sirius shall not rely on the user low-pass filter to avoid those artifacts. But also because
   such a low-pass filter might blur the data where the periodic plus smooth decomposition will not. This is why the use of low-pass filter shall only be
   considered to deal with the signal discontinuities, not its non periodicity.
