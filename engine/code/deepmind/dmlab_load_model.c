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

#include "dmlab_load_model.h"

#include <math.h>

#include "../qcommon/q_platform.h"
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

static const float kPi = 3.14159265358979323846f;

// Decode a normal vector from the runtime format used by ioq3 renderers.
static void LatLongToNormal(byte normal[2], vec3_t out) {
  float phi = normal[0] * 2.0f * kPi / 255.0f;
  float theta = normal[1] * 2.0f * kPi / 255.0f;
  out[0] = cosf(theta) * sinf(phi);
  out[1] = sinf(theta) * sinf(phi);
  out[2] = cosf(phi);
}

// Traverse the contents of md3Surface data structure 'md3_surf', invoking the
// relevant callbacks in `model_setters`.
static bool DeserialiseSurface(                 //
    const md3Surface_t* md3_surf,               //
    size_t surf_idx,                            //
    const DeepmindModelSetters* model_setters,  //
    void* model_data) {
  // Check MD3 surface signature.
  if (md3_surf->ident != LittleLong(MD3_IDENT)) {
    fputs("Invalid surface data.\n", stderr);
    return false;
  }

  // Set surface name.
  model_setters->set_surface_name(model_data, surf_idx, md3_surf->name);

  // Set component counts.
  size_t num_verts = LittleLong(md3_surf->numVerts);
  model_setters->set_surface_vertex_count(model_data, surf_idx, num_verts);
  size_t num_triangles = LittleLong(md3_surf->numTriangles);
  model_setters->set_surface_face_count(model_data, surf_idx, num_triangles);
  size_t num_shaders = LittleLong(md3_surf->numShaders);
  model_setters->set_surface_shader_count(model_data, surf_idx, num_shaders);

  // Set vertex data.
  size_t ofs_xyznormals = LittleLong(md3_surf->ofsXyzNormals);
  const md3XyzNormal_t* md3_verts =
      (const md3XyzNormal_t*)((const byte*)md3_surf + ofs_xyznormals);
  size_t ofs_st = LittleLong(md3_surf->ofsSt);
  const md3St_t* md3_st =
      (const md3St_t*)((const byte*)md3_surf + ofs_st);
  for (size_t i = 0; i < num_verts; ++i) {
    float location[3];
    float normal[3];
    float st[2];
    short md3_normal;

    // Set vertex location.
    for (size_t j = 0; j < 3; ++j) {
      location[j] = LittleShort(md3_verts[i].xyz[j]) * MD3_XYZ_SCALE;
    }
    model_setters->set_surface_vertex_location(model_data, surf_idx, i,
                                               location);

    // Set vertex normal.
    md3_normal = LittleShort(md3_verts[i].normal);
    LatLongToNormal((byte *)&md3_normal, normal);
    model_setters->set_surface_vertex_normal(model_data, surf_idx, i, normal);

    // Set vertex texture coordinates.
    for (size_t j = 0; j < 2; ++j) {
      st[j] = LittleFloat(md3_st[i].st[j]);
    }
    model_setters->set_surface_vertex_st(model_data, surf_idx, i, st);
  }

  // Set face indices.
  size_t ofs_triangles = LittleLong(md3_surf->ofsTriangles);
  const md3Triangle_t* md3_triangles =
      (const md3Triangle_t*)((const byte*)md3_surf + ofs_triangles);
  for (size_t i = 0; i < num_triangles; ++i) {
    int triangle[3];
    for (size_t j = 0; j < 3; ++j) {
      triangle[j] = LittleLong(md3_triangles[i].indexes[j]);
    }
    model_setters->set_surface_face(model_data, surf_idx, i, triangle);
  }

  // Set shaders.
  size_t ofs_shaders = LittleLong(md3_surf->ofsShaders);
  const md3Shader_t* md3_shaders =
      (const md3Shader_t*)((const byte*)md3_surf + ofs_shaders);
  for (size_t i = 0; i < num_shaders; ++i) {
    model_setters->set_surface_shader(model_data, surf_idx, i,
                                      md3_shaders[i].name);
  }

  return true;
}

// Traverse the contents of md3Tag data structure 'tag', invoking the relevant
// callbacks in `model_setters`.
static void DeserialiseTag(                     //
    const md3Tag_t* md3_tag,                    //
    size_t tag_idx,                             //
    const DeepmindModelSetters* model_setters,  //
    void* model_data) {
  model_setters->set_tag_name(model_data, tag_idx, md3_tag->name);
  for (size_t i = 0; i < 3; ++i) {
    float axis[3];
    for (size_t j = 0; j < 3; ++j) {
      axis[j] = LittleFloat(md3_tag->axis[i][j]);
    }
    model_setters->set_tag_axis(model_data, tag_idx, i, axis);
  }
  float origin[3];
  for (size_t j = 0; j < 3; ++j) {
    origin[j] = LittleFloat(md3_tag->origin[j]);
  }
  model_setters->set_tag_origin(model_data, tag_idx, origin);
}

bool dmlab_deserialise_model(                   //
    const void* buffer,                         //
    const DeepmindModelSetters* model_setters,  //
    void* model_data) {
  const md3Header_t* md3_model = (const md3Header_t*)buffer;

  // Check MD3 model_setters signature.
  if (md3_model->ident != LittleLong(MD3_IDENT)) {
    fputs("Invalid model_setters data.\n", stderr);
    return false;
  }

  // Check MD3 version.
  if (md3_model->version != LittleLong(15)) {
    fprintf(stderr, "Unsupported version model_setters: %d\n",
            LittleLong(md3_model->version));
    return false;
  }

  // Set model_setters name.
  model_setters->set_name(model_data, md3_model->name);

  // Set model_setters component counts.
  size_t num_surfs = LittleLong(md3_model->numSurfaces);
  model_setters->set_surface_count(model_data, num_surfs);
  size_t num_tags = LittleLong(md3_model->numTags);
  model_setters->set_tag_count(model_data, num_tags);

  // Set model_setters surfaces.
  size_t ofs_surfs = LittleLong(md3_model->ofsSurfaces);
  const md3Surface_t* md3_surf =
      (const md3Surface_t*)((const byte*)buffer + ofs_surfs);
  for (size_t i = 0; i < num_surfs; ++i) {
    if (!DeserialiseSurface(md3_surf, i, model_setters, model_data)) {
      return false;
    }
    md3_surf = (const md3Surface_t*)((const byte*)md3_surf +
                                     LittleLong(md3_surf->ofsEnd));
  }

  // Set model_setters tags.
  size_t ofs_tags = LittleLong(md3_model->ofsTags);
  const md3Tag_t* md3_tags =
      (const md3Tag_t*)((const byte*)buffer + ofs_tags);
  for (size_t i = 0; i < num_tags; ++i) {
    DeserialiseTag(md3_tags + i, i, model_setters, model_data);
  }

  return true;
}

bool dmlab_load_model(                          //
    const char* model_path,                     //
    const DeepmindModelSetters* model_setters,  //
    void* model_data) {
  if (!FS_Initialized()) {
    fputs("File system not initialized, cannot load models.\n", stderr);
    return false;
  }
  void* buffer;
  FS_ReadFile(model_path, &buffer);
  if (buffer == NULL) {
    fprintf(stderr, "Unable to open model_setters file: %s\n", model_path);
    return false;
  }
  if (!dmlab_deserialise_model(buffer, model_setters, model_data)) {
    return false;
  }
  FS_FreeFile(buffer);
  return true;
}
