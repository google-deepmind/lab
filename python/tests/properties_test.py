# Copyright 2018-2019 Google Inc.
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
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import six

import deepmind_lab


def props_to_dictionary_recurse(env, key, result):
  prefix_length = len(key) + 1 if key else 0
  for sub_key, attribs in sorted(env.properties(key).items()):
    next_key = sub_key[prefix_length:]
    if attribs & deepmind_lab.LISTABLE == deepmind_lab.LISTABLE:
      props_to_dictionary_recurse(env, sub_key, result.setdefault(next_key, {}))
    elif attribs & deepmind_lab.READABLE == deepmind_lab.READABLE:
      result[next_key] = env.read_property(sub_key)
    elif attribs & deepmind_lab.WRITABLE == deepmind_lab.WRITABLE:
      result[next_key] = '<write-only>'


def props_to_dictionary(env):
  result = {}
  props_to_dictionary_recurse(env, '', result)
  return result


class Properties(absltest.TestCase):

  def test_engine_properties(self):
    env = deepmind_lab.Lab('tests/properties_test', [], config={'fps': '15'})
    props = props_to_dictionary(env)
    self.assertEqual(props['engine']['fps'], '15')

    with self.assertRaises(TypeError) as context:
      env.write_property('engine.fps', 'bar')
    self.assertIn('\'engine.fps\' not writable.', str(context.exception))

    # Check engine.notReachable wasn't registered.
    self.assertNotIn('notReachable', props['engine'])

    # Try and write to root engine list.
    with self.assertRaises(TypeError) as context:
      env.write_property('engine', 'foo')
    self.assertIn('\'engine\' not writable.', str(context.exception))

  def test_list_properties(self):
    env = deepmind_lab.Lab('tests/properties_test', [], config={})
    props = props_to_dictionary(env)
    expected_params = {
        'arg1': 'true',
        'arg2': '19',
        'arg3': 'hello',
        'arrArgs': {
            '1': 'arr1',
            '2': 'arr2',
            '3': 'arr3'
        },
        'subArgs': {
            'sub1': 'val1',
            'sub2': 'val2',
            'sub3': 'val3'
        }
    }
    self.assertEqual(props['params'], expected_params)
    env.write_property('params.arrArgs.1', 'newarr1')
    props = props_to_dictionary(env)
    self.assertEqual(props['params']['arrArgs']['1'], 'newarr1')
    env.write_property('params.arg1', 'false')

    self.assertIsNotNone(props['func'])
    self.assertIsNotNone(props['func']['times10'])

    for i in six.moves.range(10):
      self.assertEqual(env.read_property('func.times10.' + str(i)), str(i * 10))

  def test_property_exceptions(self):
    env = deepmind_lab.Lab('tests/properties_test', [], config={})

    # Check works.
    self.assertEqual(env.read_property('params.arg1'), 'true')

    # Attempt to write to list.
    with self.assertRaises(TypeError) as context:
      env.write_property('params', 'val')
    self.assertIn('\'params\' not writable.', str(context.exception))

    # Attempt to write to missing.
    with self.assertRaises(KeyError) as context:
      env.write_property('unknown', 'val')
    self.assertIn('\'unknown\' not found.', str(context.exception))

    # Attempt to write wrong type.
    with self.assertRaises(TypeError) as context:
      env.write_property('params.arg1', 'not_bool')
    self.assertIn('Type error! Cannot assign \'not_bool\' to \'params.arg1\'',
                  str(context.exception))

    # Attempt to read list.
    with self.assertRaises(TypeError) as context:
      env.read_property('params')
    self.assertIn('\'params\' not readable.', str(context.exception))

    # Attempt to read missing.
    with self.assertRaises(KeyError) as context:
      env.read_property('unknown')
    self.assertIn('\'unknown\' not found.', str(context.exception))

    # Attempt to read wrong type.
    with self.assertRaises(KeyError) as context:
      env.read_property('func.times10.not_number')
    self.assertIn('\'func.times10.not_number\' not found.',
                  str(context.exception))

    # Attempt to list value.
    with self.assertRaises(TypeError) as context:
      env.properties('params.arg1')
    self.assertIn('\'params.arg1\' not listable.', str(context.exception))

    # Attempt to list to missing.
    with self.assertRaises(KeyError) as context:
      env.properties('unknown')
    self.assertIn('\'unknown\' not found.', str(context.exception))

  def test_properties_doors(self):
    env = deepmind_lab.Lab(
        'contributed/dmlab30/explore_obstructed_goals_small', [], config={})
    self.assertEqual(env.read_property('params.episodeLengthSeconds'), '90')
    env.reset(seed=4)
    entity = ('***********\n'
              '*** I  P***\n'
              '***H* PG***\n'
              '*PPGIP  ***\n'
              '*GPP* PG***\n'
              '*PGG*GGPI *\n'
              '*H*H*H*H*H*\n'
              '* *GPGIPPG*\n'
              '* *GGP*PGP*\n'
              '* IPPP*GPG*\n'
              '***********\n')
    variations = ('...........\n'
                  '.....AAA...\n'
                  '.....AAA...\n'
                  '.AAA.AAA...\n'
                  '.AAA.AAA...\n'
                  '.AAA.AAA...\n'
                  '...........\n'
                  '...AAA.AAA.\n'
                  '...AAA.AAA.\n'
                  '...AAA.AAA.\n'
                  '...........\n')
    self.assertNotEqual(env.read_property('params.currentEntityLayer'), entity)
    self.assertNotEqual(
        env.read_property('params.currentVariationsLayer'), variations)

    # Enable override:
    env.write_property('params.overrideEntityLayer', entity)
    env.write_property('params.overrideVariationsLayer', variations)
    env.reset(seed=1)
    self.assertEqual(env.read_property('params.currentEntityLayer'), entity)
    self.assertEqual(
        env.read_property('params.currentVariationsLayer'), variations)
    env.reset(seed=2)

    # Make sure override holds:
    self.assertEqual(env.read_property('params.currentEntityLayer'), entity)
    self.assertEqual(
        env.read_property('params.currentVariationsLayer'), variations)

    # Disable override
    env.write_property('params.overrideEntityLayer', '')
    env.write_property('params.overrideVariationsLayer', '')

    env.reset(seed=3)
    self.assertNotEqual(env.read_property('params.currentEntityLayer'), entity)
    self.assertNotEqual(
        env.read_property('params.currentVariationsLayer'), variations)

    entity200 = env.read_property('func.entityLayer.200')
    variations200 = env.read_property('func.variationsLayer.200')

    entity400 = env.read_property('func.entityLayer.400')
    variations400 = env.read_property('func.variationsLayer.400')

    self.assertNotEqual(entity200, entity400)
    self.assertNotEqual(variations200, variations400)


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
