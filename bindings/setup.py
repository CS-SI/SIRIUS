import os, sysconfig
from setuptools import setup, Extension, distutils
from setuptools.command.build_ext import build_ext
from setuptools.dist import Distribution

class BinaryDistribution(Distribution):
    def is_pure(self):
        return False

extra_compile_args = sysconfig.get_config_var('CFLAGS').split()
extra_compile_args.append('-std=c++14')
#TODO: use generated config.h and avoid SIRIUS_ENABLE_LOGS and others
extra_compile_args.append('-DSIRIUS_ENABLE_LOGS=1')

# extra_compile_args.append(cpp_flag(self.compiler))
# if has_flag(self.compiler, '-fvisibility=hidden'):
#     opts.append('-fvisibility=hidden')

ext_modules = [
    Extension(name='siriuspy',
        sources=['siriuspy.cpp'],
        libraries=['libsirius'],
        extra_compile_args=extra_compile_args,
        language='c++'
    ),
]

setup(
    name='siriuspy',
    author='CS-SI',
    author_email='github.com/CS-SI/SIRIUS',
    url='github.com/CS-SI/SIRIUS',
    description='Python bindings for SIRIUS',
    long_description='Fast and simple to plug-in C++ resampling library that is taking advantage of the Fourier Transform',
    ext_modules=ext_modules,
    #install_requires=['pybind11>=2.2'],
    #cmdclass={'build_ext': Extension},
    zip_safe=False
)
