"""Setup for the deepmind_lab module."""

import setuptools

REQUIRED_PACKAGES = [
    'numpy >= 1.13.3',
    'six >= 1.10.0',
]

setuptools.setup(
    name='DeepMind Lab',
    version='1.0',
    description='DeepMind Lab',
    long_description='',
    url='https://github.com/deepmind/lab',
    author='DeepMind',
    packages=setuptools.find_packages(),
    install_requires=REQUIRED_PACKAGES,
    include_package_data=True)
