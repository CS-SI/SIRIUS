import numpy as np
import matplotlib.pyplot as plt


def create_1D_rect(n):
    rect = np.zeros(n)
    rect[n/4:3*n/4] = 255
    return rect


def sinc(x, dtype=np.float32):
    no_matter_value = 30
    N = x.size
    sincx = no_matter_value*np.ones(N, dtype=dtype)
    sincx[0:N/2] = np.sin(x[0:N/2]*np.pi)/(x[0:N/2]*np.pi)
    sincx[N/2] = 1
    sincx[N/2+1:sincx.size] = np.sin(x[N/2+1:N]*np.pi)/(x[N/2+1:N]*np.pi)
    return sincx


def create_1D_sinc(n, step=1):	
    x = np.linspace(-n/2, n/2, n*step+1)
    return x, sinc(x)


def create_1D_lanczos(n, a=3, step=1):
    x, sincx = create_1D_sinc(n, step)
    sincx_a = sinc(x/a)
    rect_lanczos = np.zeros(sincx_a.size)
    rect_lanczos[np.where((-a<x)*(x<a))] = 1
    return x,sincx*sincx_a*rect_lanczos


def create_1D_mire(n):
    low = np.zeros((n/2))
    high = 255 * np.ones((n-n/2))
    return np.concatenate((low,high))


def create_1D_triangle_signal(n=4,step=1):
    if step==1:
        return np.concatenate((np.arange(0,n/2+1),np.arange(n/2-1,(n+1)%2-1,-1)))
    else:
        return np.concatenate((np.arange(0,n/2+1-step,step),np.arange(n/2-step,(n+1)%2-1,-1*step)))


def create_1D_cosine(n=100,w=1):
    x = np.arange(-2*np.pi, 2*np.pi, 4*np.pi / n)
    return x, np.cos(x*w)


def create_1D_ramp(n=100,step=1):
    return np.arange(0,n,1/float(step))


def show_1D(signal, title='Figure'):
    plt.plot(signal)
    plt.title(title)
    plt.show()


def mirror(s, margin_factor=2):
    n = s.size
    N = n*margin_factor
    sym_s = np.zeros(s.size*margin_factor, dtype=np.float32)
    if (margin_factor*n)%2 == 0:
        start_original_signal = n*(margin_factor-1)/2
    else:
        start_original_signal = (n+1)*(margin_factor-1)/2
    stop_original_signal = start_original_signal + n
    sym_s[start_original_signal:stop_original_signal] = s
    # compute signal with its sym once
    one_sym = np.concatenate((s, s[-2:0:-1]))
    # compute how many times to sym
    nb_sym = (N-stop_original_signal)/n
    if nb_sym == 0:
        nb_sym = 1
    one_sym_duplicated = np.kron(np.ones(nb_sym),one_sym)
    sym_s[stop_original_signal:N] = np.roll(one_sym_duplicated , -s.size)[0:N-stop_original_signal]
    sym_s[0:start_original_signal][::-1] = one_sym_duplicated [-1:-1-start_original_signal:-1]

    return sym_s


def zero_pad(s, margin_factor=2, dtype=np.complex):
    # initialize data
    n = s.size
    N = n*margin_factor
    zero_padded_s = np.zeros(N, dtype=dtype)

    # not the same process if even or odd number of samples
    if (margin_factor*n)%2 == 0:
        start_original_signal = n*(margin_factor-1)/2
    else:
        start_original_signal = (n+1)*(margin_factor-1)/2
    stop_original_signal = start_original_signal + n

    zero_padded_s[start_original_signal:stop_original_signal] = s

    return zero_padded_s


def periodize(s, margin_factor=2, dtype=np.complex):
    # initialize data
    n = s.size
    N = n*margin_factor
    periodic_s = np.zeros(N, dtype=dtype)

    # not the same process if even or odd number of samples
    if n%2 == 0:
        periodic_s[n/2:n/2+n] = s
        periodic_s[n/2+n:N] = s[0:n/2]
        periodic_s[0:n/2] = s[n/2:n]
    else:
        periodic_s[(n+1)/2:(n+1)/2+n] = s
        periodic_s[(n+1)/2+n:N] = s[0:(n-1)/2]
        periodic_s[0:(n+1)/2] = s[(n-1)/2:n]

    return periodic_s


def fft1D_zero_pad(tf, zoom_factor=2):
    # shift the signal as it is easier to fill new frequencies then
    tf_s = np.fft.fftshift(tf)

    zero_padded_tf_s = zero_pad(tf_s, margin_factor=zoom_factor)
    zero_padded_tf_s *= zoom_factor

    # return the shifted back spectrum
    return np.fft.ifftshift(zero_padded_tf_s)


def fft1D_periodization(tf, zoom_factor=2):
    tf_s = np.fft.fftshift(tf)

    periodized_tf_s = periodize(tf_s, margin_factor=zoom_factor)
    periodized_tf_s *= zoom_factor

    return np.fft.ifftshift(periodized_tf_s)


def zoom_freq_zpd(signal, zoom_factor=2):
    tf_s = np.fft.fft(signal)
    zero_pad_tf = fft1D_zero_pad(tf_s, 	zoom_factor)

    return np.fft.ifft(zero_pad_tf)


def zoom_freq_img_periodization(signal, zoom_factor=2, user_kernel=None):
    # if ap_window not set, if use the perfect low-pass filter
    if user_kernel is None:
        fft_ap_window = np.ones(signal.size * zoom_factor, dtype=np.complex256)
    else:
        # initialize data
        size_w = signal.size * zoom_factor
        zero_padded_user_kernel = np.zeros(size_w, dtype=np.float32)
        # suppose we have an even number of samples
        zero_padded_user_kernel[size_w/2-user_kernel.size/2:size_w/2-user_kernel.size/2+user_kernel.size] = user_kernel

        fft_user_kernel = np.fft.fft(np.fft.ifftshift(zero_padded_user_kernel))

    tf_s = np.fft.fft(signal)
    periodized_tf = fft1D_periodization(tf_s, zoom_factor)
    periodized_tf *= fft_user_kernel

    return np.fft.ifft(periodized_tf)


def interp1d(signal, zoom_factor):
    # Calcul de la dimension de l'image cible
    n = signal.size
    N = n*zoom_factor
    zoomed_signal = np.ones((N), dtype=np.complex)

    # Pour chaque partie fractionnaire possible,
    # calcul du noyau de convolution lineaire et ajout d'une marge au signal
    BLN_kernel = np.zeros((2))
    signal = np.concatenate((signal, np.array([signal[n-1]])))
    for Fx in range(zoom_factor):
        BLN_kernel[0] = (1-Fx/np.float(zoom_factor))
        BLN_kernel[1] = Fx/np.float(zoom_factor)

        # convolution (le noyau BLN est deja retourne, le mode est exact avec symmetrie au bord)
        zoomed_signal[Fx::zoom_factor] = signal[0:n] * BLN_kernel[0]
        zoomed_signal[Fx::zoom_factor] += signal[1:] * BLN_kernel[1]

    return(zoomed_signal)
    
    
def zoom_freq_img_perAndSmooth(signal, zoom_factor):
    n = signal.size
    # Etape 1 : Calcul de la partie lisse (s) de l'image
    x = np.arange(n)
    s = np.zeros(signal.shape)
    s = ((signal[n-1] - signal[0])/np.float(n)) * (x - (n-1)/2)

    # Etape 2 : Calcul de la partie periodique de l'image
    p = signal - s

    # Etape 3 : Zoom par zero padding de p
    tf_p = np.fft.fft(p)
    z_tf_p = fft1D_zero_pad(tf_p, zoom_factor)

    # Etape 4 : Zoom par simple interpolation de s
    bln_interp_s = interp1d(s, zoom_factor)
    show_1D(bln_interp_s,'bln_interp_s')

    #plt.suptitle('Periodic and smooth decomposition : spatial domain')
    #plt.subplot(411)
    #plt.plot(signal, label='signal')
    #plt.legend()
    #plt.subplot(412)
    #plt.plot(p, label='periodic')
    #plt.legend()
    #plt.subplot(413)
    #plt.plot(s, label='smooth')
    #plt.legend()
    #plt.subplot(414)
    #plt.plot(p+s, label='periodic + smooth')
    #plt.legend()
    #plt.show()

    # Etape 5 : merge et retour en spatial
    return np.fft.ifft(z_tf_p) + bln_interp_s
