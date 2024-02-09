# Unit test for the ConfigItem class
# Verifies the following features
#   - Basic interface
#   - Immutability
#   - Nesting behaviour
#   - Reproducibility of the input dict

import unittest
from unittest import TestCase
from copy import deepcopy

from EasyjetHub.steering.utils.config_flags import ConfigItem, to_immutable


class TestConfigItem(TestCase):
    def setUp(self):
        self.ref_dict = dict(
            a_string='hesitation is defeat',
            a_float=3.1415,
        )

        self.ref_citem = ConfigItem(**self.ref_dict)

        self.nested_dict = deepcopy(self.ref_dict)
        # Repeating keys in the subdict is a good test
        # that we don't bleed information
        # from one instance to another
        self.nested_dict['a_subdict'] = dict(
            a_string='the cake is a lie',
            an_int=125,
        )

        # Explicitly convert subdicts
        # ConfigItem's constructor will not do so
        _subcitem = ConfigItem(**self.nested_dict['a_subdict'])
        _dict_containing_citem = deepcopy(self.ref_dict)
        _dict_containing_citem['a_subdict'] = _subcitem
        self.nested_citem = ConfigItem(**_dict_containing_citem)

        # Exercise the recursive converter
        self.nested_citem_2 = to_immutable(_dict_containing_citem)

        # The constructor does not permit lists as values
        self.dict_holding_list = dict(
            a_sublist=['a', 'b', 'c'],
        )

        self.lists_inside_lists = dict(
            a_sublist=[
                ['a', 'b', 'c'],
                [1, 2, 3],
            ]
        )

        self.dict_inside_lists = dict(
            a_sublist=[
                ['a', 'b', 'c'],
                [dict(a_string='turtles all the way down')],
            ],
        )

    def test_immutable(self):
        with self.assertRaises(TypeError):
            self.ref_citem.a_float = 2.71828
        with self.assertRaises(TypeError):
            self.ref_citem.new_field = 'potato potahto'

    def test_construct_unhashable(self):
        with self.assertRaises(TypeError):
            ConfigItem(**self.dict_holding_list)

    def test_retrieve(self):
        self.assertEqual(self.ref_citem.a_float, self.ref_dict['a_float'])

    def test_interface(self):
        self.assertEqual(set(self.ref_citem.keys()), set(self.ref_dict.keys()))
        self.assertEqual(set(self.ref_citem.values()), set(self.ref_dict.values()))
        self.assertEqual(dict(self.ref_citem.items()), dict(self.ref_dict.items()))

    def test_nested_retrieve(self):
        from_citem = self.nested_citem.a_subdict.an_int
        from_dict = self.nested_dict['a_subdict']['an_int']
        self.assertEqual(from_citem, from_dict)

    def test_to_immutable(self):
        self.assertEqual(self.nested_citem, self.nested_citem_2)

    def test_as_mutable(self):
        self.assertEqual(self.ref_citem.as_mutable(), self.ref_dict)

    def test_nested_as_mutable(self):
        self.assertEqual(self.nested_citem.as_mutable(), self.nested_dict)

    def test_nested_lists_as_mutable(self):
        citem_with_nested_lists = to_immutable(self.lists_inside_lists)
        self.assertEqual(citem_with_nested_lists.as_mutable(), self.lists_inside_lists)

    def test_more_nested_as_mutable(self):
        more_nested_citem = to_immutable(self.dict_inside_lists)
        self.assertEqual(more_nested_citem.as_mutable(), self.dict_inside_lists)


if __name__ == '__main__':
    unittest.main()
