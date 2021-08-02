# Copyright 2019 Google LLC
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

"""Conformance test for the dm_env API bindings."""

import os
from absl.testing import absltest
from dm_env import test_utils
import dmenv_module


class DmenvTest(test_utils.EnvironmentTestMixin, absltest.TestCase):

  def make_object_under_test(self):
    if 'TEST_SRCDIR' in os.environ:
      dmenv_module.set_runfiles_path(
          os.path.join(os.environ['TEST_SRCDIR'],
                       'org_deepmind_lab'))

    return dmenv_module.Lab(
        level='tests/entity_info_test',
        observation_names=['RGB_INTERLEAVED'],
        config={
            'fps': '60',
            'width': '32',
            'height': '32'
        })


if __name__ == '__main__':
  absltest.main()
