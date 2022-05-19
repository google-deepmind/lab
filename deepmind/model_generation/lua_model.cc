// Copyright (C) 2017-2019 Google Inc.
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

#include "deepmind/model_generation/lua_model.h"

#include <cmath>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/str_cat.h"
#include "deepmind/include/deepmind_calls.h"
#include "deepmind/include/deepmind_model_getters.h"
#include "deepmind/include/deepmind_model_setters.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/model_generation/geometry_cone.h"
#include "deepmind/model_generation/geometry_cube.h"
#include "deepmind/model_generation/geometry_cylinder.h"
#include "deepmind/model_generation/geometry_sphere.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/model_getters.h"
#include "deepmind/model_generation/model_lua.h"
#include "deepmind/model_generation/model_setters.h"
#include "deepmind/model_generation/model_util.h"
#include "deepmind/model_generation/transform.h"
#include "deepmind/model_generation/transform_lua.h"

namespace deepmind {
namespace lab {

const char* LuaModel::ClassName() { return "deepmind.lab.Model"; }

void LuaModel::Register(lua_State* L) {
  const Class::Reg methods[] = {
      {"cone", &Class::Member<&LuaModel::CreateCone>},
      {"cube", &Class::Member<&LuaModel::CreateCube>},
      {"cylinder", &Class::Member<&LuaModel::CreateCylinder>},
      {"sphere", &Class::Member<&LuaModel::CreateSphere>},
      {"hierarchy", &Class::Member<&LuaModel::CreateHierarchy>},
      {"circularLayout", &Class::Member<&LuaModel::CreateCircularLayout>},
      {"linearLayout", &Class::Member<&LuaModel::CreateLinearLayout>},
      {"loadMD3", &Class::Member<&LuaModel::LoadMD3>},
      {"saveMD3", &Class::Member<&LuaModel::SaveMD3>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaModel::CreateCone(lua_State* L) {
  lua::TableRef table;
  if (lua::Read(L, -1, &table)) {
    geometry::Cone cone;
    // Handle both regular and elliptic cones.
    if (table.LookUp("radius", &cone.width_radius)) {
      cone.depth_radius = cone.width_radius;
    } else {
      table.LookUp("widthRadius", &cone.width_radius);
      table.LookUp("depthRadius", &cone.depth_radius);
    }
    table.LookUp("height", &cone.height);
    table.LookUp("phiSegments", &cone.num_phi_segments);
    table.LookUp("radiusSegments", &cone.num_radius_segments);
    table.LookUp("heightSegments", &cone.num_height_segments);
    table.LookUp("shaderName", &cone.shader_name);
    Model model = {"cone", {CreateSurface(cone)}, CreateLocators(cone)};
    Push(L, model);
    return 1;
  }
  return "[model.cone] Must call with argument table.";
}

lua::NResultsOr LuaModel::CreateCube(lua_State* L) {
  lua::TableRef table;
  if (lua::Read(L, -1, &table)) {
    geometry::Cube cube;
    // Handle both cubes and arbitrary cuboids.
    if (table.LookUp("size", &cube.width)) {
      cube.depth = cube.height = cube.width;
    } else {
      table.LookUp("width", &cube.width);
      table.LookUp("height", &cube.height);
      table.LookUp("depth", &cube.depth);
    }
    if (table.LookUp("segments", &cube.num_width_segments)) {
      cube.num_height_segments = cube.num_depth_segments =
          cube.num_width_segments;
    } else {
      table.LookUp("widthSegments", &cube.num_width_segments);
      table.LookUp("heightSegments", &cube.num_height_segments);
      table.LookUp("depthSegments", &cube.num_depth_segments);
    }
    table.LookUp("shaderName", &cube.shader_name);
    Model model = {"cube", {CreateSurface(cube)}, CreateLocators(cube)};
    Push(L, model);
    return 1;
  }
  return "[model.cube] Must call with argument table.";
}

lua::NResultsOr LuaModel::CreateCylinder(lua_State* L) {
  lua::TableRef table;
  if (lua::Read(L, -1, &table)) {
    geometry::Cylinder cylinder;
    // Handle both regular and elliptic cylinders.
    if (table.LookUp("radius", &cylinder.width_radius)) {
      cylinder.depth_radius = cylinder.width_radius;
    } else {
      table.LookUp("widthRadius", &cylinder.width_radius);
      table.LookUp("depthRadius", &cylinder.depth_radius);
    }
    table.LookUp("height", &cylinder.height);
    table.LookUp("phiSegments", &cylinder.num_phi_segments);
    table.LookUp("radiusSegments", &cylinder.num_radius_segments);
    table.LookUp("heightSegments", &cylinder.num_height_segments);
    table.LookUp("shaderName", &cylinder.shader_name);
    Model model = {
        "cylinder", {CreateSurface(cylinder)}, CreateLocators(cylinder)};
    Push(L, model);
    return 1;
  }
  return "[model.cylinder] Must call with argument table.";
}

lua::NResultsOr LuaModel::CreateSphere(lua_State* L) {
  lua::TableRef table;
  if (lua::Read(L, -1, &table)) {
    geometry::Sphere sphere;
    // Handle both spheres and arbitrary ellipsoids.
    if (table.LookUp("radius", &sphere.width_radius)) {
      sphere.height_radius = sphere.depth_radius = sphere.width_radius;
    } else {
      table.LookUp("widthRadius", &sphere.width_radius);
      table.LookUp("heightRadius", &sphere.height_radius);
      table.LookUp("depthRadius", &sphere.depth_radius);
    }
    table.LookUp("phiSegments", &sphere.num_phi_segments);
    table.LookUp("thetaSegments", &sphere.num_theta_segments);
    table.LookUp("shaderName", &sphere.shader_name);
    Model model = {"sphere", {CreateSurface(sphere)}, CreateLocators(sphere)};
    Push(L, model);
    return 1;
  }
  return "[model.sphere] Must call with argument table.";
}

namespace {

// Flatten a hierarchy of models by depth-first recursion from a given node,
// transforming all traversed surfaces and moving them onto the output model.
bool RecurseHierarchy(          //
    const lua::TableRef& node,  //
    const std::string& prefix,  //
    Transform xfrm,             //
    Model* model,               //
    Eigen::AlignedBox3f* bbox,  //
    std::string* error) {
  Model node_model;
  if (!node.LookUp("model", &node_model)) {
    *error = absl::StrCat("Failed to load model at ", prefix);
    return false;
  }
  // Retrieve node transform, otherwise treat as identity.
  Transform node_xfrm;
  node.LookUpToStack("transform");
  if (node.LookUp("transform", &node_xfrm)) {
    xfrm = xfrm * node_xfrm;
  }
  // Retrieve inbound locator, otherwise treat as identity.
  std::string node_locator_name = "identity";
  if (node.LookUp("locator", &node_locator_name)) {
    auto iit = node_model.locators.find(node_locator_name);
    if (iit == node_model.locators.end()) {
      *error = absl::StrCat("Failed to find ", node_locator_name,
                            " amongst model locators at ", prefix);
      return false;
    }
    const Transform& node_locator_xfrm = iit->second;
    xfrm = xfrm * node_locator_xfrm.inverse();
  }
  // Transform the node surfaces and append them to the output model.
  Eigen::Matrix3f xfrm_i = xfrm.linear().inverse();
  for (auto& surface : node_model.surfaces) {
    TransformSurface(xfrm, xfrm_i, &surface, bbox);
    surface.name = absl::StrCat(prefix, surface.name);
    model->surfaces.emplace_back(std::move(surface));
  }
  // Recurse over children.
  lua::TableRef node_children;
  if (node.LookUp("children", &node_children)) {
    auto keys = node_children.Keys<std::string>();
    for (std::size_t i = 0; i < keys.size(); ++i) {
      const auto& locator_name = keys[i];
      // Compute child transform.
      auto iit = node_model.locators.find(locator_name);
      if (iit == node_model.locators.end()) {
        *error = absl::StrCat("Failed to find ", locator_name,
                              " amongst model locators at ", prefix);
        return false;
      }
      Transform child_xfrm = xfrm * iit->second;
      // Construct the child prefix.
      std::string child_prefix = absl::StrCat(prefix, "c", i, ":");
      // Tail recursion.
      lua::TableRef child_node;
      if (!node_children.LookUp(locator_name, &child_node)) {
        *error = absl::StrCat("Failed to read child node connected to ",
                              locator_name, " at ", prefix);
        return false;
      }
      if (!RecurseHierarchy(child_node, child_prefix, child_xfrm, model, bbox,
                            error)) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace

lua::NResultsOr LuaModel::CreateHierarchy(lua_State* L) {
  lua::TableRef table;
  if (lua::Read(L, -1, &table)) {
    Model model;
    model.name = "hierarchy";
    Transform xfrm;
    xfrm = Eigen::Matrix4f::Identity();
    Eigen::AlignedBox3f bbox;
    std::string rec_error;
    if (!RecurseHierarchy(table, "root_", xfrm, &model, &bbox, &rec_error)) {
      return absl::StrCat("[model.hierarchy] ", rec_error);
    }
    // Construct locators
    geometry::Cube cube;
    cube.width = bbox.max()[0] - bbox.min()[0];
    cube.depth = bbox.max()[1] - bbox.min()[1];
    cube.height = bbox.max()[2] - bbox.min()[2];
    model.locators = CreateLocators(cube, (bbox.max() + bbox.min()) * 0.5f);
    Push(L, model);
    return 1;
  }
  return "[model.hierarchy] Must call with argument table.";
}

lua::NResultsOr LuaModel::CreateCircularLayout(lua_State* L) {
  float radius;
  int num_samples;
  if (lua::Read(L, -2, &radius) && lua::Read(L, -1, &num_samples)) {
    if (num_samples < 1) {
      return absl::StrCat(
          "[model.circularLayout] number of samples must be greater than 0, "
          "received: ",
          lua::ToString(L, -1));
    }
    Model::LocatorMap locators;
    for (int i = 0; i < num_samples; ++i) {
      const float phi = 2.0f * geometry::kPi * i / num_samples;
      const Eigen::Vector3f y_dir(std::cos(phi), -std::sin(phi), 0.0f);
      locators[absl::StrCat("layout_", i)] = geometry::CreateZAlignedFrame(
          y_dir * radius, Eigen::Vector3f::UnitZ(), y_dir);
    }
    Model res = {"circular_layout", {}, std::move(locators)};
    Push(L, res);
    return 1;
  }
  return absl::StrCat(
      "[model.circularLayout] Must contain layout radius and number of "
      "samples, received: ",
      lua::ToString(L, -2), ", ", lua::ToString(L, -1));
}

lua::NResultsOr LuaModel::CreateLinearLayout(lua_State* L) {
  float length;
  int num_samples;
  if (lua::Read(L, -2, &length) && lua::Read(L, -1, &num_samples)) {
    if (num_samples < 1) {
      return absl::StrCat(
          "[model.linearLayout] number of samples must be greater than 0, "
          "received: ",
          lua::ToString(L, -1));
    }
    Model::LocatorMap locators;
    if (num_samples == 1) {
      Transform xfrm;
      xfrm = Eigen::Matrix4f::Identity();
      locators = {{"layout_0", xfrm}};
    } else {
      for (int i = 0; i < num_samples; ++i) {
        const float x = i / (num_samples - 1.0f) - 0.5f;
        locators[absl::StrCat("layout_", i)] =
            Eigen::Translation3f(x * length, 0.0f, 0.0f);
      }
    }
    Model res = {"linear_layout", {}, std::move(locators)};
    Push(L, res);
    return 1;
  }
  return absl::StrCat(
      "[model.linearLayout] Must contain layout length and number of samples, "
      "received: ",
      lua::ToString(L, -2), ", ", lua::ToString(L, -1));
}

lua::NResultsOr LuaModel::LoadMD3(lua_State* L) {
  std::string model_path;
  if (lua::Read(L, -1, &model_path)) {
    ModelSettersData model_data;
    DeepmindModelSetters model_setters = ModelSetters();
    if (!calls_->load_model(model_path.c_str(), &model_setters, &model_data)) {
      return "[model.loadMD3] Unable to open model file: " + model_path;
    }
    Push(L, model_data.model);
    return 1;
  }
  return absl::StrCat("[model.loadMD3] Must call with model path, received: ",
                      lua::ToString(L, -1));
}

lua::NResultsOr LuaModel::SaveMD3(lua_State* L) {
  Model model;
  std::string model_path;
  if (Read(L, -2, &model) && lua::Read(L, -1, &model_path)) {
    DeepmindModelGetters model_getters = ModelGetters();
    if (!calls_->save_model(&model_getters, &model, model_path.c_str())) {
      return absl::StrCat("[model.saveMD3] Unable to save model file: ",
                          model_path);
    }
    return 0;
  }
  return absl::StrCat(
      "[model.saveMD3] Must call with model and model_path, received: ",
      lua::ToString(L, -2), ", ", lua::ToString(L, -1));
}

}  // namespace lab
}  // namespace deepmind
