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

#include "dmlab_save_model.h"

#include <math.h>

#include "../qcommon/q_platform.h"
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

// The AABB functions below have been duplicated from q3map2/libs/mathlib.
typedef struct aabb_s {
  vec3_t origin;
  vec3_t extents;
  vec_t radius;
} aabb_t;

static void AABBUpdateRadius(aabb_t *aabb) {
  aabb->radius = VectorLength(aabb->extents);
}

static void AABBClear(aabb_t *aabb) {
  aabb->origin[0] = aabb->origin[1] = aabb->origin[2] = 0;
  aabb->extents[0] = aabb->extents[1] = aabb->extents[2] = -1;
}

static void AABBExtendByPoint(aabb_t *aabb, const vec3_t point) {
  vec_t min, max, displacement;
  for (int i = 0; i < 3; ++i) {
    displacement = point[i] - aabb->origin[i];
    if (fabs(displacement) > aabb->extents[i]) {
      if (aabb->extents[i] < 0) { // degenerate
        min = max = point[i];
      } else if (displacement > 0) {
        min = aabb->origin[i] - aabb->extents[i];
        max = aabb->origin[i] + displacement;
      } else {
        max = aabb->origin[i] + aabb->extents[i];
        min = aabb->origin[i] + displacement;
      }
      aabb->origin[i] = (min + max) * 0.5f;
      aabb->extents[i] = max - aabb->origin[i];
    }
  }
}

// Encode a normal vector in the runtime format used by ioq3 renderers,
// expressing them in spherical coordinates (longitude, latitude) and storing
// such coordinates in unsigned byte range.
static void NormalToLatLong(const vec3_t normal, byte out[2]) {
  // check for singularities
  if (normal[0] == 0 && normal[1] == 0) {
    if (normal[2] > 0) {
      out[0] = 0;
      out[1] = 0;       // lat = 0, long = 0
    } else {
      out[0] = 128;
      out[1] = 0;       // lat = 0, long = 128
    }
  } else {
    int a = RAD2DEG(atan2(normal[1], normal[0])) * (255.0f / 360.0f);
    int b = RAD2DEG(acos(normal[2])) * (255.0f / 360.0f);
    out[0] = b & 0xff;   // longitude
    out[1] = a & 0xff;   // latitude
  }
}

// Compute the size required to store the surface with index 'surf_idx', using
// the surface data returned by the callbacks in 'model'.
static size_t SurfaceSize(              //
    size_t surf_idx,                    //
    const DeepmindModelGetters* model,  //
    void* model_data) {
  size_t res;
  res = sizeof(md3Surface_t);
  res += model->get_surface_shader_count(model_data, surf_idx) *
      sizeof(md3Shader_t);
  res += model->get_surface_face_count(model_data, surf_idx) *
      sizeof(md3Triangle_t);
  res += model->get_surface_vertex_count(model_data, surf_idx) *
      (sizeof(md3St_t) + sizeof(md3XyzNormal_t));
  return res;
}

// Returns the size required to store the serialised form of 'model'.
size_t dmlab_serialised_model_size(     //
    const DeepmindModelGetters* model,  //
    void* model_data) {
  size_t res = sizeof(md3Header_t);
  res += sizeof(md3Frame_t);
  res += model->get_tag_count(model_data) * sizeof(md3Tag_t);
  size_t num_surfs = model->get_surface_count(model_data);
  for (size_t i = 0; i < num_surfs; ++i) {
    res += SurfaceSize(i, model, model_data);
  }
  return res;
}

// Fill buffer 'surf' with the surface data for index 'surf_idx' returned by the
// callbacks in 'model'.
static void SerialiseSurface(           //
    size_t surf_idx,                    //
    const DeepmindModelGetters* model,  //
    void* model_data,                   //
    md3Surface_t* md3_surf,             //
    aabb_t* bbox) {
  // Fill surface header.
  md3_surf->ident = LittleLong(MD3_IDENT);
  model->get_surface_name(model_data, surf_idx, MAX_QPATH, md3_surf->name);
  md3_surf->flags = 0;
  md3_surf->numFrames = LittleLong(1);
  size_t num_shaders = model->get_surface_shader_count(model_data, surf_idx);
  md3_surf->numShaders = LittleLong(num_shaders);
  size_t num_verts = model->get_surface_vertex_count(model_data, surf_idx);
  md3_surf->numVerts = LittleLong(num_verts);
  size_t num_triangles = model->get_surface_face_count(model_data, surf_idx);
  md3_surf->numTriangles = LittleLong(num_triangles);
  size_t ofs_shaders = sizeof(md3Surface_t);
  md3_surf->ofsShaders = LittleLong(ofs_shaders);
  size_t ofs_triangles = ofs_shaders + num_shaders * sizeof(md3Shader_t);
  md3_surf->ofsTriangles = LittleLong(ofs_triangles);
  size_t ofs_st = ofs_triangles + num_triangles * sizeof(md3Triangle_t);
  md3_surf->ofsSt = LittleLong(ofs_st);
  size_t ofs_xyznormals = ofs_st + num_verts * sizeof(md3St_t);
  md3_surf->ofsXyzNormals = LittleLong(ofs_xyznormals);
  size_t ofs_end = ofs_xyznormals + num_verts * sizeof(md3XyzNormal_t);
  md3_surf->ofsEnd = LittleLong(ofs_end);

  // Fill shader data.
  md3Shader_t* md3_shaders = (md3Shader_t*)((byte*)md3_surf + ofs_shaders);
  for (size_t i = 0; i < num_shaders; ++i) {
    model->get_surface_shader(model_data, surf_idx, i, MAX_QPATH,
                              md3_shaders[i].name);
    md3_shaders[i].shaderIndex = 0;
  }

  // Fill triangle data.
  md3Triangle_t* md3_triangles =
      (md3Triangle_t*)((byte*)md3_surf + ofs_triangles);
  for (size_t i = 0; i < num_triangles; ++i) {
    int triangle[3];
    model->get_surface_face(model_data, surf_idx, i, triangle);
    for (size_t j = 0; j < 3; ++j) {
      md3_triangles[i].indexes[j] = LittleLong(triangle[j]);
    }
  }

  // Fill texture coordinate data.
  md3St_t* md3_st = (md3St_t*)((byte*)md3_surf + ofs_st);
  for (size_t i = 0; i < num_verts; ++i) {
    float st[2];
    model->get_surface_vertex_st(model_data, surf_idx, i, st);
    for (size_t j = 0; j < 2; ++j) {
      md3_st[i].st[j] = LittleFloat(st[j]);
    }
  }

  // Fill vertex data.
  md3XyzNormal_t* md3_verts =
      (md3XyzNormal_t*)((byte*)md3_surf + ofs_xyznormals);
  for (size_t i = 0; i < num_verts; ++i) {
    float location[3];
    float normal[3];
    short md3_normal;

    // Fill vertex location.
    model->get_surface_vertex_location(model_data, surf_idx, i, location);
    for (size_t j = 0; j < 3; ++j) {
      md3_verts[i].xyz[j] = LittleShort(location[j] / MD3_XYZ_SCALE);
    }

    // Fill vertex normal.
    model->get_surface_vertex_normal(model_data, surf_idx, i, normal);
    NormalToLatLong(normal, (byte *)&md3_normal);
    md3_verts[i].normal = LittleShort(md3_normal);

    // Update bounding box.
    AABBExtendByPoint(bbox, location);
  }
}

// Fill buffer 'tag' with the tag data for index 'tag_idx' returned by the
// callbacks in 'model'.
static void SerialiseTag(               //
    size_t tag_idx,                     //
    const DeepmindModelGetters* model,  //
    void* model_data,                   //
    md3Tag_t* tag) {
  model->get_tag_name(model_data, tag_idx, MAX_QPATH, tag->name);
  for (size_t i = 0; i < 3; ++i) {
    float axis[3];
    model->get_tag_axis(model_data, tag_idx, i, axis);
    for (size_t j = 0; j < 3; ++j) {
      tag->axis[i][j] = LittleFloat(axis[j]);
    }
  }
  float origin[3];
  model->get_tag_origin(model_data, tag_idx, origin);
  for (size_t j = 0; j < 3; ++j) {
    tag->origin[j] = LittleFloat(origin[j]);
  }
}

void dmlab_serialise_model(             //
    const DeepmindModelGetters* model,  //
    void* model_data,                   //
    void* buffer) {
  // Fill model data.
  md3Header_t* md3_model = (md3Header_t*)buffer;
  md3_model->ident = LittleLong(MD3_IDENT);
  md3_model->version =LittleLong(MD3_VERSION);
  model->get_name(model_data, MAX_QPATH, md3_model->name);
  md3_model->flags = 0;
  md3_model->numFrames = LittleLong(1);
  size_t num_tags = model->get_tag_count(model_data);
  md3_model->numTags = LittleLong(num_tags);
  size_t num_surfs = model->get_surface_count(model_data);
  md3_model->numSurfaces = LittleLong(num_surfs);
  md3_model->numSkins = 0;
  size_t ofs_frames = sizeof(md3Header_t);
  md3_model->ofsFrames = LittleLong(ofs_frames);
  size_t ofs_tags = ofs_frames + sizeof(md3Frame_t);
  md3_model->ofsTags = LittleLong(ofs_tags);
  size_t ofs_surfs = ofs_tags + num_tags * sizeof(md3Tag_t);
  md3_model->ofsSurfaces = LittleLong(ofs_surfs);

  // Fill tags.
  md3Tag_t* md3_tags = (md3Tag_t*)(buffer + ofs_tags);
  for (size_t i = 0; i < num_tags; ++i) {
    SerialiseTag(i, model, model_data, md3_tags + i);
  }

  // Fill surfaces.
  aabb_t bbox;
  AABBClear(&bbox);
  md3Surface_t* md3_surf = (md3Surface_t*)(buffer + ofs_surfs);
  for (size_t i = 0; i < num_surfs; ++i) {
    SerialiseSurface(i, model, model_data, md3_surf, &bbox);
    md3_surf = (md3Surface_t*)((byte*)md3_surf + LittleLong(md3_surf->ofsEnd));
  }

  // Fill frame data.
  AABBUpdateRadius(&bbox);
  md3Frame_t* md3_frame = (md3Frame_t*)(buffer + ofs_frames);
  for (size_t j = 0; j < 3; ++j) {
    md3_frame->bounds[0][j] = LittleFloat(bbox.origin[j] - bbox.extents[j]);
    md3_frame->bounds[1][j] = LittleFloat(bbox.origin[j] + bbox.extents[j]);
    md3_frame->localOrigin[j] = LittleFloat(bbox.origin[j]);
  }
  md3_frame->radius = LittleFloat(bbox.radius);
  Q_strncpyz(md3_frame->name, md3_model->name, sizeof(md3_frame->name));

  // Complete model data.
  size_t ofs_end = (byte*)md3_surf - (byte*)buffer;
  md3_model->ofsEnd = LittleLong(ofs_end);
}

bool dmlab_save_model(                  //
    const DeepmindModelGetters* model,  //
    void* model_data,                   //
    const char* model_path) {
  if (!FS_Initialized()) {
    fputs("File system not initialized, cannot save models.\n", stderr);
    return false;
  }
  size_t len = dmlab_serialised_model_size(model, model_data);
  byte* buffer = Hunk_AllocateTempMemory(len);
  if (buffer == NULL) {
    fprintf(stderr,
            "Unable to allocate intermediate storage to serialize model: %s\n",
            model_path);
    return false;
  }
  dmlab_serialise_model(model, model_data, buffer);
  FS_WriteFile(model_path, buffer, len);
  Hunk_FreeTempMemory(buffer);
  return true;
}
