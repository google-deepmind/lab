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
// Functions to be used as callbacks within the engine for model loading.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_MODEL_SETTERS_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_MODEL_SETTERS_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepmindModelSetters_s DeepmindModelSetters;

struct DeepmindModelSetters_s {
  // Called with the name for this model.
  void (*set_name)(     //
      void* user_data,  //
      const char* name);

  // Called with the number of surfaces for the model, prior to any call related
  // to any of the them.
  void (*set_surface_count)(  //
      void* user_data,        //
      size_t num_surfaces);

  // Called with the name for the surface with index 'surf_idx'.
  void (*set_surface_name)(  //
      void* user_data,       //
      size_t surf_idx,       //
      const char* name);

  // Called with the number of vertices for the surface with index 'surf_idx',
  // prior to any call related to vertex or face data.
  void (*set_surface_vertex_count)(  //
      void* user_data,               //
      size_t surf_idx,               //
      size_t num_verts);

  // Called with the location coordinates of the vertex with index 'vert_idx'
  // within the surface with index 'surf_idx'.
  void (*set_surface_vertex_location)(  //
      void* user_data,                  //
      size_t surf_idx,                  //
      size_t vert_idx,                  //
      float location[3]);

  // Called with the normal coordinates of the vertex with index 'vert_idx'
  // within the surface with index 'surf_idx'.
  void (*set_surface_vertex_normal)(  //
      void* user_data,                //
      size_t surf_idx,                //
      size_t vert_idx,                //
      float normal[3]);

  // Called with the texture coordinates of the vertex with index 'vert_idx'
  // within the surface with index 'surf_idx'.
  void (*set_surface_vertex_st)(  //
      void* user_data,            //
      size_t surf_idx,            //
      size_t vert_idx,            //
      float st[2]);

  // Called with the number of faces for the surface with index 'surf_idx',
  // prior to any call related to face data.
  void (*set_surface_face_count)(  //
      void* user_data,             //
      size_t surf_idx,             //
      size_t num_faces);

  // Called with the indices of the triangular face with index 'face_idx' within
  // the surface with index 'surf_idx'.
  void (*set_surface_face)(  //
      void* user_data,       //
      size_t surf_idx,       //
      size_t face_idx,       //
      int indices[3]);

  // Called with the number of shaders for the surface with index 'surf_idx',
  // prior to any call related to shader data.
  void (*set_surface_shader_count)(  //
      void* user_data,               //
      size_t surf_idx,               //
      size_t num_shaders);

  // Called with the name of the shader with index 'shad_idx' within the surface
  // with index 'surf_idx'.
  void (*set_surface_shader)(  //
      void* user_data,         //
      size_t surf_idx,         //
      size_t shad_idx,         //
      const char* name);

  // Called with the number of tags for the model, prior to any call related to
  // any of the them.
  void (*set_tag_count)(  //
      void* user_data,    //
      size_t num_tags);

  // Called with the name for the tag with index 'tag_idx', prior to any cal
  // related to the tag data.
  void (*set_tag_name)(  //
      void* user_data,   //
      size_t tag_idx,    //
      const char* name);

  // Called with the coordinates of the axis with index 'axis_idx' within the
  // tag with index 'tag_idx'.
  void (*set_tag_axis)(  //
      void* user_data,   //
      size_t tag_idx,    //
      size_t axis_idx,   //
      float axis[3]);

  // Called with the coordinates of the origin of the tag with index 'tag_idx'.
  void (*set_tag_origin)(  //
      void* user_data,     //
      size_t tag_idx,      //
      float origin[3]);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_MODEL_SETTERS_H_
