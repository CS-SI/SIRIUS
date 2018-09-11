# Sirius Changelog

## 0.2.0

### Features
* export `libsirius` and `libsirius-static` CMake targets when installing Sirius: use `find_package(SIRIUS CONFIG)` with `-DSIRIUS_DIR=/path/to/install/directory/share/cmake` to find Sirius libraries (#41)

### Bug fixes
* Windows support: Visual Studio 2017 (#41)
* OS X support: Xcode 8.3 and 9.4 (#41)
* Linux support: GCC 5, 6 and 7 and Clang 4, 5 and 6 (#41)


## 0.1.0 (09/08/2018)

### Features
* Frequency resampling
  * [Zero padding upsampling][zero-padding-upsampling]
  * [Periodization upsampling][periodization-upsampling]
* [Periodic plus smooth image decomposition][periodic-plus-smooth-image-decomposition]
* [Filtering][image-filtering]
* Stream processing

[zero-padding-upsampling]: https://cs-si.github.io/SIRIUS/html/upsampling/sinc_as_zpd.html
[periodization-upsampling]: https://cs-si.github.io/SIRIUS/html/upsampling/user_kernel.html#when-sirius-uses-a-filter-to-upsample-the-spectrum-is-periodized-instead-of-zero-padded
[periodic-plus-smooth-image-decomposition]: https://hal.archives-ouvertes.fr/file/index/docid/388020/filename/2009-11.pdf
[image-filtering]: https://cs-si.github.io/SIRIUS/html/upsampling/user_kernel.html