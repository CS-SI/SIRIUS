#!/usr/bin/env python3

import os
import siriuspy
import inspect
def print_classes():
    for name, obj in inspect.getmembers(siriuspy):
        if inspect.isclass(obj):
            print(obj) #, obj.__name__)

#print_classes()
import unittest
import numpy
from scipy import misc
import matplotlib.pyplot as plt
from PIL import Image
from osgeo import gdal

def gdal_load_image(filename, bindex=1):
    dataset = gdal.Open(filename, gdal.GA_ReadOnly)
    if not dataset:
        raise ValueError('gdal.Open failed for', filename)
    band = dataset.GetRasterBand(bindex)
    nparray = band.ReadAsArray().astype(numpy.float)
    return nparray


npimage = numpy.zeros((256,256))
for row in range(256):
    for col in range(256):
        npimage[row][col] = row * 10 + col + 1


data_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), os.pardir, "data"))
dirac_image = gdal_load_image(os.path.join(data_dir, "filters", "ZOOM_1_2.tif"))
sinc_image  = gdal_load_image(os.path.join(data_dir, "filters", "ZOOM_2.tif"))
sinc_padding  = siriuspy.Padding(0, 0, 0, 0, siriuspy.PaddingType.kMirrorPadding)
dirac_padding = siriuspy.Padding(0, 0, 0, 0, siriuspy.PaddingType.kMirrorPadding)

class TestResampler(unittest.TestCase):

    def test_log_level_critical(self):
        zoomed_image = siriuspy.resampler(npimage, 6, log_level=siriuspy.spdlog.critical)
        self.assertEqual(zoomed_image.shape, (1536,1536))

    def test_log_level_debug(self):
        zoomed_image = siriuspy.resampler(npimage, 6, log_level=siriuspy.spdlog.debug)
        self.assertEqual(zoomed_image.shape, (1536,1536))

    def test_log_level_info(self):
        zoomed_image = siriuspy.resampler(npimage, 6, log_level=siriuspy.spdlog.off)
        self.assertEqual(zoomed_image.shape, (1536,1536))

    def test_log_level_trace(self):
        zoomed_image = siriuspy.resampler(npimage, 6, log_level=siriuspy.spdlog.off)
        self.assertEqual(zoomed_image.shape, (1536,1536))


    def test_resampler_with_all_non_default_values(self):
        zoomed_image = siriuspy.resampler(npimage, 8,
                                              output_resolution=4,
                                              filter_image=dirac_image,
                                              image_padding=sinc_padding,
                                              image_decomposition=siriuspy.ImageDecompositionPolicies.kRegular,
                                              zoom_strategy=siriuspy.FrequencyZoomStrategies.kZeroPadding)
        self.assertEqual(zoomed_image.shape, (512,512))

    def test_resampler_with_all_options(self):
        #print("zoom_strategy will not be changed\n")
        zoomed_image = siriuspy.resampler(npimage, 2,
                                              output_resolution=3,
                                              filter_image=sinc_image,
                                              image_padding=sinc_padding,
                                              image_decomposition=siriuspy.ImageDecompositionPolicies.kPeriodicSmooth,
                                              zoom_strategy=siriuspy.FrequencyZoomStrategies.kZeroPadding)
        self.assertEqual(zoomed_image.shape, (128,128))

    def test_resampler_with_different_image_decomposition_and_zoom_strategy(self):
        zoomed_image = siriuspy.resampler(npimage, 8,
                                              output_resolution=4,
                                              filter_image=dirac_image,
                                              image_decomposition=siriuspy.ImageDecompositionPolicies.kRegular,
                                              zoom_strategy=siriuspy.FrequencyZoomStrategies.kPeriodization)
        self.assertEqual(zoomed_image.shape, (512,512))

    def test_resampler_with_filter_image_and_output_resolution(self):
        #print("zoom_strategy will default to kPeriodization because of argument 'filter_image'\n")
        zoomed_image = siriuspy.resampler(npimage, 2,
                                              output_resolution=1,
                                              filter_image=sinc_image)
        self.assertEqual(zoomed_image.shape, (448,448))

    def test_resampler_without_imagepadding_option(self):
        zoomed_image = siriuspy.resampler(npimage, 2,
                                              output_resolution=4,
                                              filter_image=sinc_image,
                                              zoom_strategy=siriuspy.FrequencyZoomStrategies.kZeroPadding)
        self.assertEqual(zoomed_image.shape, (96,96))

    def test_resampler_with_only_required_options(self):
        zoomed_image = siriuspy.resampler(npimage, 2)
        self.assertEqual(zoomed_image.shape, (512,512))

if __name__ == '__main__':
    #unittest.main()
    help(siriuspy.resampler)
