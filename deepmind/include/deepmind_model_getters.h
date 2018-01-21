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
//
// Functions to be used as callbacks within the engine for model saving.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_MODEL_GETTERS_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_MODEL_GETTERS_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepmindModelGetters_s DeepmindModelGetters;

struct DeepmindModelGetters_s {
  // Copies this model's name onto 'name', up to 'max_length' characters.
  void (*get_name)(           //
      const void* user_data,  //
      size_t max_length,      //
      char* name);

  // Returns the number of surfaces for the model.
  size_t (*get_surface_count)(  //
      const void* user_data);

  // Copies the name for the surface with index 'surf_idx' onto 'name', up to
  // 'max_length' characters.
  void (*get_surface_name)(   //
      const void* user_data,  //
      size_t surf_idx,        //
      size_t max_length,      //
      char* name);

  // Returns the number of vertices for the surface with index 'surf_idx'.
  size_t (*get_surface_vertex_count)(  //
      const void* user_data,           //
      size_t surf_idx);

  // Copies the position coordinates of the vertex with index 'vert_idx' within
  // the surface with index 'surf_idx' onto 'location'.
  void (*get_surface_vertex_location)(  //
      const void* user_data,            //
      size_t surf_idx,                  //
      size_t vert_idx,                  //
      float location[3]);

  // Copies the normal coordinates of the vertex with index 'vert_idx' within
  // the surface with index 'surf_idx' onto 'normal'.
  void (*get_surface_vertex_normal)(  //
      const void* user_data,          //
      size_t surf_idx,                //
      size_t vert_idx,                //
      float normal[3]);

  // Copies the texture coordinates of the vertex with index 'vert_idx' within
  // the surface with index 'surf_idx' onto 'st'.
  void (*get_surface_vertex_st)(  //
      const void* user_data,      //
      size_t surf_idx,            //
      size_t vert_idx,            //
      float st[2]);

  // Returns the number of faces for the surface with index 'surf_idx'.
  size_t (*get_surface_face_count)(  //
      const void* user_data,         //
      size_t surf_idx);

  // Copies the triangular face with index 'face_idx' within the surface with
  // index 'surf_idx' onto 'indices'.
  void (*get_surface_face)(   //
      const void* user_data,  //
      size_t surf_idx,        //
      size_t face_idx,        //
      int indices[3]);

  // Returns the number of shaders for the surface with index 'surf_idx'.
  size_t (*get_surface_shader_count)(  //
      const void* user_data,           //
      size_t surf_idx);

  // Copies the name of the shader with index 'shad_idx' within the surface
  // with index 'surf_idx' onto 'name', up to 'max_length' characters.
  void (*get_surface_shader)(  //
      const void* user_data,   //
      size_t surf_idx,         //
      size_t shad_idx,         //
      size_t max_length,       //
      char* name);

  // Returns the number of tags for the model.
  size_t (*get_tag_count)(  //
      const void* user_data);

  // Copies the name for the tag with index 'tag_idx' onto 'name', up to
  // 'max_length' characters.
  void (*get_tag_name)(       //
      const void* user_data,  //
      size_t tag_idx,         //
      size_t max_length,      //
      char* name);

  // Copies the coordinates of the axis with index 'axis_idx' within the
  // tag with index 'tag_idx' onto 'axis'.
  void (*get_tag_axis)(       //
      const void* user_data,  //
      size_t tag_idx,         //
      size_t axis_idx,        //
      float axis[3]);

  // Copies the coordinates of the origin of the tag with index 'tag_idx' onto
  // 'origin'.
  void (*get_tag_origin)(     //
      const void* user_data,  //
      size_t tag_idx,         //
      float origin[3]);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_MODEL_GETTERS_H_
