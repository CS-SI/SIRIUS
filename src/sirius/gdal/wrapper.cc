/**
 * Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
 *
 * This file is part of Sirius
 *
 *     https://github.com/CS-SI/SIRIUS
 *
 * Sirius is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Sirius is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sirius.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sirius/gdal/wrapper.h"

#include "sirius/gdal/exception.h"

#include "sirius/utils/log.h"

namespace sirius {
namespace gdal {

GeoReference::GeoReference()
    : geo_transform{0, 1, 0, 0, 0, 1},
      projection_ref(""),
      is_initialized(false) {}

GeoReference::GeoReference(const std::vector<double>& geo_trans,
                           const std::string& proj_ref)
    : geo_transform(geo_trans),
      projection_ref(proj_ref),
      is_initialized(true) {}

DatasetUPtr LoadDataset(const std::string& filepath) {
    if (filepath.empty()) {
        LOG("gdal", debug, "no filepath provided");
        return {};
    }

    ::GDALAllRegister();

    LOG("gdal", trace, "loading dataset '{}'", filepath);
    DatasetUPtr dataset(
          static_cast<GDALDataset*>(::GDALOpen(filepath.c_str(), GA_ReadOnly)));
    if (dataset == nullptr) {
        LOG("gdal", error, "could not open the image file '{}'", filepath);
        throw gdal::Exception();
    }

    return dataset;
}

DatasetUPtr CreateDataset(const std::string& filepath, int w, int h,
                          int n_bands, const GeoReference& geo_ref) {
    if (filepath.empty()) {
        LOG("gdal", debug, "no filepath provided");
        return {};
    }

    ::GDALAllRegister();

    LOG("gdal", trace, "creating dataset '{}'", filepath);

    auto driver = ::GetGDALDriverManager()->GetDriverByName("GTiff");
    DatasetUPtr dataset(
          driver->Create(filepath.c_str(), w, h, n_bands, GDT_Float32, NULL));
    if (dataset == nullptr) {
        LOG("gdal", error, "could not create the image file '{}'", filepath);
        throw gdal::Exception();
    }

    if (geo_ref.is_initialized) {
        CPLErr err = dataset->SetGeoTransform(
              const_cast<double*>(geo_ref.geo_transform.data()));
        if (err) {
            LOG("gdal", warn,
                "GDAL error: {} - could not write output geo transform", err);
        }

        err = dataset->SetProjection(geo_ref.projection_ref.data());
        if (err) {
            LOG("gdal", warn,
                "GDAL error: {} - could not write output coordinate system",
                err);
        }
    }

    return dataset;
}

Image LoadImage(const std::string& filepath) {
    if (filepath.empty()) {
        LOG("gdal", debug, "no filepath provided");
        return {};
    }

    LOG("gdal", trace, "loading image '{}'", filepath);
    auto dataset = LoadDataset(filepath);

    Size tmp_size = {dataset->GetRasterYSize(), dataset->GetRasterXSize()};
    LOG("gdal", trace, "image size: {}x{}", tmp_size.row, tmp_size.col);

    Buffer tmp_buffer(tmp_size.row * tmp_size.col);

    CPLErr err = dataset->GetRasterBand(1)->RasterIO(
          GF_Read, 0, 0, tmp_size.col, tmp_size.row, tmp_buffer.data(),
          tmp_size.col, tmp_size.row, GDT_Float64, 0, 0);
    if (err) {
        LOG("gdal", error,
            "GDAL error: {} - could not get image data from file '{}'", err,
            filepath);
        throw gdal::Exception();
    }

    return {tmp_size, std::move(tmp_buffer)};
}

void SaveImage(const Image& image, const std::string& output_filepath,
               const GeoReference& geoRef) {
    LOG("gdal", trace, "saving image into '{}'", output_filepath);

    // TODO: basic save implementation, test only ATM
    auto dataset = CreateDataset(output_filepath, image.size.col,
                                 image.size.row, 1, geoRef);

    auto band = dataset->GetRasterBand(1);
    CPLErr err =
          band->RasterIO(GF_Write, 0, 0, image.size.col, image.size.row,
                         const_cast<double*>(image.data.data()), image.size.col,
                         image.size.row, GDT_Float64, 0, 0);
    if (err) {
        LOG("image", error, "GDAL error: {} - could not write in file '{}'",
            err, output_filepath);
        throw gdal::Exception();
    }
}

GeoReference ComputeResampledGeoReference(const std::string& input_path,
                                          const ZoomRatio& zoom_ratio) {
    auto input_dataset = sirius::gdal::LoadDataset(input_path);

    return {ComputeResampledGeoTransform(input_dataset.get(), zoom_ratio),
            input_dataset->GetProjectionRef()};
}

std::vector<double> ComputeResampledGeoTransform(GDALDataset* dataset,
                                                 const ZoomRatio& zoom_ratio) {
    std::vector<double> geo_transform(6);
    CPLErr err = dataset->GetGeoTransform(geo_transform.data());
    if (err) {
        LOG("gdal", debug,
            "GDAL error: {} - could not read input geo transform", err);
        return geo_transform;
    }

    // move origin to the center of input top left pixel
    geo_transform[0] += (0.5 * geo_transform[1]);
    geo_transform[3] += (0.5 * geo_transform[5]);

    // set output pixel size
    geo_transform[1] *= (zoom_ratio.output_resolution() /
                         static_cast<double>(zoom_ratio.input_resolution()));
    geo_transform[5] *= (zoom_ratio.output_resolution() /
                         static_cast<double>(zoom_ratio.input_resolution()));

    // place output origin to the top left corner of top left pixel
    geo_transform[0] -= (0.5 * geo_transform[1]);
    geo_transform[3] -= (0.5 * geo_transform[5]);

    return geo_transform;
}

std::vector<double> GetGeoTransform(GDALDataset* dataset) {
    std::vector<double> geo_transform(6);
    CPLErr err = dataset->GetGeoTransform(geo_transform.data());
    if (err) {
        LOG("gdal", debug,
            "GDAL error: {} - could not read input geo transform", err);
        return geo_transform;
    }

    return geo_transform;
}

GeoReference GetGeoReference(GDALDataset* dataset) {
    return {GetGeoTransform(dataset), dataset->GetProjectionRef()};
}

GeoReference ComputeShiftedGeoReference(GDALDataset* dataset, float row_shift,
                                        float col_shift) {
    auto geo_ref = GetGeoReference(dataset);
    if (geo_ref.geo_transform[0] != 0.0 && geo_ref.geo_transform[3] != 0) {
        geo_ref.geo_transform[0] += row_shift * geo_ref.geo_transform[1];
        geo_ref.geo_transform[3] += col_shift * geo_ref.geo_transform[5];
    }

    return geo_ref;
}

}  // namespace gdal
}  // namespace sirius
