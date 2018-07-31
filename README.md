<h1 align="center">
  <br>
  <a href="https://CS-SI.github.io/SIRIUS/html/Sirius.html"><img src="https://raw.githubusercontent.com/CS-SI/SIRIUS/master/doc/img/sirius-logo.svg?sanitize=true" alt="Sirius by CS-SI" width=35%></a>
</h1>

<h4 align="center">A fast resampling software with simple to plugin library.</h4>

<p align="center">
  <a href="https://travis-ci.org/CS-SI/SIRIUS"><img src="https://travis-ci.org/CS-SI/SIRIUS.svg?branch=master"></a>
  <a href="https://hub.docker.com/r/ldumas/sirius_dockerfile/tags/"><img src="https://img.shields.io/docker/automated/ldumas/sirius_dockerfile.svg"></a>
  <a href="https://github.com/CS-SI/SIRIUS/issues"><img src="https://img.shields.io/github/issues/CS-SI/SIRIUS.svg"></a>
  <a href="https://opensource.org/licenses/GPL-3.0/"><img src="https://img.shields.io/badge/licence-GPL-blue.svg"></a>
  <a href="http://makeapullrequest.com"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=shields"></a>
</p>

<p align="center">
  <a href="#overview">Overview</a> •
  <a href="#docker_app">Docker App</a> •
  <a href="#how-to-build">How To Build</a> •
  <a href="#how-to-use">How To Use</a> •
  <a href="#credits">Acknowledgement</a>
</p>

<h4 align="center">
  <a href="https://uk.c-s.fr/"><img src="https://github.com/CS-SI/SIRIUS/blob/master/doc/img/logo_cssi.jpg" alt="CS" width="149px"></a>
</h4>

As of today, and to our knowledge, there is no complete open source resampling tool for satellite imagery based on
frequency resampling. Numerous softwares, some being open source, can be found to resample (most of the time upsample)
data. Most of them offers various interpolators ([GIMP], [Pandore], [Getreuer], [OTB]),
some being very fast but rather innacurate (nearest neighbors, bilinear) and some offering high quality results
(more often than not based on a Lancsoz convolutional kernel) but being very time consuming.
Then only a few provides ways to deal efficiently with large amount of data ([OTB]) or proposes a
fast and accurate sinus cardinal interpolator implemented as frequencial zero padding ([Getreuer]).
None of those however allows one to resample satellite images in frequency domain with a floating upscaling
or downscaling factor.

It could be argued that the [OTB] remains the best solution for the purpose of resampling satellite images.
However, and though there exists ways to filter satellite images in frequency domain with the [OTB],
it is not possible to resample such an image in frequency domain.

Sirius then aims at filling this void offering a fast and simple to plug-in resampling C++ library that is taking advantage of the Fourier Transform.

## Overview

* [Theoretical Basis Documentation][Theoretical Basis]
* [Internals][Internals]

## Docker App

Sirius is delivered inside a docker container. Assuming one has previously installed docker, then Sirius can be launched
using the following commands :

```sh
# pull image from the registry
docker pull ldumas/sirius_dockerfile:sirius
# then run sirius
docker run ldumas/sirius_dockerfile:sirius [OPTION...] input-image output-image

# if no arguments are set then sirius is launched without any argument and its help is displayed
# see below for the list of sirius args

# note that using docker volume might be a good solution to give the container access to input-image
# and to give host access to the output-image :
docker run -v /home/user/outdir/:/outdir -v /home/user/input_images:/input ldumas/sirius_dockerfile:sirius /input/input-image.tif /outdir/output-image.tif -z 1 -d 2
```

## How to Build

Sirius is using [CMake] to build its libraries and executables.

### Requirements

* C++14 compiler (GCC >= 5)
* [CMake] >=3.2
* [GDAL] development kit, >=2
* [FFTW] development kit, >=3
* [Doxygen] if documentation option is enabled

### Internal dependencies

* [spdlog v0.17.0]
* [cxxopts v2.1.0]
* [GSL v1.0.0]
* [catch v2.2.3]
* [cmake-modules]

### Options

* `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo or MinSizeRel
* `CMAKE_INSTALL_PREFIX`: directory path where the built artifacts (include directory, library, docs) will be gathered
* `ENABLE_CACHE_OPTIMIZATION`: set to `ON` to build with cache optimization for FFTW and Filter
* `ENABLE_GSL_CONTRACTS`: set to `ON` to build with GSL contracts (e.g. bounds checking). This option should be `OFF` on release mode.
* `ENABLE_LOGS`: set to `ON` if you want to build Sirius with the logs
* `ENABLE_UNIT_TESTS`: set to `ON` if you want to build the unit tests
* `ENABLE_DOCUMENTATION`: set to `ON` if you want to build the documentation

### Example

```sh
# CWD is Sirius root directory
mkdir .build
cd .build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/tmp/sirius \
         -DENABLE_CACHE_OPTIMIZATION=ON \
         -DENABLE_GSL_CONTRACTS=OFF \
         -DENABLE_LOGS=ON \
         -DENABLE_UNIT_TESTS=OFF \
         -DENABLE_DOCUMENTATION=ON
cmake --build . --target sirius
cmake --build . --target doc
cmake --build . --target install
```

## How to use

### Host requirements

* [GDAL] library
* [FFTW3] library

### Sirius standalone tool

Sirius is shipped as a standalone tool that offers filtering and resampling features.

```sh
$ ./sirius -h
Sirius X.Y.Z (...)
Standalone tool to resample and filter images in the frequency domain

Usage:
  ./sirius [OPTION...] input-image output-image

  -h, --help           Show help
  -v, --verbosity arg  Set verbosity level
                       (trace,debug,info,warn,err,critical,off) (default: info)

 resampling options:
  -z, --input-resolution arg    Numerator of the resampling ratio (default:
                                1)
  -d, --output-resolution arg   Denominator of the resampling ratio (default:
                                1)
      --no-image-decomposition  Do not decompose the input image (default is
                                periodic plus smooth image decomposition)
      --upsample-periodization  Force periodization as upsampling algorithm
                                (default algorithm if a filter is provided). A
                                filter is required to use this algorithm
      --upsample-zero-padding   Force zero padding as upsampling algorithm
                                (default algorithm if no filter is provided)

 filter options:
      --filter arg           Path to the filter image to apply to the source
                             or resampled image
      --filter-no-padding    Do not add filter margins on input borders
                             (default is mirror padding)
      --filter-zero-padding  Use zero padding strategy to add filter margins
                             on input borders (default is mirror padding)
      --filter-normalize     Normalize filter coefficients (default is no
                             normalization)

 streaming options:
      --stream                  Enable stream mode
      --block-width arg         Width of a stream block (default: 256)
      --block-height arg        Height of a stream block (default: 256)
      --no-block-resizing       Disable block resizing optimization
      --parallel-workers [=arg(=1)]
                                Parallel workers used to compute resampling
                                (8 max) (default: 1)
```

#### Processing mode options

##### Regular mode

Regular mode (default mode) will put the whole image in memory and then processed it. **This mode should only be used on small image**.

The following command line will zoom in the image `/path/to/input-file.tif` by 4/3 with the periodic plus smooth image decomposition, apply the filter `/path/to/filter-image-4-3.tif` to the zoomed image and write the result into `/path/to/output-file.tif`.


```sh
./sirius -z 4 -d 3 \
         --filter /path/to/filter-image-4-3.tif \
         /path/to/input-file.tif /path/to/output-file.tif
```

##### Stream mode

Stream mode is activated with the option `--stream`. It will cut the image into multiple blocks of small size (default block size is 256x256). Each block will be processed separately and result blocks will be aggregated to generate the output image. **This mode must be used on large image.**

Stream mode can be run in mono-threaded context (`--parallel-workers=1`) or in multi-threaded context (`--parallel-workers=N` where N is the requested number of threads which will compute the resampling).

```sh
./sirius -z 4 -d 3 \
         --stream --parallel-workers=4 \
         --filter /path/to/filter-image-4-3.tif \
         /path/to/input-file.tif /path/to/output-file.tif
```

It is possible to customize block size with the options `--block-witdh=XXX` and `--block-height=YYY`.

Default behavior tries to optimize block size so that the processed block (block size + filter margins) width and height are dyadic. You can disable this optimization with the option `--no-block-resizing`.

When dealing with real zoom, block width and height are computed so that they comply with the zoom ratio.

#### Resampling options

Sirius can use two image decomposition algorithms:
* Periodic plus Smooth (default behavior) is splitting the input image into a periodic part and a smooth image part.
* None (`--no-image-decomposition`) is using raw image data without any processing.

Sirius can use two upsampling strategies:
* Periodization: default behavior if a filter is provided.
* Zero padding: default algorithm if no filter is provided)


Upsampling strategies can be forced with the following options:
* `--upsampling-zero-padding`
* `--upsampling-periodization`

**Force periodization upsampling without providing a filter will result in an error.**

More details on algorithms in the [Theoretical Basis documentation][Theoretical Basis].

#### Filter options

A filter image path can be specified with the option `--filter`. This filter will be applied:
* on the resampled image if the image is zoomed in
* on the source image if the image is zoomed out

Default behavior will pad filter margins with a mirroring of the image borders.

`--filter-no-padding` will change the padding strategy and will not pad filter margins on the image borders.

`--filter-zero-padding` will change the padding strategy and zero pad filter margins on the image borders.

It is assumed that the filter is already normalized. If not, the option `--filter-normalize` will normize it before any processing.

More details on filters in the [Theoretical Basis documentation][Theoretical Basis].

#### Examples

##### Zoom in

The following command line will zoom in `input/lena.jpg` by 2 using periodic plus smooth image decomposition and zero padding zoom.

```sh
./sirius -z 2 -d 1 \
         --upsample-zero-padding \
         input/lena.jpg output/lena_z2.jpg
```

The following command line will zoom in `input/lena.jpg` by 2 using periodic plus smooth image decomposition, periodization upsampling and filter for upsampling 2.

```sh
./sirius -z 2 -d 1 \
         input/lena.jpg output/lena_z2.jpg
```

The following command line will zoom in `input/sentinel2_20m.tif` by 2 using stream mode and 8 workers, periodic plus smooth image decomposition, periodization upsampling and filter for upsampling 2.

```sh
./sirius --stream --parallel-workers=8 \
         -z 2 -d 1 \
         --filter filters/ZOOM_2.tif \
         input/sentinel2_20m.tif output/sentinel2_20m_z2.tif
```

##### Zoom out

The following command line will zoom out `input/lena.jpg` by 1/2 using periodic plus smooth image decomposition and filter for zoom 1/2.

```sh
./sirius -z 1 -d 2 \
         --filter filters/ZOOM_1_2.tif \
         input/lena.jpg output/lena_z2.jpg
```

The following command line will zoom out `input/disparity.png` by 1/2 using periodic plus smooth image decomposition and filter for zoom 1/2.

```sh
./sirius -z 1 -d 2 \
         --filter filters/ZOOM_1_2.tif \
         input/disparity.png output/disparity_z1_2.jpg
```

The following command line will zoom out `input/sentinel2_10m.tif` by 1/2 using using stream mode and 8 workers, periodic plus smooth image decomposition and filter for zoom 1/2.

```sh
./sirius --stream --parallel-workers=8 \
         -z 1 -d 2 \
         --filter filters/ZOOM_1_2.tif \
         input/sentinel2_10m.tif output/sentinel2_10m_z1_2.tif
```

### Sirius library API

Sirius is designed to be easy to use.

The main interface to compute a frequency resampling is `IFrequencyResampler` and it only requires an image, a zoom ratio and an optional filter.

`IFrequencyResampler` objects are instantiated by the `FrequencyResamplerFactory`.

#### Example without filter

```cpp
#include "sirius/filter.h"
#include "sirius/frequency_resampler_factory.h"
#include "sirius/image.h"
#include "sirius/types.h"

// create an image
sirius::Image image = {...};

// configure the zoom ratio
sirius::ZoomRatio zoom_ratio{7, 5};

// compose a frequency resampler from sirius::ImageDecompositionPolicies and
//     sirius::FrequencyZoomStrategies enums
sirius::IFrequencyResampler::UPtr freq_resampler =
      sirius::FrequencyResamplerFactory::Create(
            sirius::ImageDecompositionPolicies::kPeriodicSmooth,
            sirius::FrequencyZoomStrategies::kZeroPadding);

// compute the resampled image
sirius::Image resampled_image = freq_resampler->Compute(
      zoom_ratio, image, {}, {});
```

#### Example with filter

```cpp
#include "sirius/filter.h"
#include "sirius/frequency_resampler_factory.h"
#include "sirius/image.h"
#include "sirius/types.h"

// create an image
sirius::Image image = {...};

// configure the zoom ratio
sirius::ZoomRatio zoom_ratio = {7, 5};

// create a filter from an image file
sirius::Filter filter = sirius::Filter::Create("/path/to/filter/image_7_5.tif",
                                               zoom_ratio);

// compose a frequency resampler from sirius::ImageDecompositionPolicies and
//     sirius::FrequencyZoomStrategies enums
sirius::IFrequencyResampler::UPtr freq_resampler =
      sirius::FrequencyResamplerFactory::Create(
            sirius::ImageDecompositionPolicies::kPeriodicSmooth,
            sirius::FrequencyZoomStrategies::kPeriodization);

// compute the resampled image
sirius::Image resampled_image = freq_resampler->Compute(
      zoom_ratio, image, filter.padding(), filter);
```

#### Thread safety

Compute a resampled image with Sirius is thread safe so it is possible to use the same `IFrequencyResampler` object in a multi-threaded context.

Process an image with a `Filter` object is also thread safe so you can reuse the same filter in a multi-threaded context.

## Unit tests

Running tests requires data features (input image, filters) which are available [here][Sirius test data features].

You need to execute the tests in the root directory of those data features. Expected directory tree is:
```
ROOT_DATA_FEATURES/input
                  /filters
                  /output
```

`frequency_resampler_tests` and `functional_tests` will create output images in the directory `ROOT_DATA_FEATURES/output`

## Acknowledgement

Sirius developers would like to thank:
* Matteo Frigo, Steven G. Johnson et al. for [FFTW3 library (GNU General Public License v3)][FFTW3]
* [GDAL (MIT License)][GDAL] project team
* Gabi Melman et al. for [spdlog (MIT License)][spdlog]
* Jarryd Beck et al. for [cxxopts (MIT License)][cxxopts]
* [Catch2 (Boost Software License - Version 1.0)][catch2] project team
* [GSL (MIT License)][GSL] project team
* Ryan Pavlik et al. for [cmake-modules (Boost Software License - Version 1.0)][cmake-modules]



[OTB]: https://www.orfeo-toolbox.org "Orfeo ToolBox"
[GIMP]: https://www.gimp.org/fr/ "GNU Image Manipulation Program"
[Pandore]: https://clouard.users.greyc.fr/Pandore "Pandore: Une bibliothèque d'opérateurs de traitement d'images (Version 6.6). Laboratoire GREYC."
[Getreuer]: https://doi.org/10.5201/ipol.2011.g_lmii "Getreuer, P. (2011). Linear Methods for Image Interpolation. Image Processing On Line, 1, 238259."

[Internals]: INTERNALS.md "Internals"
[Theoretical Basis]: https://CS-SI.github.io/SIRIUS/html/Sirius.html
[Sirius test data features]: https://github.com/CS-SI/SIRIUS "Sirius test data features"

[CS-SI]: https://uk.c-s.fr/ "CS Systèmes d'information"
[CMake]: https://cmake.org/ "CMake"
[GDAL]: http://www.gdal.org/ "Geospatial Data Abstraction Library"
[FFTW]: http://www.fftw.org/ "Fastest Fourier Transform in the West"
[Doxygen]: http://www.doxygen.org "Doxygen"
[FFTW3]: http://www.fftw.org/fftw-paper-ieee.pdf "Matteo Frigo and Steven G. Johnson, “The design and implementation of FFTW3,” Proc. IEEE 93 (2), 216231 (2005)"
[spdlog]: https://github.com/gabime/spdlog "spdlog"
[spdlog v0.17.0]: https://github.com/gabime/spdlog/tree/v0.17.0 "spdlog v0.17.0"
[cxxopts]: https://github.com/jarro2783/cxxopts "cxxopts"
[cxxopts v2.1.0]: https://github.com/jarro2783/cxxopts/tree/v2.1.0 "cxxopts v2.1.0"
[GSL]: https://github.com/Microsoft/GSL "Guideline Support Library"
[GSL v1.0.0]: https://github.com/Microsoft/GSL/tree/v1.0.0 "Guideline Support Library v1.0.0"
[catch2]: https://github.com/catchorg/Catch2/tree/v2.2.3 "Catch2"
[catch v2.2.3]: https://github.com/catchorg/Catch2/tree/v2.2.3 "Catch v2.2.3"
[cmake-modules]: https://github.com/rpavlik/cmake-modules "CMake Modules"
