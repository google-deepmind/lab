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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace tensor {
namespace {

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
  std::vector<std::size_t> shape = {4, 3};
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
  std::vector<std::size_t> shape = {7, 5, 2};
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

TEST(TensorViewTest, Reshape) {
  std::vector<std::size_t> shape = {3, 8};
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
  std::vector<std::size_t> shape = {5, 2};
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
  std::vector<std::size_t> shape = {5, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.ForEachIndexed([](const std::vector<std::size_t>& index, std::size_t v) {
    EXPECT_EQ(2U, index.size());
    EXPECT_EQ(v / 2, index[0]);
    EXPECT_EQ(v % 2, index[1]);
  });
}

TEST(TensorViewTest, ForEachIndexedMutable) {
  std::vector<std::size_t> shape = {5, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view(Layout(std::move(shape)), storage.data());
  view.ForEachIndexedMutable([](const std::vector<std::size_t>& index, int* v) {
    EXPECT_THAT(index, ElementsAre(*v / 2, *v % 2));
    *v = index[1];
  });
  EXPECT_THAT(storage, ElementsAre(0, 1, 0, 1, 0, 1, 0, 1, 0, 1));
}

TEST(TensorViewTest, TestAssign) {
  std::vector<std::size_t> shape1 = {3, 4};
  std::vector<float> storage1(Layout::num_elements(shape1));
  TensorView<float> view1(Layout(std::move(shape1)), storage1.data());
  view1.Assign(2.0);

  std::vector<std::size_t> shape2 = {2, 6};
  std::vector<float> storage2 =
      MakeSequence<float>(Layout::num_elements(shape2));
  TensorView<float> view2(Layout(std::move(shape2)), storage2.data());
  ASSERT_TRUE(view1.CAssign(view2));
  EXPECT_EQ(storage1, storage2);
}

TEST(TensorViewTest, TestAdd) {
  std::vector<std::size_t> shape = {2, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view(Layout(std::move(shape)), storage.data());
  view.Add(1);
  EXPECT_THAT(storage, ElementsAre(1, 2, 3, 4));
  ASSERT_TRUE(view.CAdd(view));
  EXPECT_THAT(storage, ElementsAre(2, 4, 6, 8));
}

TEST(TensorViewTest, TestMul) {
  std::vector<std::size_t> shape = {2, 2};
  std::vector<int> storage = MakeSequence<int>(Layout::num_elements(shape));
  TensorView<int> view(Layout(std::move(shape)), storage.data());
  view.Add(1);
  view.Mul(2);
  EXPECT_THAT(storage, ElementsAre(2, 4, 6, 8));
  ASSERT_TRUE(view.CMul(view));
  EXPECT_THAT(storage, ElementsAre(4, 16, 36, 64));
}

TEST(TensorViewTest, TestDiv) {
  std::vector<std::size_t> shape = {2, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Add(1);
  view.Div(2);
  EXPECT_THAT(storage, ElementsAre(0.5f, 1.0f, 1.5f, 2.0f));
  ASSERT_TRUE(view.CDiv(view));
  EXPECT_THAT(storage, ElementsAre(1.0f, 1.0f, 1.0f, 1.0f));
}

TEST(TensorViewTest, TestSub) {
  std::vector<std::size_t> shape = {2, 2};
  std::vector<float> storage = MakeSequence<float>(Layout::num_elements(shape));
  TensorView<float> view(Layout(std::move(shape)), storage.data());
  view.Sub(2);
  EXPECT_THAT(storage, ElementsAre(-2.0, -1.0, 0.0f, 1.0f));
  ASSERT_TRUE(view.CSub(view));
  EXPECT_THAT(storage, ElementsAre(0.0f, 0.0f, 0.0f, 0.0f));
}

TEST(TensorViewTest, TestGet) {
  std::vector<std::size_t> shape = {5, 5};
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

TEST(TensorViewTest, TestSet) {
  std::vector<std::size_t> shape = {3, 3};
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
  std::vector<std::size_t> shape = {2, 8};
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
  std::vector<std::size_t> shape = {4, 2};
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
    TensorView<float> view_empty(Layout({}), nullptr);
    ss << view_empty;
    const auto& result_empty = ss.str();
    EXPECT_THAT(result_empty, HasSubstr("Empty"));
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
    std::vector<std::size_t> shape_large = {32, 32};
    std::vector<float> storage_large =
        MakeSequence<float>(Layout::num_elements(shape_large));
    TensorView<float> view_large(Layout(std::move(shape_large)),
                                 storage_large.data());
    ss << view_large;
    const auto& result_large = ss.str();
    EXPECT_THAT(result_large, HasSubstr("Too many elements"));
  }
}

}  // namespace
}  // namespace tensor
}  // namespace lab
}  // namespace deepmind
