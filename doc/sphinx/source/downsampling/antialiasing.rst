.. include:: ../common.rst

.. _antialiasing:

*************
Anti-Aliasing
*************

Because downsampling is the process of reducing a signal sampling frequency, it can cause severe aliasing by breaking
the Shannon criteria.

Hence, it is highly advised to provide Sirius a low-pass filter to apply to the original signal spectrum before
downsampling it. Once the high frequencies are filtered, Sirius can safely decimate the original signal.

Such a low-pass filter must cut-off the frequencies :math:`f, |f| > |\frac{f_e}{2*z_d}|` with :math:`z_d` the
downsampling factor.

Here is an example based on a high frequency cosine to emphasize the potential aliasing when downsampling :

.. ipython:: python
    :okwarning:

    # cosine signal based on two cosine, one with high frequency
    x,cos4 = create_1D_cosine( 100, 4)
    x,cos20 = create_1D_cosine( 100, 20)
    s = cos4+cos20
    fft_s = np.fft.fftshift(np.fft.fft(s))
    plt.suptitle('Sum of two cosine, one with high frequency.\n Shannon criteria is met here');
    plt.subplot(121);
    plt.plot(x, s, label='The signal');plt.legend();
    plt.subplot(122);
    @savefig high_freq_cosine.png
    plt.plot(np.fft.fftshift(np.fft.fftfreq(fft_s.size))*fft_s.size, fft_s, label='The signal FT');plt.legend();

.. ipython:: python

    # decimating the signal without caution
    s_decim = s[0::2]
    plt.close()
    plt.suptitle('Downsampling without filtering high frequencies. \nShannon criteria is not met');
    plt.subplot(121);
    plt.plot(x,s,label='Original signal');
    plt.plot(x[::2],s_decim,'--', label='Decimated signal');plt.legend()
    plt.subplot(122);
    @savefig hig_freq_cosine_decimated_aliasing.png
    plt.plot(np.fft.fftshift(np.fft.fftfreq(s_decim.size))*s_decim.size, np.fft.fftshift(np.fft.fft(s_decim)), label='Aliased decimated signal FT');plt.legend()

.. ipython:: python

    # decimating after low-pass filtering
    x_, lanczos_kernel_up2 = create_1D_lanczos( 50, a=5, step=2)
    lanczos_kernel_up2[0::2]/=(np.sum(lanczos_kernel_up2[0::2])/0.5)
    lanczos_kernel_up2[1::2]/=(np.sum(lanczos_kernel_up2[1::2])/0.5)
    fft_lanczos = np.fft.fftshift(np.fft.fft(np.fft.ifftshift(lanczos_kernel_up2[0:-1])))
    plt.close()
    plt.title('Low-pass filtering of the original signal FT.\n Frequencies above fe/4 are filtered')
    plt.plot(np.fft.fftshift(np.fft.fftfreq(fft_s.size))*fft_s.size, fft_lanczos, label='Low-pass filter (lanczos FT)');plt.legend()
    plt.plot(np.fft.fftshift(np.fft.fftfreq(fft_s.size))*fft_s.size, fft_s * fft_lanczos, label='Original signal FT filtered'); plt.legend()
    @savefig high_cosine_ft_filtered.png
    plt.plot(np.fft.fftshift(np.fft.fftfreq(fft_s.size))*fft_s.size, fft_s, '--', label='Original signal FT');plt.legend()

.. ipython:: python

    s_filtered = np.fft.ifft(np.fft.ifftshift(fft_s * fft_lanczos))
    s_filtered_decim = s_filtered[0::2]
    plt.close()
    plt.title('Decimation with no aliasing.')
    plt.plot(x, s_filtered, label='Original signal filtered');
    @savefig hig_freq_cosine_decimated_no_aliasing.png
    plt.plot(x[::2],s_filtered_decim,'--', label='Decimated signal');plt.legend()

.. ipython:: python

    plt.close()
    plt.title('Comparing with and without aliasing.')
    plt.plot(x,s,'--',label='Original signal');
    plt.plot(x[::2],s_filtered_decim,label='Orignal signal filtered before decimation');
    @savefig high_freq_cosine_decim_compared.png
    plt.plot(x[::2],s_decim, label='Orignal signal decimated without low-pass filtering'); plt.legend()

