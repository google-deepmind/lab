// Copyright (C) 2016 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////

#include "deepmind/tensor/tensor_view.h"

#include <random>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace tensor {
namespace {

using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;
using ::testing::HasSubstr;

template <typename T>
std::vector<T> MakeSequence(std::size_t num_elements) {
  std::vector<T> result;
  result.reserve(num_elements);
  for (std::size_t i = 0; i < num_elements; ++i) {
    result.push_back(static_cast<T>(i));
  }
  return result;
}

TEST(TensorViewTest, Layout) {
  ShapeVector shape = {4, 3};
  std::vector<unsigned char> storage(Layout::num_elements(shape));
  TensorView<unsigned char> byte_tensor_view(Layout(shape), storage.data());
  EXPECT_THAT(byte_tensor_view.stride(), ElementsAre(3, 1));

  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      std::size_t offset;
      ASSERT_TRUE(byte_tensor_view.GetOffset({i, j}, &offset));
      EXPECT_EQ(i * 3 + j, offset);
    }
  }
  EXPECT_THAT(byte_tensor_view.shape(), ElementsAre(4, 3));
  EXPECT_TRUE(byte_tensor_view.IsContiguous());
  EXPECT_TRUE(byte_tensor_view.Transpose(0, 1));
  EXPECT_THAT(byte_tensor_view.shape(), ElementsAre(3, 4));
  EXPECT_FALSE(byte_tensor_view.IsContiguous());
  EXPECT_FALSE(byte_tensor_view.Transpose(1, 2));
  EXPECT_THAT(byte_tensor_view.stride(), ElementsAre(1, 3));
}

TEST(TensorViewTest, NonContiguous) {
  ShapeVector shape = {7, 5, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> int_tensor_view(Layout(shape), storage.data());
  ASSERT_TRUE(int_tensor_view.Narrow(0, 1, 5));
  EXPECT_THAT(int_tensor_view.shape(), ElementsAre(5, 5, 2));
  ASSERT_TRUE(int_tensor_view.Narrow(1, 1, 3));
  EXPECT_THAT(int_tensor_view.shape(), ElementsAre(5, 3, 2));
  ASSERT_TRUE(int_tensor_view.Transpose(0, 1));
  EXPECT_THAT(int_tensor_view.shape(), ElementsAre(3, 5, 2));
  ASSERT_TRUE(int_tensor_view.Select(2, 1));
  EXPECT_THAT(int_tensor_view.shape(), ElementsAre(3, 5));
  EXPECT_FALSE(int_tensor_view.IsContiguous());

  int_tensor_view.ForEachOffset([&storage](std::size_t offset) {
    EXPECT_EQ(static_cast<std::size_t>(storage[offset]), offset);
  });
}

TEST(TensorViewTest, Reverse) {
  ShapeVector shape = {5, 5};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> int_tensor_view(Layout(shape), storage.data());

  ShapeVector index = {0, 0};
  std::size_t offset;

  ASSERT_TRUE(int_tensor_view.Reverse(1));

  EXPECT_TRUE(int_tensor_view.GetOffset({0, 0}, &offset));
  EXPECT_EQ(4, offset);
  EXPECT_TRUE(int_tensor_view.GetOffset({0, 4}, &offset));
  EXPECT_EQ(0, offset);

  EXPECT_TRUE(int_tensor_view.GetOffset({4, 0}, &offset));
  EXPECT_EQ(24, offset);
  EXPECT_TRUE(int_tensor_view.GetOffset({4, 4}, &offset));
  EXPECT_EQ(20, offset);

  ASSERT_TRUE(int_tensor_view.Reverse(0));

  EXPECT_TRUE(int_tensor_view.GetOffset({0, 0}, &offset));
  EXPECT_EQ(24, offset);
  EXPECT_TRUE(int_tensor_view.GetOffset({0, 4}, &offset));
  EXPECT_EQ(20, offset);

  EXPECT_TRUE(int_tensor_view.GetOffset({4, 0}, &offset));
  EXPECT_EQ(4, offset);
  EXPECT_TRUE(int_tensor_view.GetOffset({4, 4}, &offset));
  EXPECT_EQ(0, offset);

  ASSERT_TRUE(int_tensor_view.Reverse(1));

  EXPECT_TRUE(int_tensor_view.GetOffset({0, 0}, &offset));
  EXPECT_EQ(20, offset);
  EXPECT_TRUE(int_tensor_view.GetOffset({0, 4}, &offset));
  EXPECT_EQ(24, offset);

  EXPECT_TRUE(int_tensor_view.GetOffset({4, 0}, &offset));
  EXPECT_EQ(0, offset);
  EXPECT_TRUE(int_tensor_view.GetOffset({4, 4}, &offset));
  EXPECT_EQ(4, offset);

  ASSERT_TRUE(int_tensor_view.Reverse(0));

  int_tensor_view.ForEachIndexed([](const ShapeVector& id, int value) {
    EXPECT_EQ(id[0] * 5 + id[1], value);
  });

  ASSERT_TRUE(int_tensor_view.Reverse(0));

  int_tensor_view.ForEachIndexed([](const ShapeVector& id, int value) {
    EXPECT_EQ((4 - id[0]) * 5 + id[1], value);
  });

  ASSERT_TRUE(int_tensor_view.Reverse(1));

  int_tensor_view.ForEachIndexed([](const ShapeVector& id, int value) {
    EXPECT_EQ((4 - id[0]) * 5 + 4 - id[1], value);
  });

  ASSERT_TRUE(int_tensor_view.Reverse(0));

  int_tensor_view.ForEachIndexed([](const ShapeVector& id, int value) {
    EXPECT_EQ(id[0] * 5 + 4 - id[1], value);
  });

  ASSERT_FALSE(int_tensor_view.Reverse(3));
}

TEST(TensorViewTest, Reshape) {
  ShapeVector shape = {3, 8};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  EXPECT_TRUE(view.Reshape({4, 6}));
  EXPECT_THAT(view.stride(), ElementsAre(6, 1));
  float counter = 0.0f;
  view.ForEach([&counter](float v) { if (v == counter) ++counter; });
  EXPECT_EQ(24.0f, counter);
  EXPECT_FALSE(view.Reshape({6, 6}));
}

TEST(TensorViewTest, ForEach) {
  ShapeVector shape = {5, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  EXPECT_EQ(10U, view.num_elements());
  EXPECT_EQ(0.0f, view.storage()[0]);
  EXPECT_EQ(5.0f, view.storage()[5]);

  float counter = 0.0f;
  view.ForEach([&counter](float v) { if (v == counter) ++counter; });
  EXPECT_EQ(10U, view.num_elements());
  EXPECT_TRUE(view.Transpose(0, 1));
  counter = 0.0f;
  view.ForEach([&counter](float v) { if (v == counter) ++counter; });
  EXPECT_NE(10.0f, counter);
  counter = 0.0f;
  view.ForEachMutable([&counter](float* v) { *v = counter++; });
  counter = 0.0f;
  view.ForEach([&counter](float v) { if (v == counter) ++counter; });
  EXPECT_EQ(10.0f, counter);
}

TEST(TensorViewTest, ForEachIndexed) {
  ShapeVector shape = {5, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.ForEachIndexed([](const ShapeVector& index, std::size_t v) {
    EXPECT_EQ(2U, index.size());
    EXPECT_EQ(v / 2, index[0]);
    EXPECT_EQ(v % 2, index[1]);
  });
}

TEST(TensorViewTest, ForEachIndexedMutable) {
  ShapeVector shape = {5, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view(Layout(std::move(shape)), storage.data());
  view.ForEachIndexedMutable([](const ShapeVector& index, int* v) {
    EXPECT_THAT(index, ElementsAre(*v / 2, *v % 2));
    *v = index[1];
  });
  EXPECT_THAT(storage, ElementsAre(0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
}

TEST(TensorViewTest, TestAssign) {
  ShapeVector shape1 = {3, 4};
  std::vector<float> storage1(Layout::num_elements(shape1));
  TensorView<float> view1(Layout(std::move(shape1)), storage1.data());
  view1.Assign(2.0);

  ShapeVector shape2 = {2, 6};
  std::vector<float> storage2 =
      MakeSequence<float>(Layout::num_elements(shape2));
  TensorView<float> view2(Layout(std::move(shape2)), storage2.data());
  ASSERT_TRUE(view1.CAssign(view2));
  EXPECT_EQ(storage1, storage2);
}

TEST(TensorViewTest, TestAssignNonContig) {
  ShapeVector shape1 = {6, 6};
  std::vector<float> storage1(Layout::num_elements(shape1));
  TensorView<float> view1(Layout(std::move(shape1)), storage1.data());
  view1.Assign(2.0);

  ShapeVector shape2 = {6, 6};
  std::vector<float> storage2 =
      MakeSequence<float>(Layout::num_elements(shape2));
  TensorView<float> view2(Layout(std::move(shape2)), storage2.data());

  ASSERT_TRUE(view2.Narrow(0, 2, 3));
  ASSERT_TRUE(view2.Narrow(1, 2, 3));

  ASSERT_TRUE(view1.Narrow(0, 2, 3));
  ASSERT_TRUE(view1.Narrow(1, 2, 3));

  ASSERT_TRUE(view2.CAssign(view1));
  view2.ForEach([] (float val) {
    EXPECT_EQ(val, 2.0f);
  });
}

TEST(TensorViewTest, TestAdd) {
  ShapeVector shape = {2, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view(Layout(std::move(shape)), storage.data());
  view.Add(1);
  EXPECT_THAT(storage, ElementsAre(1, 2, 3, 4));
  ASSERT_TRUE(view.CAdd(view));
  EXPECT_THAT(storage, ElementsAre(2, 4, 6, 8));
}

TEST(TensorViewTest, TestMul) {
  ShapeVector shape = {2, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view(Layout(std::move(shape)), storage.data());
  view.Add(1);
  view.Mul(2);
  EXPECT_THAT(storage, ElementsAre(2, 4, 6, 8));
  ASSERT_TRUE(view.CMul(view));
  EXPECT_THAT(storage, ElementsAre(4, 16, 36, 64));
}

TEST(TensorViewTest, TestDiv) {
  ShapeVector shape = {2, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Add(1);
  view.Div(2);
  EXPECT_THAT(storage, ElementsAre(0.5f, 1.0f, 1.5f, 2.0f));
  ASSERT_TRUE(view.CDiv(view));
  EXPECT_THAT(storage, ElementsAre(1.0f, 1.0f, 1.0f, 1.0f));
}

TEST(TensorViewTest, TestSub) {
  ShapeVector shape = {2, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Sub(2);
  EXPECT_THAT(storage, ElementsAre(-2.0, -1.0, 0.0f, 1.0f));
  ASSERT_TRUE(view.CSub(view));
  EXPECT_THAT(storage, ElementsAre(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST(TensorViewTest, TestGet) {
  ShapeVector shape = {5, 5};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  float value;
  ASSERT_TRUE(view.Get({1, 1}, &value));
  EXPECT_EQ(6.0f, value);
  ASSERT_FALSE(view.Get({1, 5}, &value));
  ASSERT_TRUE(view.Select(1, 4));
  for (int i = 0; i < 5; ++i) {
    ASSERT_TRUE(view.Get(i, &value));
    EXPECT_EQ(5.0f * i + 4.0f, value);
  }
  ASSERT_FALSE(view.Get(5, &value));
}

TEST(TensorViewTest, TestMMulFloat) {
  ShapeVector lhs_shape = {2, 2};
  std::vector<float> lhs_storage =
      MakeSequence<float>(Layout::num_elements(lhs_shape));
  TensorView<float> lhs_view(Layout(std::move(lhs_shape)), lhs_storage.data());
  ShapeVector rhs_shape = {2, 3};
  std::vector<float> rhs_storage =
      MakeSequence<float>(Layout::num_elements(rhs_shape));
  TensorView<float> rhs_view(Layout(std::move(rhs_shape)), rhs_storage.data());
  ShapeVector shape = {2, 3};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  ASSERT_TRUE(view.MMul(lhs_view, rhs_view));
  EXPECT_THAT(storage, ElementsAre(3.0f, 4.0f, 5.0f, 9.0f, 14.0f, 19.0f));
}

TEST(TensorViewTest, TestMMulByte) {
  ShapeVector lhs_shape = {2, 2};
  std::vector<char> lhs_storage =
      MakeSequence<char>(Layout::num_elements(lhs_shape));
  TensorView<char> lhs_view(Layout(std::move(lhs_shape)), lhs_storage.data());
  ShapeVector rhs_shape = {2, 3};
  std::vector<char> rhs_storage =
      MakeSequence<char>(Layout::num_elements(rhs_shape));
  TensorView<char> rhs_view(Layout(std::move(rhs_shape)), rhs_storage.data());
  ShapeVector shape = {2, 3};
  std::vector<char> storage = MakeSequence<char>(Layout::num_elements(shape));
  TensorView<char> view(Layout(std::move(shape)), storage.data());
  ASSERT_TRUE(view.MMul(lhs_view, rhs_view));
  EXPECT_THAT(storage, ElementsAre(3, 4, 5, 9, 14, 19));
}

TEST(TensorViewTest, TestMMulOverlap) {
  ShapeVector lhs_shape = {2, 2};
  std::vector<float> lhs_storage =
      MakeSequence<float>(Layout::num_elements(lhs_shape));
  TensorView<float> lhs_view(Layout(std::move(lhs_shape)), lhs_storage.data());
  ShapeVector rhs_shape = {2, 2};
  std::vector<float> rhs_storage =
      MakeSequence<float>(Layout::num_elements(rhs_shape));
  TensorView<float> rhs_view(Layout(std::move(rhs_shape)), rhs_storage.data());
  ASSERT_TRUE(lhs_view.MMul(lhs_view, rhs_view));
  EXPECT_THAT(lhs_storage, ElementsAre(2, 3, 6, 11));
}

TEST(TensorViewTest, TestMMulOverlapNarrow) {
  ShapeVector lhs_shape = {2, 3};
  std::vector<float> lhs_storage =
      MakeSequence<float>(Layout::num_elements(lhs_shape));
  TensorView<float> lhs_view(Layout(std::move(lhs_shape)), lhs_storage.data());
  ASSERT_TRUE(lhs_view.Narrow(1, 1, 2));
  ASSERT_EQ(lhs_view.shape()[0], 2);
  ASSERT_EQ(lhs_view.shape()[1], 2);
  ShapeVector rhs_shape = {2, 2};
  std::vector<float> rhs_storage =
      MakeSequence<float>(Layout::num_elements(rhs_shape));
  TensorView<float> rhs_view(Layout(std::move(rhs_shape)), rhs_storage.data());
  ASSERT_TRUE(lhs_view.MMul(lhs_view, rhs_view));
  EXPECT_THAT(lhs_storage, ElementsAre(0, 4, 7, 3, 10, 19));
}

TEST(TensorViewTest, TestFloor) {
  ShapeVector shape = {2, 2};
  std::vector<float> storage = {-2.25, -1.75, 0.5, 1.0};
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Floor();
  EXPECT_THAT(storage, ElementsAre(-3.0, -2.0, 0.0, 1.0));
}

TEST(TensorViewTest, TestCeil) {
  ShapeVector shape = {2, 2};
  std::vector<float> storage = {-2.25, -1.75, 0.5, 1.0};
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Ceil();
  EXPECT_THAT(storage, ElementsAre(-2.0, -1.0, 1.0, 1.0));
}

TEST(TensorViewTest, TestRound) {
  ShapeVector shape = {2, 2};
  std::vector<float> storage = {-2.25, -1.75, 0.5, 1.0};
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Round();
  EXPECT_THAT(storage, ElementsAre(-2.0, -2.0, 1.0, 1.0));
}

TEST(TensorViewTest, TestSum) {
  ShapeVector shape = {4};
  std::vector<int> storage = {1, 2, 3, 4};
  TensorView<int> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Sum(), 10);
}

TEST(TensorViewTest, TestSumEmpty) {
  ShapeVector shape = {0};
  std::vector<int> storage = {};
  TensorView<int> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Sum(), 0);
}

TEST(TensorViewTest, TestSumByteToInt) {
  ShapeVector shape = {3};
  std::vector<unsigned char> storage = {255, 255, 255};
  TensorView<unsigned char> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Sum<int>(), 255 + 255 + 255);
}

TEST(TensorViewTest, TestSumSignedCharToInt) {
  ShapeVector shape = {3};
  std::vector<signed char> storage = {-128, -100, -50};
  TensorView<signed char> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Sum<int>(), -128 - 100 - 50);
}

TEST(TensorViewTest, TestProduct) {
  ShapeVector shape = {4};
  std::vector<int> storage = {2, 3, 5, 7};
  TensorView<int> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Product(), 2 * 3 * 5 * 7);
}

TEST(TensorViewTest, TestProductEmpty) {
  ShapeVector shape = {0};
  std::vector<int> storage = {};
  TensorView<int> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Product(), 1);
}

TEST(TensorViewTest, TestProductByteToInt) {
  ShapeVector shape = {4};
  std::vector<unsigned char> storage = {16, 32, 64, 128};
  TensorView<unsigned char> view(Layout(shape), storage.data());
  EXPECT_EQ(view.Product<int>(), 16 * 32 * 64 * 128);
}

TEST(TensorViewTest, TestLengthSquard) {
  ShapeVector shape = {2};
  std::vector<int> storage = {5, 5};
  TensorView<int> view(Layout(shape), storage.data());
  EXPECT_EQ(view.LengthSquared(), 25 + 25);
}

TEST(TensorViewTest, TestLengthSquardEmpty) {
  ShapeVector shape = {0};
  std::vector<int> storage = {};
  TensorView<int> view(Layout(shape), storage.data());
  EXPECT_EQ(view.LengthSquared(), 0);
}

TEST(TensorViewTest, TestLengthSquardByteToInt) {
  ShapeVector shape = {2};
  std::vector<unsigned char> storage = {255, 128};
  TensorView<unsigned char> view(Layout(shape), storage.data());
  EXPECT_EQ(view.LengthSquared<int>(), 255 * 255 + 128 * 128);
}

TEST(TensorViewTest, TestLengthSquardSignedCharToInt) {
  ShapeVector shape = {2};
  std::vector<signed char> storage = {-120, 100};
  TensorView<signed char> view(Layout(shape), storage.data());
  EXPECT_EQ(view.LengthSquared<int>(), 120 * 120 + 100 * 100);
}

TEST(TensorViewTest, TestDotProduct) {
  ShapeVector shape = {2};
  std::vector<int> storage1 = {-2, 3};
  TensorView<int> view1(Layout(shape), storage1.data());
  std::vector<int> storage2 = {2, 5};
  TensorView<int> view2(Layout(shape), storage2.data());
  int result;
  EXPECT_TRUE(view1.DotProduct(view2, &result));
  EXPECT_EQ(result, 11);
}

TEST(TensorViewTest, TestDotProductNoOverflow) {
  ShapeVector shape = {2};
  std::vector<signed char> storage1 = {-20, 30};
  TensorView<signed char> view1(Layout(shape), storage1.data());
  std::vector<signed char> storage2 = {20, 50};
  TensorView<signed char> view2(Layout(shape), storage2.data());
  int result;
  EXPECT_TRUE(view1.DotProduct(view2, &result));
  EXPECT_EQ(result, 1100);
}

TEST(TensorViewTest, TestDotProductSizeMismatch) {
  ShapeVector shape1 = {2};
  std::vector<signed char> storage1 = {-20, 30};
  TensorView<signed char> view1(Layout(shape1), storage1.data());
  ShapeVector shape2 = {3};
  std::vector<signed char> storage2 = {20, 50, 3};
  TensorView<signed char> view2(Layout(shape2), storage2.data());
  int result;
  EXPECT_FALSE(view1.DotProduct(view2, &result));
}

TEST(TensorViewTest, TestShuffle) {
  ShapeVector shape = {5};
  std::vector<float> storage = {1.0, 2.0, 3.0, 4.0, 5.0};
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  std::mt19937_64 prbg(123);
  EXPECT_TRUE(view.Shuffle(&prbg));
  EXPECT_THAT(storage, UnorderedElementsAre(1.0, 2.0, 3.0, 4.0, 5.0));
}

TEST(TensorViewTest, TestShuffleEmpty) {
  ShapeVector shape = {0};
  std::vector<float> storage = {};
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  std::mt19937_64 prbg(123);
  EXPECT_TRUE(view.Shuffle(&prbg));
  EXPECT_TRUE(storage.empty());
}

TEST(TensorViewTest, TestSet) {
  ShapeVector shape = {3, 3};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  ASSERT_TRUE(view.Set({1, 1}, 555.0f));
  ASSERT_FALSE(view.Set({1, 3}, 222.0f));
  ASSERT_TRUE(view.Select(1, 2));
  for (int i = 0; i < 3; ++i) {
    ASSERT_TRUE(view.Set(i, 33.0));
  }
  ASSERT_FALSE(view.Set(3, 222.0f));
  EXPECT_THAT(storage, ElementsAre(0.0f, 1.0f, 33.0f,    //
                                   3.0f, 555.0f, 33.0f,  //
                                   6.0f, 7.0f, 33.0f));
}

TEST(TensorViewTest, TestEqual) {
  ShapeVector shape = {2, 8};
  std::vector<int> storage1 =
      MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view1(Layout(shape), storage1.data());
  std::vector<int> storage2 = storage1;
  TensorView<int> view2(Layout(std::move(shape)), storage2.data());
  EXPECT_TRUE(view1 == view2);
  ASSERT_TRUE(view1.Reshape({4, 4}));
  EXPECT_FALSE(view1 == view2);
  ASSERT_TRUE(view2.Reshape({4, 4}));
  EXPECT_TRUE(view1 == view2);
  view1.Select(1, 0);
  EXPECT_FALSE(view1 == view2);
  view2.Select(0, 1);
  EXPECT_FALSE(view1 == view2);
}

TEST(TensorViewTest, TestStream) {
  ShapeVector shape = {4, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  {
    std::stringstream ss;
    ss << view;
    const auto& result = ss.str();
    EXPECT_THAT(result, HasSubstr("4, 2"));
    EXPECT_THAT(result, HasSubstr("[[0, 1]"));
    EXPECT_THAT(result, HasSubstr(" [2, 3]"));
    EXPECT_THAT(result, HasSubstr(" [4, 5]"));
    EXPECT_THAT(result, HasSubstr(" [6, 7]]"));
  }
  {
    view.Add(6.0f);
    std::stringstream ss;
    ss << view;
    const auto& result = ss.str();
    EXPECT_THAT(result, HasSubstr("4, 2"));
    EXPECT_THAT(result, HasSubstr("[[ 6,  7]"));
    EXPECT_THAT(result, HasSubstr(" [ 8,  9]"));
    EXPECT_THAT(result, HasSubstr(" [10, 11]"));
    EXPECT_THAT(result, HasSubstr(" [12, 13]]"));
  }
  {
    std::stringstream ss;
    TensorView<float> view_empty(Layout({0}), nullptr);
    ss << view_empty;
    const auto& result_empty = ss.str();
    EXPECT_THAT(result_empty, HasSubstr("0"));
  }
  {
    std::stringstream ss;
    float value = 555.0f;
    TensorView<float> view_single(Layout({1}), &value);
    ss << view_single;
    const auto& result_single = ss.str();
    EXPECT_THAT(result_single, HasSubstr("555"));
  }
  {
    std::stringstream ss;
    ShapeVector shape_large = {7, 7, 7, 7};
    std::vector<float> storage_large =
        MakeSequence<float>(Layout::num_elements(shape_large));
    TensorView<float> view_large(Layout(std::move(shape_large)),
                                 storage_large.data());
    ss << view_large;
    const auto& result_large = ss.str();
    EXPECT_THAT(result_large, HasSubstr("..."));
  }
}

}  // namespace
}  // namespace tensor
}  // namespace lab
}  // namespace deepmind
