// Copyright (C) 2017 Google Inc.
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

#include <cmath>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/model_generation/geometry_cone.h"
#include "deepmind/model_generation/geometry_cube.h"
#include "deepmind/model_generation/geometry_cylinder.h"
#include "deepmind/model_generation/geometry_sphere.h"
#include "deepmind/model_generation/geometry_util.h"

namespace deepmind {
namespace lab {
namespace geometry {
namespace {

#define VERIFY_EQ(a, b)                                               \
  if ((a) != (b))                                                     \
    return ::testing::AssertionFailure()                              \
           << "\n    Value of: " #b "\n      Actual: " << (b) << "\n" \
           << "    Expected: " #a "\n    Which is: " << (a) << "\n  ";

testing::AssertionResult TestDiskTopology(  //
    int num_sectors,                        //
    int num_tracks,                         //
    int offset,                             //
    std::vector<int>::const_iterator it) {
  for (int j = 0; j < num_sectors; ++j) {
    VERIFY_EQ(*it++, offset + (j + 1) * (num_tracks + 1));
    VERIFY_EQ(*it++, offset + (j + 1) * (num_tracks + 1) - 1);
    VERIFY_EQ(*it++, offset + j * (num_tracks + 1));
    for (int i = 1; i < num_tracks; ++i) {
      VERIFY_EQ(*it++, offset + (j + 1) * (num_tracks + 1) + i);
      VERIFY_EQ(*it++, offset + (j + 1) * (num_tracks + 1) + i - 1);
      VERIFY_EQ(*it++, offset + j * (num_tracks + 1) + i - 1);
      VERIFY_EQ(*it++, offset + j * (num_tracks + 1) + i);
      VERIFY_EQ(*it++, offset + (j + 1) * (num_tracks + 1) + i);
      VERIFY_EQ(*it++, offset + j * (num_tracks + 1) + i - 1);
    }
  }
  return testing::AssertionSuccess();
}

testing::AssertionResult TestRectTopology(  //
    int num_rows,                           //
    int num_cols,                           //
    int offset,                             //
    std::vector<int>::const_iterator it) {
  for (int j = 0; j < num_cols; ++j) {
    for (int i = 0; i < num_rows; ++i) {
      VERIFY_EQ(*it++, offset + j * (num_rows + 1) + i);
      VERIFY_EQ(*it++, offset + (j + 1) * (num_rows + 1) + i);
      VERIFY_EQ(*it++, offset + (j + 1) * (num_rows + 1) + i + 1);
      VERIFY_EQ(*it++, offset + j * (num_rows + 1) + i);
      VERIFY_EQ(*it++, offset + (j + 1) * (num_rows + 1) + i + 1);
      VERIFY_EQ(*it++, offset + j * (num_rows + 1) + i + 1);
    }
  }
  return testing::AssertionSuccess();
}

#undef VERIFY_EQ

TEST(DeepmindGeometryTest, DiskMesh) {
  std::size_t num_vertices, num_triangles;
  ComputeDiskMeshSize(16, 4, &num_vertices, &num_triangles);
  EXPECT_EQ(num_vertices, 84);
  EXPECT_EQ(num_triangles, 112);

  struct MockEvaluator {
    MOCK_METHOD2(eval, std::array<float, 8>(float u, float v));
  } mockEvaluator;

  {
    testing::InSequence s;
    float u, v;
    u = 0.0f;
    for (int i = 1; i <= 4; ++i) {
      v = i / 4.0f;
      EXPECT_CALL(mockEvaluator, eval(u, v));
    }
    for (int j = 1; j <= 16; ++j) {
      u = (j - 0.5f) / 16.0f;
      v = 0.0f;
      EXPECT_CALL(mockEvaluator, eval(u, v));
      u = j / 16.0f;
      for (int i = 1; i < 4; ++i) {
        v = i / 4.0f;
        EXPECT_CALL(mockEvaluator, eval(u, v));
      }
      v = 1.0f;
      EXPECT_CALL(mockEvaluator, eval(u, v));
    }
  }

  std::vector<float> vertices;
  std::vector<int> indices;
  BuildDiskMesh(16, 4, 0,
                [&mockEvaluator](float u, float v) -> std::array<float, 8> {
                  return mockEvaluator.eval(u, v);
                },
                &vertices, &indices);
  EXPECT_TRUE(TestDiskTopology(16, 4, 0, indices.begin()));
}

TEST(DeepmindGeometryTest, RectMesh) {
  std::size_t num_vertices, num_triangles;
  ComputeRectMeshSize(4, 5, &num_vertices, &num_triangles);
  EXPECT_EQ(num_vertices, 30);
  EXPECT_EQ(num_triangles, 40);

  struct MockEvaluator {
    MOCK_METHOD2(eval, std::array<float, 8>(float u, float v));
  } mockEvaluator;

  {
    testing::InSequence s;
    for (int j = 0; j <= 5; ++j) {
      float u = j / 5.0f;
      for (int i = 0; i <= 4; ++i) {
        float v = i / 4.0f;
        EXPECT_CALL(mockEvaluator, eval(u, v));
      }
    }
  }

  std::vector<float> vertices;
  std::vector<int> indices;
  BuildRectMesh(4, 5, 0,
                [&mockEvaluator](float u, float v) -> std::array<float, 8> {
                  return mockEvaluator.eval(u, v);
                },
                &vertices, &indices);
  EXPECT_TRUE(TestRectTopology(4, 5, 0, indices.begin()));
}

TEST(DeepmindGeometryTest, ConeSurface) {
  Cone cone;
  cone.num_phi_segments = 4;
  cone.num_radius_segments = 4;
  cone.num_height_segments = 4;
  Model::Surface surf = CreateSurface(cone);
  ASSERT_EQ(surf.vertices.size(), 168 * 8);
  ASSERT_EQ(surf.indices.size(), 224 * 3);
  const float nz = 1.0f / std::sqrt(5.0f);
  EXPECT_NEAR(surf.vertices[3 * 8 + 5], nz, kEpsilon);
  EXPECT_NEAR(surf.vertices[4 * 8 + 5], nz, kEpsilon);
  EXPECT_NEAR(surf.vertices[83 * 8 + 5], nz, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 5], -1.0f, kEpsilon);
  EXPECT_TRUE(TestDiskTopology(16, 4, 0, surf.indices.begin()));
  EXPECT_TRUE(TestDiskTopology(16, 4, 84, surf.indices.begin() + 112 * 3));
}

TEST(DeepmindGeometryTest, ConeLocators) {
  Cone cone;
  cone.num_phi_segments = 4;
  cone.num_radius_segments = 4;
  cone.num_height_segments = 4;
  Model::LocatorMap locators = CreateLocators(cone);
  EXPECT_EQ(locators.size(), 27 * 2);
  auto it = locators.find("centre_top_left_p");
  ASSERT_NE(it, locators.end());
  const float nz = 1.0f / std::sqrt(5.0f);
  EXPECT_NEAR(it->second(0, 2), -2.0f * nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f, kEpsilon);
  it = locators.find("front_bottom_centre_p");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 2.0f * nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), -nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.5f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), -0.5f, kEpsilon);
  it = locators.find("centre_bottom_centre_s");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), 1.0f, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), -0.5f, kEpsilon);
}

TEST(DeepmindGeometryTest, CubeSurface) {
  Cube cube;
  cube.num_width_segments = 4;
  cube.num_depth_segments = 4;
  cube.num_height_segments = 4;
  Model::Surface surf = CreateSurface(cube);
  ASSERT_EQ(surf.vertices.size(), 150 * 8);
  ASSERT_EQ(surf.indices.size(), 192 * 3);
  EXPECT_NEAR(surf.vertices[3], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[25 * 8 + 3], -1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[25 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[25 * 8 + 5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[50 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[50 * 8 + 4], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[50 * 8 + 5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[75 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[75 * 8 + 4], -1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[75 * 8 + 5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[100 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[100 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[100 * 8 + 5], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[125 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[125 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[125 * 8 + 5], -1.0f, kEpsilon);
  EXPECT_TRUE(TestRectTopology(4, 4, 0, surf.indices.begin()));
  EXPECT_TRUE(TestRectTopology(4, 4, 25, surf.indices.begin() + 32 * 3));
  EXPECT_TRUE(TestRectTopology(4, 4, 50, surf.indices.begin() + 64 * 3));
  EXPECT_TRUE(TestRectTopology(4, 4, 75, surf.indices.begin() + 96 * 3));
  EXPECT_TRUE(TestRectTopology(4, 4, 100, surf.indices.begin() + 128 * 3));
  EXPECT_TRUE(TestRectTopology(4, 4, 125, surf.indices.begin() + 160 * 3));
}

TEST(DeepmindGeometryTest, CubeLocators) {
  Cube cube;
  cube.num_width_segments = 4;
  cube.num_depth_segments = 4;
  cube.num_height_segments = 4;
  Model::LocatorMap locators = CreateLocators(cube);
  EXPECT_EQ(locators.size(), 27 * 2);
  auto it = locators.find("centre_top_left_p");
  ASSERT_NE(it, locators.end());
  const float nz = 1.0f / std::sqrt(2.0f);
  EXPECT_NEAR(it->second(0, 2), -nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), -0.5f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f, kEpsilon);
  it = locators.find("front_top_centre_p");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.5f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f, kEpsilon);
  it = locators.find("centre_bottom_centre_s");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), 1.0f, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), -0.5f, kEpsilon);
}

TEST(DeepmindGeometryTest, CylinderSurface) {
  Cylinder cylinder;
  cylinder.num_phi_segments = 4;
  cylinder.num_radius_segments = 4;
  cylinder.num_height_segments = 4;
  Model::Surface surf = CreateSurface(cylinder);
  ASSERT_EQ(surf.vertices.size(), 253 * 8);
  ASSERT_EQ(surf.indices.size(), 352 * 3);
  EXPECT_NEAR(surf.vertices[3], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[80 * 8 + 3], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[80 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[80 * 8 + 5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 5], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[172 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[172 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[172 * 8 + 5], -1.0f, kEpsilon);
  EXPECT_TRUE(TestRectTopology(4, 16, 0, surf.indices.begin()));
  EXPECT_TRUE(TestDiskTopology(16, 4, 85, surf.indices.begin() + 128 * 3));
  EXPECT_TRUE(TestDiskTopology(16, 4, 169, surf.indices.begin() + 240 * 3));
}

TEST(DeepmindGeometryTest, CylinderLocators) {
  Cylinder cylinder;
  cylinder.num_phi_segments = 4;
  cylinder.num_radius_segments = 4;
  cylinder.num_height_segments = 4;
  Model::LocatorMap locators = CreateLocators(cylinder);
  EXPECT_EQ(locators.size(), 27 * 2);
  auto it = locators.find("centre_top_left_p");
  ASSERT_NE(it, locators.end());
  const float nz = 1.0f / std::sqrt(2.0f);
  EXPECT_NEAR(it->second(0, 2), -nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), -0.5f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f, kEpsilon);
  it = locators.find("front_top_centre_p");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.5f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f, kEpsilon);
  it = locators.find("centre_bottom_centre_s");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), 1.0f, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), -0.5f, kEpsilon);
}

TEST(DeepmindGeometryTest, SphereSurface) {
  Sphere sphere;
  sphere.num_phi_segments = 4;
  sphere.num_theta_segments = 4;
  Model::Surface surf = CreateSurface(sphere);
  ASSERT_EQ(surf.vertices.size(), 168 * 8);
  ASSERT_EQ(surf.indices.size(), 224 * 3);
  EXPECT_NEAR(surf.vertices[3 * 8 + 3], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[3 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[3 * 8 + 5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[4 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[4 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[4 * 8 + 5], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[83 * 8 + 3], 1.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[83 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[83 * 8 + 5], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 3], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 4], 0.0f, kEpsilon);
  EXPECT_NEAR(surf.vertices[88 * 8 + 5], -1.0f, kEpsilon);
  EXPECT_TRUE(TestDiskTopology(16, 4, 0, surf.indices.begin()));
  EXPECT_TRUE(TestDiskTopology(16, 4, 84, surf.indices.begin() + 112 * 3));
}

TEST(DeepmindGeometryTest, SphereLocators) {
  Sphere sphere;
  sphere.num_phi_segments = 4;
  sphere.num_theta_segments = 4;
  Model::LocatorMap locators = CreateLocators(sphere);
  EXPECT_EQ(locators.size(), 27 * 2);
  auto it = locators.find("back_top_left_p");
  ASSERT_NE(it, locators.end());
  const float nz = 1.0f / std::sqrt(3.0f);
  EXPECT_NEAR(it->second(0, 2), -nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), -nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), -0.5f * nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), -0.5f * nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f * nz, kEpsilon);
  it = locators.find("front_top_right_p");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), nz, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.5f * nz, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.5f * nz, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), 0.5f * nz, kEpsilon);
  it = locators.find("centre_bottom_centre_s");
  ASSERT_NE(it, locators.end());
  EXPECT_NEAR(it->second(0, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 2), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 2), 1.0f, kEpsilon);
  EXPECT_NEAR(it->second(0, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(1, 3), 0.0f, kEpsilon);
  EXPECT_NEAR(it->second(2, 3), -0.5f, kEpsilon);
}

TEST(DeepmindGeometryTest, ZAlignedFrame) {
  Eigen::Matrix4f ref_data;
  const float kRcpSqrt2 = 1.0f / std::sqrt(2.0f);
  const float kRcpSqrt3 = 1.0f / std::sqrt(3.0f);
  const float kRcpSqrt6 = 1.0f / std::sqrt(6.0f);
  ref_data << kRcpSqrt2, -kRcpSqrt6, kRcpSqrt3, 1.0f,  //
      0.0f, 2.0f * kRcpSqrt6, kRcpSqrt3, 2.0f,         //
      -kRcpSqrt2, -kRcpSqrt6, kRcpSqrt3, 3.0f,         //
      0.0f, 0.0f, 0.0f, 1.0f;
  auto tst_xfrm = CreateZAlignedFrame(
      Eigen::Vector3f(1.0f, 2.0f, 3.0f),
      Eigen::Vector3f(1.0f, 1.0f, 1.0f).normalized(), Eigen::Vector3f::UnitY());
  EXPECT_NEAR((ref_data - tst_xfrm.matrix()).norm(), 0.0f, kEpsilon);
}

TEST(DeepmindGeometryTest, DefaultZDir) {
  auto tst0_zdir = ComputeDefaultZDir(0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f);
  EXPECT_NEAR((tst0_zdir - Eigen::Vector3f::UnitZ()).norm(), 0.0f, kEpsilon);
  auto tst1_zdir = ComputeDefaultZDir(1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 3.0f);
  const float kRcpSqrt14 = 1.0f / std::sqrt(14.0f);  // 1 / norm([1, 2, 3])
  Eigen::Vector3f ref1_zdir(kRcpSqrt14, 2.0f * kRcpSqrt14, 3.0f * kRcpSqrt14);
  EXPECT_NEAR((tst1_zdir - ref1_zdir).norm(), 0.0f, kEpsilon);
}

TEST(DeepmindGeometryTest, DefaultYVector) {
  auto tst0_yvec = ComputeDefaultYVector(0.0f, 0.0f, 1.0f);
  EXPECT_NEAR((tst0_yvec - Eigen::Vector3f::UnitY()).norm(), 0.0f, kEpsilon);
  auto tst1_yvec = ComputeDefaultYVector(0.0f, 0.0f, -1.0f);
  EXPECT_NEAR((tst1_yvec + Eigen::Vector3f::UnitY()).norm(), 0.0f, kEpsilon);
  auto tst2_yvec = ComputeDefaultYVector(1.0f, 1.0f, 1.0f);
  EXPECT_NEAR((tst2_yvec + Eigen::Vector3f::UnitZ()).norm(), 0.0f, kEpsilon);
}

}  // namespace
}  // namespace geometry
}  // namespace lab
}  // namespace deepmind
