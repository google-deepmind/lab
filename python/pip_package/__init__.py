"""Loads deepmind_lab.so."""

import imp
import pkg_resources

imp.load_dynamic(__name__, pkg_resources.resource_filename(
    __name__, 'deepmind_lab.so'))
