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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/include/deepmind_context.h"
#include "deepmind/support/test_srcdir.h"

namespace {

using ::deepmind::lab::TestSrcDir;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;

TEST(DeepmindCallbackTest, CreateAndDestroyContext) {
  DeepmindContext ctx{};
  const char arg0[] = "dmlab";
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/callbacks_test");

  ctx.hooks.add_setting(ctx.userdata, "command", "hello");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));
  ASSERT_EQ(0, ctx.hooks.start(ctx.userdata, 0, 0));

  const char* cmd_line = ctx.hooks.replace_command_line(ctx.userdata, arg0);
  EXPECT_THAT(cmd_line, ::testing::HasSubstr("hello"));
  EXPECT_STREQ("lt_chasm_1", ctx.hooks.next_map(ctx.userdata));
  EXPECT_STREQ("lt_chasm_2", ctx.hooks.next_map(ctx.userdata));
  EXPECT_EQ(1, ctx.hooks.run_lua_snippet(
                   ctx.userdata, "return (...):commandLine('') and 1 or 0"));
  dmlab_release_context(&ctx);
}

TEST(DeepmindCallbackTest, CustomObservations) {
  DeepmindContext ctx{};
  const char callbacks_test[] = "tests/callbacks_test";
  const char order[] = "Find Apples!";
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.add_setting(ctx.userdata, "order", order);
  ctx.hooks.set_level_name(ctx.userdata, callbacks_test);
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));
  ASSERT_EQ(3, ctx.hooks.custom_observation_count(ctx.userdata));
  EnvCApi_ObservationSpec spec;

  EXPECT_STREQ("LOCATION", ctx.hooks.custom_observation_name(ctx.userdata, 0));
  ctx.hooks.custom_observation_spec(ctx.userdata, 0, &spec);
  EXPECT_EQ(1, spec.dims);
  EXPECT_EQ(EnvCApi_ObservationDoubles, spec.type);
  EXPECT_EQ(3, spec.shape[0]);

  EXPECT_STREQ("ORDER", ctx.hooks.custom_observation_name(ctx.userdata, 1));
  ctx.hooks.custom_observation_spec(ctx.userdata, 1, &spec);
  EXPECT_EQ(1, spec.dims);
  EXPECT_EQ(EnvCApi_ObservationBytes, spec.type);
  EXPECT_EQ(0, spec.shape[0]);

  EXPECT_STREQ("EPISODE", ctx.hooks.custom_observation_name(ctx.userdata, 2));
  ctx.hooks.custom_observation_spec(ctx.userdata, 2, &spec);
  EXPECT_EQ(1, spec.dims);
  EXPECT_EQ(EnvCApi_ObservationDoubles, spec.type);
  EXPECT_EQ(1, spec.shape[0]);

  const int episode = 10;
  ASSERT_EQ(0, ctx.hooks.start(ctx.userdata, episode, 0));

  EnvCApi_Observation obs;
  ctx.hooks.custom_observation(ctx.userdata, 0, &obs);
  ASSERT_EQ(1, obs.spec.dims);
  ASSERT_EQ(EnvCApi_ObservationDoubles, obs.spec.type);
  EXPECT_THAT(std::make_tuple(obs.payload.doubles, obs.spec.shape[0]),
              ElementsAre(10.0, 20.0, 30.0));

  ctx.hooks.custom_observation(ctx.userdata, 1, &obs);
  ASSERT_EQ(1, obs.spec.dims);
  ASSERT_EQ(EnvCApi_ObservationBytes, obs.spec.type);
  EXPECT_THAT(std::make_tuple(obs.payload.bytes, obs.spec.shape[0]),
              ElementsAreArray(order, sizeof(order) - 1));

  ctx.hooks.custom_observation(ctx.userdata, 2, &obs);
  ASSERT_EQ(1, obs.spec.dims);
  ASSERT_EQ(EnvCApi_ObservationDoubles, obs.spec.type);
  EXPECT_THAT(std::make_tuple(obs.payload.doubles, obs.spec.shape[0]),
              ElementsAre(episode));

  dmlab_release_context(&ctx);
}

TEST(DeepmindCallbackTest, CreateModel) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/callbacks_test");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));

  ASSERT_TRUE(ctx.hooks.find_model(ctx.userdata, "cube"));
  DeepmindModelGetters model;
  void* model_data;
  ctx.hooks.model_getters(ctx.userdata, &model, &model_data);

  ASSERT_EQ(model.get_surface_count(model_data), 1);

  ASSERT_EQ(model.get_surface_vertex_count(model_data, 0), 24);
  ASSERT_EQ(model.get_surface_face_count(model_data, 0), 12);

  float vertex_normal[3];
  model.get_surface_vertex_normal(model_data, 0, 12, vertex_normal);
  EXPECT_THAT(vertex_normal, ElementsAre(-1.0, 0.0, 0.0));
  model.get_surface_vertex_normal(model_data, 0, 23, vertex_normal);
  EXPECT_THAT(vertex_normal, ElementsAre(0.0, -1.0, 0.0));

  int face_indices[3];
  model.get_surface_face(model_data, 0, 11, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(20, 22, 23));

  ctx.hooks.clear_model(ctx.userdata);
  dmlab_release_context(&ctx);
}

}  // namespace
