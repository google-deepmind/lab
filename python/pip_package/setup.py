"""Setup for the deepmind_lab module."""

import setuptools

REQUIRED_PACKAGES = [
    'numpy >= 1.13.3',
    'six >= 1.10.0',
]

setuptools.setup(
    name='deepmind-lab',
    version='1.0',
    description='DeepMind Lab: A 3D learning environment',
    long_description='',
    url='https://github.com/deepmind/lab',
    author='DeepMind',
    packages=setuptools.find_packages(),
    install_requires=REQUIRED_PACKAGES,
    include_package_data=True)
