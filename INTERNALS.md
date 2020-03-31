# Internals

## Private API

### FFTW

[FFTW] is used to compute FFT and IFFT of images. API Wrappers have been created in Sirius to make [FFTW] object lifetime management easier. On top of that, thread safety was taken into account to allow computation in a multi-threaded context.

### GDAL

[GDAL] is used to load image into memory and to save computed image. Just as [FFTW], API wrappers have been created for [GDAL] object lifetime management.

## C++ features

Sirius relies heavily on modern C++ features (C++11/14) such as smart pointers (`std::unique_ptr`, `std::shared_ptr`), concurrency API (`std::async`) or lambdas.

### Smart pointers

[Smart pointers] are used to manage object lifetime ([RAII idiom][RAII]). When a smart pointer goes out of scope, the default behavior is to delete the managed resource using `delete` or `delete[]` (when the smart pointer resource is declared as an array).

When the resource type has its own deleter, it is possible to customize the smart pointer deleter. This feature is used in Sirius for example on FFTW types (`fftw_complex` arrays, `double` arrays, `fftw_plan`) and GDAL type (`GDALDataset`).

```cpp
namespace gdal {

namespace detail {

struct DatasetDeleter {
    void operator()(::GDALDataset* dataset) { ::GDALClose(dataset); }
};

}  // namespace detail

using DatasetUPtr = std::unique_ptr<::GDALDataset, detail::DatasetDeleter>;

}  // namespace gdal
```

### Concurrency API

Sirius multi-threaded streaming is based on [lambdas][lambda] and [task mechanism][std::async]:

* One task will read an image block from the input image and feed an input queue
* N worker tasks will consume the input queue, compute the resampling and feed an output queue
* One task will consume the output queue and write the resampled block into the output image.

Tasks are created using [`std::async`][std::async] with the policy `std::launch::async` (force the creation of a new thread to execute the given task).

```cpp
auto task = []() { ... };
auto task_future = std::async(std::launch::async, task);
// wait end of task or exception
task_future.get();
```

## Design Patterns

### Singleton

Sirius is using a singleton to address the thread safety issue of [FFTW] plan creation.

This singleton is responsible of creating and deleting plan and is interacting with the `fftw_plan` smart pointer deleter.
[Meyers singleton implementation][MeyersSingleton] is used.

```cpp
class Dummy {
  public:
    // Get Dummy singleton instance
    static Dummy& Instance() {
        static Dummy instance;
        return instance;
    }

  private:
    Dummy() = default;
    ~Dummy() = default;

    // not copyable
    Dummy(const Dummy&) = delete;
    Dummy& operator=(const Dummy&) = delete;
    // not moveable
    Dummy(Dummy&&) = delete;
    Dummy& operator=(Dummy&&) = delete;
};
```

C++11 standard guarantees that a unique instance will be initialized and that this initialization is thread safe.

### Factory

The `FrequencyResampler` is the combination of two algorithms:
* an image decomposition algorithm
* an upsampling algorithm

Sirius provides an `IFrequencyResampler` factory to compose the resampler requested by a client.
API clients should only deal with `IFrequencyResampler` interface and should not be concerned by `IFrequencyResampler` implementations.

### Policy/Strategy

`FrequencyResampler` is the composition of an image decomposition algorithm and an upsampling algorithm.
The policy/strategy pattern is used to adapt the internal behavior of this class.

An algorithm implementation must comply with an implicit algorithm interface.

E.g, processing algorithms (upsampling, translation) should comply with:

```cpp
class ProcessorStrategy {
  public:
    Image Process(const Image& image, const transformation::Parameters& transformation_parameters) const;
};
```

Interpolation algorithms should comply with:

```cpp
class Interpolator {
  public:
    Image Interpolate2D(const Image& image, const transformation::Parameters& parameters) const;
};
```

Image decomposition algorithms should comply with:

```cpp
template <typename Transformation, typename ImageProcessor,
          typename ImageInterpolator>
class ImageDecompositionPolicy : private ImageProcessor, private ImageInterpolator {
  public:
    Image DecomposeAndProcess(
          const Image& image,
          const typename Transformation::Parameters& parameters) const;
};
```

## Insiders

### Logs

Sirius is using [spdlog] as its logger library.

This library makes it easy to generate and format logs in an application. Sinks are powerful components to generate (colored) logs in `stdout` or `stderr` but you can also send your logs in (rotating) files or to `syslog` without overhead.

Sirius provides `LOG` macro to create logs easily. CMake `ENABLE_LOGS` option is also available to lighten the generated binaries by removing all log strings.

### LRU Cache

Sirius is using a basic Last Recently Used (LRU) cache implementation to optimize some computation at the cost of memory overhead. CMake `ENABLE_CACHE_OPTIMIZATION` option is available to control this behavior.

FFTW plans are cached so that a plan with a given size is reused if it has already been created. Filter FFTs are also cached to be reused on a specific image FFTs.

### Concurrent queue

Sirius multi-threaded streaming is using `ConcurrentQueue` instances to feed resampler workers and to feed output stream.

This queue implementation is based on [std::condition_variable].

`ConcurrentQueue` objects have a maximum size and an active status.

When maximum size is reached, `Push` operations will block until an element is popped from the queue. Likewise, `Pop` operations will block until there is an element to pop from the queue.

To unblock `Push` or `Pop` operations (e.g. after an error occurred), you can deactivate the queue. Push new elements in a inactive queue will fail and pop element will be possible as long as there is an element inside the queue.

[FFTW]: http://www.fftw.org/ "Fastest Fourier Transform in the West"
[GDAL]: http://www.gdal.org/ "Geospatial Data Abstraction Library"
[Smart pointers]: https://en.cppreference.com/w/cpp/memory "Smart pointers"
[RAII]: https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization "Resource Acquisition Is Initialization"
[lambda]: https://en.cppreference.com/w/cpp/language/lambda "C++ lambda"
[std::async]: https://en.cppreference.com/w/cpp/thread/async "std::async"
[MeyersSingleton]: https://www.pearson.com/us/higher-education/program/Meyers-Effective-C-55-Specific-Ways-to-Improve-Your-Programs-and-Designs-3rd-Edition/PGM73417.html "Meyers singleton implementation"
[spdlog]: https://github.com/gabime/spdlog
[std::condition_variable]: https://en.cppreference.com/w/cpp/thread/condition_variable "std::condition_variable"
