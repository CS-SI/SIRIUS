#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "sirius/filter.h"
#include "sirius/types.h"
#include "sirius/image.h"
#include "sirius/frequency_resampler_factory.h"
#include "sirius/i_frequency_resampler.h"

#include "sirius/utils/log.h"
#include <spdlog/spdlog.h>

namespace py = pybind11;

//using namespace sirius;
py::array_t<double> resampler(py::buffer input_array, int input_resolution, py::kwargs kwargs) {
  int output_resolution = 1;
  sirius::FrequencyZoomStrategies zoom_strategy = sirius::FrequencyZoomStrategies::kZeroPadding;
  sirius::ImageDecompositionPolicies image_decomposition = sirius::ImageDecompositionPolicies::kPeriodicSmooth;
  sirius::Padding image_padding(0,0,0,0);
  sirius::Filter filter  = {};
  sirius::ZoomRatio zoom_ratio = sirius::ZoomRatio::Create(input_resolution, output_resolution);
  auto log_level = spdlog::level::level_enum::off;
  for (auto item : kwargs) {
    std::string key = py::cast<std::string>(item.first);
    if (key == "log_level")  {
      log_level = py::cast<spdlog::level::level_enum>(item.second);
      break;
    }
  }

  sirius::utils::LoggerManager::Instance().SetLogLevel(log_level);
    
  for (auto item : kwargs) {
    std::string key = py::cast<std::string>(item.first);
    if (key == "output_resolution") {
      output_resolution = py::cast<int>(item.second);
      zoom_ratio = sirius::ZoomRatio::Create(input_resolution, output_resolution);
    }    
    else if (key == "filter_image") {
      py::buffer filter_buf = py::cast<py::buffer>(item.second);
      py::buffer_info filter_info = filter_buf.request();
      if (filter_info.ndim != 2)
	throw std::runtime_error("Incompatible buffer dimension!");
      auto img = sirius::Image(sirius::Size(filter_info.shape[0],  filter_info.shape[1]),
			       std::vector<double>(static_cast<double*>(filter_info.ptr),
						   static_cast<double*>(filter_info.ptr) + filter_info.shape[0] * filter_info.shape[1]));
      filter = sirius::Filter::Create(img, zoom_ratio);
    }
    else if (key == "zoom_strategy")
      zoom_strategy = py::cast<sirius::FrequencyZoomStrategies>(item.second);
    else if (key == "image_decomposition")
      image_decomposition = py::cast<sirius::ImageDecompositionPolicies>(item.second);
    else if (key == "image_padding")
      image_padding = py::cast<sirius::Padding>(item.second);
  }

  if (kwargs.contains("filter_image") && !kwargs.contains("zoom_strategy"))
    zoom_strategy = sirius::FrequencyZoomStrategies::kPeriodization;

  py::buffer_info input_array_info = input_array.request();
  if (input_array_info.format != py::format_descriptor<double>::format())
    throw std::runtime_error("Incompatible format: expected a double array!");
  
  if (input_array_info.ndim != 2)
    throw std::runtime_error("Incompatible buffer dimension!");
  
  auto input_size = sirius::Size(input_array_info.shape[0],  input_array_info.shape[1]);
  auto input_image = sirius::Image(input_size, std::vector<double>(static_cast<double*>(input_array_info.ptr),
								   static_cast<double*>(input_array_info.ptr) + input_size.row * input_size.col));
  
  sirius::IFrequencyResampler::UPtr freq_resampler = sirius::FrequencyResamplerFactory::Create(image_decomposition, zoom_strategy);
  auto output_image = freq_resampler->Compute(zoom_ratio, input_image, image_padding, filter);
  auto output_buffer_size = output_image.size.row * output_image.size.col;
  return py::array_t<double>(std::vector<ptrdiff_t>{output_image.size.row, output_image.size.col}, &output_image.data[0]);
}

PYBIND11_MODULE(siriuspy, m) {
  using namespace sirius;
 
 py::enum_<spdlog::level::level_enum>(m, "spdlog")
   .value("trace", spdlog::level::trace)
   .value("debug", spdlog::level::debug)
   .value("info", spdlog::level::info)
   .value("warn", spdlog::level::warn)
   .value("err", spdlog::level::err)
   .value("critical", spdlog::level::critical)
   .value("off", spdlog::level::off)
   .export_values();
   
  py::enum_<sirius::ImageDecompositionPolicies>(m, "ImageDecompositionPolicies")
    .value("kRegular", sirius::ImageDecompositionPolicies::kRegular)
    .value("kPeriodicSmooth", sirius::ImageDecompositionPolicies::kPeriodicSmooth)
    .export_values();
  
  py::enum_<sirius::FrequencyZoomStrategies>(m, "FrequencyZoomStrategies")
    .value("kZeroPadding", sirius::FrequencyZoomStrategies::kZeroPadding)
    .value("kPeriodization", sirius::FrequencyZoomStrategies::kPeriodization)
    .export_values();

  py::class_<ZoomRatio>(m, "ZoomRatio")
    .def_static("Create",     py::overload_cast<int,int>(&ZoomRatio::Create))
    .def("ratio",                                 &ZoomRatio::ratio)
    .def("IsRealZoom",                            &ZoomRatio::IsRealZoom)
    .def("output_resolution",                     &ZoomRatio::output_resolution)
    .def("input_resolution",                      &ZoomRatio::input_resolution)
    ;
  py::class_<Padding>(m, "Padding")
    .def(py::init<int,int,int,int,PaddingType>())
    .def("IsEmpty", &Padding::IsEmpty)
    ;
  py::enum_<sirius::PaddingType>(m, "PaddingType", py::arithmetic())
    .value("kZeroPadding", sirius::PaddingType::kZeroPadding)
    .value("kMirrorPadding", sirius::PaddingType::kMirrorPadding)
    .export_values();
  
  m.def("resampler", &resampler, "Resample an image by a ratio in the frequency domain");  

}
