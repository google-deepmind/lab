// Copyright (C) 2016-2017 Google Inc.
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

#include "deepmind/engine/context.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

#include "deepmind/support/logging.h"
#include "absl/strings/str_cat.h"
#include "deepmind/engine/lua_image.h"
#include "deepmind/engine/lua_maze_generation.h"
#include "deepmind/engine/lua_random.h"
#include "deepmind/engine/lua_text_level_maker.h"
#include "deepmind/include/deepmind_model_getters.h"
#include "deepmind/level_generation/compile_map.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/model_generation/lua_model.h"
#include "deepmind/model_generation/lua_transform.h"
#include "deepmind/model_generation/model_getters.h"
#include "deepmind/model_generation/model_lua.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"
#include "public/level_cache_types.h"
#include "third_party/rl_api/env_c_api.h"

using deepmind::lab::Context;

extern "C" {
static void add_setting(void* userdata, const char* key, const char* value) {
  return static_cast<Context*>(userdata)->AddSetting(key, value);
}

static void set_level_cache_settings(
    void* userdata, bool local, bool global,
    DeepMindLabLevelCacheParams level_cache_params) {
  return static_cast<Context*>(userdata)->SetLevelCacheSetting(
      local, global, level_cache_params);
}

static int set_script_name(void* userdata, const char* script_name) {
  return static_cast<Context*>(userdata)->SetScriptName(script_name);
}

static int init(void* userdata) {
  return static_cast<Context*>(userdata)->Init();
}

static int start(void* userdata, int episode, int seed) {
  return static_cast<Context*>(userdata)->Start(episode, seed);
}

static const char* error_message(void* userdata) {
  return static_cast<Context*>(userdata)->ErrorMessage();
}

static void set_error_message(void* userdata, const char* error_message) {
  static_cast<Context*>(userdata)->SetErrorMessage(error_message);
}

static int map_loaded(void* userdata) {
  return static_cast<Context*>(userdata)->MapLoaded();
}

static const char* replace_command_line(void* userdata,
                                        const char* old_commandline) {
  return static_cast<Context*>(userdata)->GetCommandLine(old_commandline);
}

static const char* get_temporary_folder(void* userdata) {
  return static_cast<Context*>(userdata)->TempDirectory().c_str();
}

static const char* next_map(void* userdata) {
  return static_cast<Context*>(userdata)->NextMap();
}

void update_inventory(void* userdata, bool is_spawning, int player_id,
                      int gadget_count, int gadget_inventory[], int stat_count,
                      int stat_inventory[], int powerup_count,
                      int powerup_time[], int gadget_held, float height,
                      float position[3], float view_angles[3]) {
  static_cast<Context*>(userdata)->UpdateInventory(
      is_spawning, player_id, gadget_count, gadget_inventory, stat_count,
      stat_inventory, powerup_count, powerup_time, gadget_held, height,
      position, view_angles);
}

static int game_type(void* userdata) {
  return static_cast<Context*>(userdata)->GameType();
}

static char team_select(void* userdata, int player_id,
                        const char* player_name) {
  return static_cast<Context*>(userdata)->TeamSelect(player_id, player_name);
}

static bool has_alt_cameras(void* userdata) {
  return static_cast<Context*>(userdata)->HasAltCameras();
}

static void set_has_alt_cameras(void* userdata, bool has_alt_camera) {
  static_cast<Context*>(userdata)->SetHasAltCameras(has_alt_camera);
}

static void custom_view(void* userdata, int* width, int* height,
                        float position[3], float view_angles[3],
                        bool* render_player) {
  static_cast<Context*>(userdata)->Game().GetCustomView(
      width, height, position, view_angles, render_player);
}

static int run_lua_snippet(void* userdata, const char* command) {
  std::size_t command_len = std::strlen(command);
  return static_cast<Context*>(userdata)->RunLuaSnippet(command, command_len);
}

static void set_native_app(void* userdata, bool v) {
  return static_cast<Context*>(userdata)->SetNativeApp(v);
}

static bool get_native_app(void* userdata) {
  return static_cast<Context*>(userdata)->NativeApp();
}

static void set_actions(void* userdata, double look_down_up,
                        double look_left_right, signed char move_back_forward,
                        signed char strafe_left_right, signed char crouch_jump,
                        int buttons_down) {
  return static_cast<Context*>(userdata)->SetActions(
      look_down_up, look_left_right, move_back_forward, strafe_left_right,
      crouch_jump, buttons_down);
}

static void get_actions(void* userdata, double* look_down_up,
                        double* look_left_right, signed char* move_back_forward,
                        signed char* strafe_left_right,
                        signed char* crouch_jump, int* buttons_down) {
  return static_cast<Context*>(userdata)->GetActions(
      look_down_up, look_left_right, move_back_forward, strafe_left_right,
      crouch_jump, buttons_down);
}

static int update_spawn_vars(void* userdata, char* spawn_var_chars,
                             int* num_spawn_var_chars,
                             int spawn_var_offsets[][2], int* num_spawn_vars) {
  return static_cast<Context*>(userdata)->MutablePickups()->UpdateSpawnVars(
      spawn_var_chars, num_spawn_var_chars, spawn_var_offsets, num_spawn_vars);
}

static int make_extra_entities(void* userdata) {
  return static_cast<Context*>(userdata)->MutablePickups()->MakeExtraEntities();
}

static void read_extra_entity(void* userdata, int entity_index,
                              char* spawn_var_chars, int* num_spawn_var_chars,
                              int spawn_var_offsets[][2], int* num_spawn_vars) {
  return static_cast<Context*>(userdata)->MutablePickups()->ReadExtraEntity(
      entity_index, spawn_var_chars, num_spawn_var_chars, spawn_var_offsets,
      num_spawn_vars);
}

static int dynamic_spawn_entity_count(void* userdata) {
  return static_cast<Context*>(userdata)->Pickups().DynamicSpawnEntityCount();
}

static void read_dynamic_spawn_entity(void* userdata, int entity_index,
                                      char* spawn_var_chars,
                                      int* num_spawn_var_chars,
                                      int spawn_var_offsets[][2],
                                      int* num_spawn_vars) {
  static_cast<Context*>(userdata)->Pickups().ReadDynamicSpawnEntity(
      entity_index, spawn_var_chars, num_spawn_var_chars, spawn_var_offsets,
      num_spawn_vars);
}

static void clear_dynamic_spawn_entities(void* userdata) {
  static_cast<Context*>(userdata)
      ->MutablePickups()
      ->ClearDynamicSpawnEntities();
}

static int register_dynamic_items(void* userdata) {
  return static_cast<Context*>(userdata)
      ->MutablePickups()
      ->RegisterDynamicItems();
}
static void read_dynamic_item_name(void* userdata, int item_index,
                                   char* item_name) {
  static_cast<Context*>(userdata)->Pickups().ReadDynamicItemName(item_index,
                                                                 item_name);
}

static bool find_item(void* userdata, const char* class_name, int* index) {
  return static_cast<Context*>(userdata)->MutablePickups()->FindItem(class_name,
                                                                     index);
}

static int item_count(void* userdata) {
  return static_cast<const Context*>(userdata)->Pickups().ItemCount();
}

static bool item(void* userdata, int index,
                 char* item_name, int max_item_name,
                 char* class_name, int max_class_name,
                 char* model_name, int max_model_name,
                 int* quantity, int* type, int* tag) {
  return static_cast<const Context*>(userdata)->Pickups().GetItem(
      index, item_name, max_item_name, class_name, max_class_name, model_name,
      max_model_name, quantity, type, tag);
}

static void clear_items(void* userdata) {
  static_cast<Context*>(userdata)->MutablePickups()->ClearItems();
}

static bool find_model(void* userdata, const char* model_name) {
  return static_cast<Context*>(userdata)->FindModel(model_name);
}

static void model_getters(void* userdata, DeepmindModelGetters* model_getters,
                          void** model_data) {
  static_cast<Context*>(userdata)->GetModelGetters(model_getters, model_data);
}

static void clear_model(void* userdata) {
  static_cast<Context*>(userdata)->ClearModel();
}

static bool map_finished(void* userdata) {
  return static_cast<Context*>(userdata)->Game().MapFinished();
}

static void set_map_finished(void* userdata, bool map_finished) {
  static_cast<Context*>(userdata)->MutableGame()->SetMapFinished(map_finished);
}

static bool can_pickup(void* userdata, int entity_id) {
  return static_cast<Context*>(userdata)->MutablePickups()->CanPickup(
      entity_id);
}

static bool override_pickup(void* userdata, int entity_id, int* respawn) {
  return static_cast<Context*>(userdata)->MutablePickups()->OverridePickup(
      entity_id, respawn);
}

static bool can_trigger(void* userdata, int entity_id,
                        const char* target_name) {
  return static_cast<Context*>(userdata)->CanTrigger(entity_id, target_name);
}

static bool override_trigger(void* userdata, int entity_id,
                             const char* target_name) {
  return static_cast<Context*>(userdata)->OverrideTrigger(entity_id,
                                                          target_name);
}

static void trigger_lookat(void* userdata, int entity_id, bool looked_at,
                           const float position[3]) {
  static_cast<Context*>(userdata)->TriggerLookat(entity_id, looked_at,
                                                 position);
}

static int reward_override(void* userdata, const char* reason_opt,
                           int player_id, int team,
                           const int* other_player_id_opt,
                           const float* origin_opt, int score) {
  return static_cast<Context*>(userdata)->RewardOverride(
      reason_opt, player_id, team, other_player_id_opt, origin_opt, score);
}

static void add_score(void* userdata, int player_id, double reward) {
  static_cast<Context*>(userdata)->AddScore(player_id, reward);
}

static int make_random_seed(void* userdata) {
  return static_cast<Context*>(userdata)->MakeRandomSeed();
}

static double has_episode_finished(void* userdata,
                                   double elapsed_episode_time_seconds) {
  return static_cast<Context*>(userdata)->HasEpisodeFinished(
      elapsed_episode_time_seconds);
}

static void add_bots(void* userdata) {
  return static_cast<Context*>(userdata)->AddBots();
}

static bool modify_rgba_texture(void* userdata, const char* name,
                                unsigned char* data, int width, int height) {
  return static_cast<Context*>(userdata)->ModifyRgbaTexture(name, data, width,
                                                            height);
}

static bool replace_model_name(void* userdata, const char* name, char* new_name,
                               int new_name_size, char* texture_prefix,
                               int texture_prefix_size) {
  return static_cast<Context*>(userdata)->ReplaceModelName(
      name, new_name, new_name_size, texture_prefix, texture_prefix_size);
}

static bool replace_texture_name(void* userdata, const char* name,
                                 char* new_name, int max_size) {
  return static_cast<Context*>(userdata)->ReplaceTextureName(name, new_name,
                                                             max_size);
}

static bool load_texture(void* userdata, const char* name,
                         unsigned char** pixels, int* width, int* height,
                         void* (*allocator)(int size)) {
  return static_cast<Context*>(userdata)->LoadTexture(name, pixels, width,
                                                      height, allocator);
}

static int custom_observation_count(void* userdata) {
  return static_cast<const Context*>(userdata)->Observations().Count();
}

static const char* custom_observation_name(void* userdata, int idx) {
  return static_cast<const Context*>(userdata)->Observations().Name(idx);
}

static void custom_observation_spec(void* userdata, int idx,
                                    EnvCApi_ObservationSpec* spec) {
  static_cast<const Context*>(userdata)->Observations().Spec(idx, spec);
}

static void custom_observation(void* userdata, int idx,
                               EnvCApi_Observation* observation) {
  return static_cast<Context*>(userdata)->MutableObservations()->Observation(
      idx, observation);
}

static void player_state(void* userdata, const float origin[3],
                         const float velocity[3], const float viewangles[3],
                         float height, int team_score, int other_team_score,
                         int player_id, bool teleported, int timestamp_msec) {
  return static_cast<Context*>(userdata)->MutableGame()->SetPlayerState(
      origin, velocity, viewangles, height, team_score, other_team_score,
      player_id, teleported, timestamp_msec);
}

static int make_screen_messages(void* userdata, int screen_width,
                                int screen_height, int line_height,
                                int string_buffer_size) {
  return static_cast<Context*>(userdata)->MakeScreenMessages(
      screen_width, screen_height, line_height, string_buffer_size);
}

static void get_screen_message(void* userdata, int message_id, char* buffer,
                               int* x, int* y, int* align_l0_r1_c2,
                               int* shadow, float rgba[4]) {
  static_cast<Context*>(userdata)->GetScreenMessage(
      message_id, buffer, x, y, align_l0_r1_c2, shadow, rgba);
}

static int make_filled_rectangles(void* userdata, int screen_width,
                                  int screen_height) {
  return static_cast<Context*>(userdata)->MakeFilledRectangles(screen_width,
                                                               screen_height);
}

static void get_filled_rectangle(void* userdata, int rectangle_id, int* x,
                                 int* y, int* width, int* height,
                                 float rgba[4]) {
  static_cast<Context*>(userdata)->GetFilledRectangle(rectangle_id, x, y, width,
                                                      height, rgba);
}

static void lua_mover(void* userdata, int entity_id,
                      const float entity_position[3],
                      const float player_position[3],
                      const float player_velocity[3],
                      float player_position_delta[3],
                      float player_velocity_delta[3]) {
  static_cast<Context*>(userdata)->CustomPlayerMovement(
      entity_id, entity_position, player_position, player_velocity,
      player_position_delta, player_velocity_delta);
}

static void game_event(void* userdata, const char* event_name, int count,
                         const float* data) {
  static_cast<Context*>(userdata)->GameEvent(event_name, count, data);
}

static void make_pk3_from_map(void* userdata, const char* map_path,
                              const char* map_name, bool gen_aas) {
  static_cast<Context*>(userdata)->MakePk3FromMap(map_path, map_name, gen_aas);
}

static void events_clear(void* userdata) {
  static_cast<Context*>(userdata)->MutableEvents()->Clear();
}

static int events_type_count(void* userdata) {
  return static_cast<const Context*>(userdata)->Events().TypeCount();
}

static const char* events_type_name(void* userdata, int event_type_idx) {
  return static_cast<const Context*>(userdata)->Events().TypeName(
      event_type_idx);
}

static int events_count(void* userdata) {
  return static_cast<const Context*>(userdata)->Events().Count();
}

static void events_export(void* userdata, int event_idx, EnvCApi_Event* event) {
  static_cast<Context*>(userdata)->MutableEvents()->Export(event_idx, event);
}

static void entities_clear(void* userdata) {
  static_cast<Context*>(userdata)->MutableGameEntities()->Clear();
}

static void entities_add(void* userdata, int entity_id, int user_id,
                            int type, int flags, float position[3],
                            const char* classname) {
  static_cast<Context*>(userdata)->MutableGameEntities()->Add(
      entity_id, user_id, type, flags, position, classname);
}

}  // extern "C"

namespace deepmind {
namespace lab {
namespace {

constexpr char kGameScriptPath[] = "/baselab/game_scripts";
constexpr char kLevelsDirectory[] = "/levels";
constexpr double kDefaultEpisodeLengthSeconds = 5 * 30.0;

constexpr const char* const kTeamNames[] = {
  "free",
  "red",
  "blue",
  "spectator"
};

lua::NResultsOr MapMakerModule(lua_State* L) {
  if (auto* ctx =
          static_cast<Context*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaTextLevelMaker::CreateObject(
        L, ctx->ExecutableRunfiles(), ctx->TempDirectory(),
        ctx->UseLocalLevelCache(), ctx->UseGlobalLevelCache(),
        ctx->LevelCacheParams());
    return 1;
  } else {
    return "Missing context!";
  }
}

lua::NResultsOr ModelModule(lua_State* L) {
  if (const auto* calls = static_cast<const DeepmindCalls*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaModel::CreateObject(L, calls);
    return 1;
  } else {
    return "Missing context!";
  }
}

}  // namespace

Context::Context(lua::Vm lua_vm, const char* executable_runfiles,
                 const DeepmindCalls* calls, DeepmindHooks* hooks,
                 bool (*file_reader_override)(const char* file_name,
                                              char** buff, size_t* size),
                 const char* temp_folder)
    : lua_vm_(std::move(lua_vm)),
      native_app_(false),
      actions_{},
      level_cache_params_{},
      game_(executable_runfiles, calls, file_reader_override,
            temp_folder != nullptr ? temp_folder : ""),
      has_alt_cameras_(false) {
  CHECK(lua_vm_ != nullptr);
  hooks->add_setting = add_setting;
  hooks->set_level_cache_settings = set_level_cache_settings;
  hooks->set_script_name = set_script_name;
  hooks->start = start;
  hooks->map_loaded = map_loaded;
  hooks->init = init;
  hooks->error_message = error_message;
  hooks->set_error_message = set_error_message;
  hooks->replace_command_line = replace_command_line;
  hooks->next_map = next_map;
  hooks->game_type = game_type;
  hooks->team_select = team_select;
  hooks->run_lua_snippet = run_lua_snippet;
  hooks->set_native_app = set_native_app;
  hooks->get_native_app = get_native_app;
  hooks->set_actions = set_actions;
  hooks->get_actions = get_actions;
  hooks->find_model = find_model;
  hooks->model_getters = model_getters;
  hooks->clear_model = clear_model;
  hooks->map_finished = map_finished;
  hooks->set_map_finished = set_map_finished;
  hooks->can_pickup = can_pickup;
  hooks->override_pickup = override_pickup;
  hooks->can_trigger = can_trigger;
  hooks->override_trigger = override_trigger;
  hooks->trigger_lookat = trigger_lookat;
  hooks->reward_override = reward_override;
  hooks->add_score = add_score;
  hooks->make_random_seed = make_random_seed;
  hooks->has_episode_finished = has_episode_finished;
  hooks->add_bots = add_bots;
  hooks->replace_model_name = replace_model_name;
  hooks->replace_texture_name = replace_texture_name;
  hooks->load_texture = load_texture;
  hooks->modify_rgba_texture = modify_rgba_texture;
  hooks->custom_observation_count = custom_observation_count;
  hooks->custom_observation_name = custom_observation_name;
  hooks->custom_observation_spec = custom_observation_spec;
  hooks->custom_observation = custom_observation;
  hooks->player_state = player_state;
  hooks->make_screen_messages = make_screen_messages;
  hooks->get_screen_message = get_screen_message;
  hooks->make_filled_rectangles = make_filled_rectangles;
  hooks->get_filled_rectangle = get_filled_rectangle;
  hooks->get_temporary_folder = get_temporary_folder;
  hooks->make_pk3_from_map = make_pk3_from_map;
  hooks->lua_mover = lua_mover;
  hooks->game_event = game_event;
  hooks->pickups.update_spawn_vars = update_spawn_vars;
  hooks->pickups.make_extra_entities = make_extra_entities;
  hooks->pickups.read_extra_entity = read_extra_entity;
  hooks->pickups.find_item = find_item;
  hooks->pickups.item_count = item_count;
  hooks->pickups.item = item;
  hooks->pickups.clear_items = clear_items;
  hooks->pickups.dynamic_spawn_entity_count = dynamic_spawn_entity_count;
  hooks->pickups.read_dynamic_spawn_entity = read_dynamic_spawn_entity;
  hooks->pickups.clear_dynamic_spawn_entities = clear_dynamic_spawn_entities;
  hooks->pickups.register_dynamic_items = register_dynamic_items;
  hooks->pickups.read_dynamic_item_name = read_dynamic_item_name;
  hooks->events.clear = events_clear;
  hooks->events.type_count = events_type_count;
  hooks->events.type_name = events_type_name;
  hooks->events.count = events_count;
  hooks->events.export_event = events_export;
  hooks->entities.clear = entities_clear;
  hooks->entities.add = entities_add;
  hooks->update_inventory = update_inventory;
  hooks->set_has_alt_cameras = set_has_alt_cameras;
  hooks->has_alt_cameras = has_alt_cameras;
  hooks->custom_view = custom_view;
}

void Context::AddSetting(const char* key, const char* value) {
  settings_.emplace(key, value);
}

lua::NResultsOr Context::PushScript() {
  lua_State* L = lua_vm_.get();
  if (script_path_.empty()) {
    return "Must have level script!";
  }
  return lua::PushScriptFile(L, script_path_);
}

int Context::SetScriptName(std::string script_name) {
  if (script_name.length() > 4 &&
    // size_t pos, size_t len, const char* s, size_t n
    script_name.compare(script_name.length() - 4, 4, ".lua", 4) == 0) {
    script_path_ = std::move(script_name);
  } else if (!script_name.empty()) {
    script_path_ = absl::StrCat(ExecutableRunfiles(), kGameScriptPath,
                                kLevelsDirectory, "/",  script_name, ".lua");
  }
  auto result = PushScript();
  lua_State* L = lua_vm_.get();
  if (result.ok()) {
    lua_pop(L, result.n_results());
    return 0;
  } else {
    error_message_ = absl::StrCat("Level not found: ", result.error());
    return 1;
  }
}

int Context::Init() {
  // Clear texture handles from previous levels and initialise temp folder.
  int init_code = MutableGame()->Init();
  if (init_code != 0) {
    return init_code;
  }

  lua_State* L = lua_vm_.get();
  auto result = PushScript();

  if (!result.ok()) {
    error_message_ = result.error();
    return 1;
  }

  std::string scripts_folder =
      absl::StrCat(ExecutableRunfiles(), kGameScriptPath);

  lua_vm_.AddPathToSearchers(scripts_folder);

  // Add the current running script to the search path.
  auto last_slash_pos = script_path_.find_last_of('/');
  if (last_slash_pos != std::string::npos) {
    auto running_script_folder = script_path_.substr(0, last_slash_pos);
    lua_vm_.AddPathToSearchers(running_script_folder);
  }

  lua_vm_.AddCModuleToSearchers("dmlab.system.image", LuaImageRequire);
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.tensor", tensor::LuaTensorConstructors);
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.maze_generation", LuaMazeGeneration::Require);
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.map_maker", &lua::Bind<MapMakerModule>, {this});
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.game", &lua::Bind<ContextGame::Module>, {MutableGame()});

  lua_vm_.AddCModuleToSearchers("dmlab.system.events",
                                &lua::Bind<ContextEvents::Module>,
                                {MutableEvents()});
  lua_vm_.AddCModuleToSearchers("dmlab.system.game_entities",
                                &lua::Bind<ContextEntities::Module>,
                                {MutableGameEntities()});
  lua_vm_.AddCModuleToSearchers("dmlab.system.pickups_spawn",
                                &lua::Bind<ContextPickups::Module>,
                                {MutablePickups()});
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.random", &lua::Bind<LuaRandom::Require>, {UserPrbg()});
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.model", &lua::Bind<ModelModule>,
      {const_cast<DeepmindCalls*>(Game().Calls())});
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.transform", LuaTransform::Require);

  lua::Push(L, script_path_);
  result = lua::Call(L, 1);
  if (!result.ok()) {
    error_message_ = result.error();
    return 1;
  }
  if (result.n_results() != 1) {
    error_message_ =
        "Lua script must return only a table or userdata with metatable.";
    lua_pop(L, result.n_results());
    return 1;
  }
  if (!lua::Read(L, -1, &script_table_ref_)) {
    error_message_ = absl::StrCat(
        "Lua script must return a table or userdata with metatable. Actually "
        "returned : '",
        lua::ToString(L, -1), "'");
    lua_pop(L, result.n_results());
    return 1;
  }
  lua_pop(L, result.n_results());
  int err = CallInit();
  if (err != 0) return err;

  MutablePickups()->SetScriptTableRef(script_table_ref_);
  return MutableObservations()->ReadSpec(script_table_ref_);
}

int Context::Start(int episode, int seed) {
  EnginePrbg()->seed(seed);
  MutableGame()->NextMap();
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("start");
  if (!lua_isnil(L, -2)) {
    lua::Push(L, episode);
    lua::Push(L, static_cast<double>(MakeRandomSeed()));
    auto result = lua::Call(L, 3);
    if (!result.ok()) {
      error_message_ = result.error();
      return 1;
    }
    lua_pop(L, result.n_results());
  } else {
    lua_pop(L, 2);
  }
  return 0;
}

int Context::MapLoaded() {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("mapLoaded");
  if (!lua_isnil(L, -2)) {
    auto result = lua::Call(L, 1);
    if (!result.ok()) {
      error_message_ = result.error();
      return 1;
    }
    lua_pop(L, result.n_results());
  } else {
    lua_pop(L, 2);
  }
  return 0;
}

bool Context::HasEpisodeFinished(double elapsed_episode_time_seconds) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("hasEpisodeFinished");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return elapsed_episode_time_seconds >= kDefaultEpisodeLengthSeconds;
  }
  lua::Push(L, elapsed_episode_time_seconds);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[hasEpisodeFinished] - " << result.error();
  CHECK(result.n_results() == 1) << "[hasEpisodeFinished] - Expect single "
                                    "return value of true or false.";
  bool finish_episode = false;
  CHECK(lua::Read(L, -1, &finish_episode))
      << "[hasEpisodeFinished] - Must return a boolean.";
  lua_pop(L, result.n_results());
  return finish_episode;
}

const char* Context::GetCommandLine(const char* old_commandline) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("commandLine");
  if (!lua_isnil(L, -2)) {
    lua::Push(L, old_commandline);
    auto result = lua::Call(L, 2);
    CHECK(result.ok()) << result.error();
    CHECK_EQ(1, result.n_results()) << "'commandLine' must return a string.";
    CHECK(lua::Read(L, -1, &command_line_))
        << "'commandLine' must return a string: Found " << lua::ToString(L, -1);
    lua_pop(L, result.n_results());
    return command_line_.c_str();
  } else {
    lua_pop(L, 2);
    return old_commandline;
  }
}

int Context::RunLuaSnippet(const char* buf, std::size_t buf_len) {
  lua_State* L = lua_vm_.get();
  int out = 0;
  lua::NResultsOr result = lua::PushScript(L, buf, buf_len, "snippet");
  if (result.ok()) {
    lua::Push(L, script_table_ref_);
    result = lua::Call(L, 1);
    if (result.ok() && result.n_results() != 0) {
      lua::Read(L, -1, &out);
    }
    lua_pop(L, result.n_results());
  }

  CHECK(result.ok()) << result.error();
  std::cout << std::flush;
  return out;
}

const char* Context::NextMap() {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("nextMap");
  CHECK(!lua_isnil(L, -2)) << "Missing Lua function nextMap";
  auto result = lua::Call(L, 1);
  CHECK(result.ok()) << result.error();
  CHECK_EQ(1, result.n_results()) << "'nextMap' must return one string.";
  CHECK(lua::Read(L, -1, &map_name_))
      << "'nextMap' must return one string: Found " << lua::ToString(L, -1);
  MutableGame()->NextMap();
  lua_pop(L, result.n_results());
  return map_name_.c_str();
}

void Context::UpdateInventory(bool is_spawning, int player_id, int gadget_count,
                              int gadget_inventory[], int stat_count,
                              int stat_inventory[], int powerup_count,
                              int powerup_time[], int gadget_held, float height,
                              float position[3], float view_angles[3]) {
  const char* update_type = is_spawning ? "spawnInventory" : "updateInventory";
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction(update_type);
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return;
  }

  auto table = lua::TableRef::Create(L);
  table.Insert("playerId", player_id + 1);
  table.Insert("amounts", absl::MakeConstSpan(gadget_inventory, gadget_count));
  table.Insert("stats", absl::MakeConstSpan(stat_inventory, stat_count));
  table.Insert("powerups", absl::MakeConstSpan(powerup_time, powerup_count));
  table.Insert("position", absl::MakeConstSpan(position, 3));
  table.Insert("angles", absl::MakeConstSpan(view_angles, 3));
  table.Insert("height", height);
  table.Insert("gadget", gadget_held + 1);
  lua::Push(L, table);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[" << update_type << "] - " << result.error();
  if (result.n_results() > 0) {
    CHECK_EQ(1, result.n_results())
        << "[" << update_type << "] - Must return table or nil!";
    if (!lua_isnil(L, -1)) {
      CHECK(lua::Read(L, -1, &table))
          << "[" << update_type << "] - Must return table or nil!";
      CHECK(table.LookUp("amounts",
                         absl::MakeSpan(gadget_inventory, gadget_count)))
          << "[" << update_type << "] - Table missing 'amounts'!";
      CHECK(table.LookUp("stats", absl::MakeSpan(stat_inventory, stat_count)))
          << "[" << update_type << "] - Table missing 'stats'!";
    }
    lua_pop(L, result.n_results());
  }
}

int Context::GameType() {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("gameType");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;  // GT_FFA from bg_public.
  } else {
    auto result = lua::Call(L, 1);
    CHECK(result.ok()) << "[gameType] - " << result.error();
    int game_type = 0;
    CHECK(lua::Read(L, -1, &game_type))
        << "[gameType] - must return integer; actual \"" << lua::ToString(L, -1)
        << "\"";
    CHECK_LT(game_type, 8)
        << "[gameType] - must return integer less than 8; actual \""
        << game_type << "\"";
    return game_type;
  }
}

char Context::TeamSelect(int player_id, const char* player_name) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("team");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return '\0';
  }

  lua::Push(L, player_id + 1);
  lua::Push(L, player_name);

  auto result = lua::Call(L, 3);
  CHECK(result.ok()) << result.error();
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return '\0';
  }
  CHECK_EQ(1, result.n_results()) << "[team] - must return one string.";
  std::string team;
  CHECK(lua::Read(L, -1, &team)) << "[team] - must return one string: Found \""
                                 << lua::ToString(L, -1) << "\"";

  lua_pop(L, result.n_results());
  CHECK(!team.empty()) << "[team] - must return one character or nil: Found \""
                       << lua::ToString(L, -1) << "\"";

  const char kTeam[] = "pbrs";
  auto ret = std::find_if(std::begin(kTeam), std::end(kTeam),
                          [&team](char t) { return team[0] == t; });
  if (ret != std::end(kTeam)) {
    return *ret;
  }

  LOG(FATAL) << "[team] - must return one of 'r', 'b', 'p' and 's'; actual"
             << lua::ToString(L, -1);
}

void Context::SetActions(double look_down_up, double look_left_right,
                         signed char move_back_forward,
                         signed char strafe_left_right, signed char crouch_jump,
                         int buttons_down) {
  actions_.look_down_up = look_down_up;
  actions_.look_left_right = look_left_right;
  actions_.move_back_forward = move_back_forward;
  actions_.strafe_left_right = strafe_left_right;
  actions_.crouch_jump = crouch_jump;
  actions_.buttons_down = buttons_down;
}

void Context::GetActions(double* look_down_up, double* look_left_right,
                         signed char* move_back_forward,
                         signed char* strafe_left_right,
                         signed char* crouch_jump, int* buttons_down) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("modifyControl");
  // Check function exists.
  if (!lua_isnil(L, -2)) {
    auto table = lua::TableRef::Create(L);
    table.Insert("lookDownUp", actions_.look_down_up);
    table.Insert("lookLeftRight", actions_.look_left_right);
    table.Insert("moveBackForward", actions_.move_back_forward);
    table.Insert("strafeLeftRight", actions_.strafe_left_right);
    table.Insert("crouchJump", actions_.crouch_jump);
    table.Insert("buttonsDown", actions_.buttons_down);
    lua::Push(L, table);
    auto result = lua::Call(L, 2);
    CHECK(result.ok()) << result.error();

    if (result.n_results() >= 1) {
      lua::Read(L, -1, &table);
      CHECK(table.LookUp("lookDownUp", look_down_up));
      CHECK(table.LookUp("lookLeftRight", look_left_right));
      CHECK(table.LookUp("moveBackForward", move_back_forward));
      CHECK(table.LookUp("strafeLeftRight", strafe_left_right));
      CHECK(table.LookUp("crouchJump", crouch_jump));
      CHECK(table.LookUp("buttonsDown", buttons_down));
      lua_pop(L, result.n_results());
      return;
    }
  } else {
    lua_pop(L, 2);
  }

  *look_down_up = actions_.look_down_up;
  *look_left_right = actions_.look_left_right;
  *move_back_forward = actions_.move_back_forward;
  *strafe_left_right = actions_.strafe_left_right;
  *crouch_jump = actions_.crouch_jump;
  *buttons_down = actions_.buttons_down;
}

int Context::MakeRandomSeed() {
  return std::uniform_int_distribution<int>(
      1, std::numeric_limits<int>::max())(*EnginePrbg());
}

bool Context::FindModel(const char* model_name) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("createModel");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, model_name);

  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "createModel: " << result.error();

  // If no description is returned or the description is nil, don't create
  // model.
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  // Read model
  std::unique_ptr<Model> model(new Model());
  CHECK(Read(L, -1, model.get()))
      << "createModel: Failed to parse data for model " << model_name;
  model_name_ = model_name;
  model_ = std::move(model);

  lua_pop(L, result.n_results());
  return true;
}

void Context::GetModelGetters(DeepmindModelGetters* model_getters,
                              void** model_data) {
  CHECK(model_) << "No model was selected for this context!";

  *model_getters = ModelGetters();
  *model_data = model_.get();
}


bool Context::CanTrigger(int entity_id, const char* target_name) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("canTrigger");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return true;
  }

  lua::Push(L, entity_id);
  lua::Push(L, target_name);

  auto result = lua::Call(L, 3);
  CHECK(result.ok()) << "[canTrigger] - " << result.error();

  CHECK(result.n_results() != 0 && !lua_isnil(L, -1))
      << "canTrigger: return value from lua canTrigger must be true or false.";

  bool can_trigger;
  CHECK(lua::Read(L, -1, &can_trigger))
      << "canTrigger: Failed to read the return value as a boolean."
      << "Return true or false.";

  lua_pop(L, result.n_results());
  return can_trigger;
}

bool Context::OverrideTrigger(int entity_id, const char* target_name) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("trigger");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, entity_id);
  lua::Push(L, target_name);

  auto result = lua::Call(L, 3);
  CHECK(result.ok()) << "[trigger] - " << result.error();

  // If nothing was returned or if the first return value is nil, the trigger
  // behaviour was not overridden
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  bool has_override;
  CHECK(lua::Read(L, -1, &has_override))
      << "trigger: Failed to read the return value as a boolean."
      << "Return true or false.";

  lua_pop(L, result.n_results());
  return has_override;
}

void Context::TriggerLookat(int entity_id, bool looked_at,
                            const float position[3]) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("lookat");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return;
  }

  lua::Push(L, entity_id);
  lua::Push(L, looked_at);
  std::array<float, 3> float_array3;
  std::copy_n(position, float_array3.size(), float_array3.data());
  lua::Push(L, float_array3);

  auto result = lua::Call(L, 4);
  CHECK(result.ok()) << "[lookat] - " << result.error();

  lua_pop(L, result.n_results());
}

int Context::RewardOverride(const char* optional_reason, int player_id,
                            int team, const int* optional_other_player_id,
                            const float* optional_origin, int score) {
  if (optional_reason != nullptr) {
    lua_State* L = script_table_ref_.LuaState();
    script_table_ref_.PushMemberFunction("rewardOverride");
    // Check function exists.
    if (!lua_isnil(L, -2)) {
      auto args = lua::TableRef::Create(L);
      args.Insert("reason", optional_reason);
      args.Insert("playerId", player_id + 1);
      if (team >= 0 && team
          < std::distance(std::begin(kTeamNames), std::end(kTeamNames))) {
        args.Insert("team", kTeamNames[team]);
      }
      if (optional_other_player_id != nullptr) {
        args.Insert("otherPlayerId", *optional_other_player_id + 1);
      }
      if (optional_origin != nullptr) {
        std::array<float, 3> float_array3 = {
            {optional_origin[0], optional_origin[1], optional_origin[2]}};
        args.Insert("location", float_array3);
      }
      args.Insert("score", score);
      lua::Push(L, args);
      auto result = lua::Call(L, 2);
      CHECK(result.ok()) << "[scoreOverride] - " << result.error();
      CHECK(result.n_results() <= 1)
          << "[scoreOverride] - Must return new score or nil";
      if (result.n_results() == 1 && !lua_isnil(L, -1)) {
        CHECK(lua::Read(L, -1, &score))
            << "[scoreOverride] - Score must be an integer!";
      }
      lua_pop(L, result.n_results());
    } else {
      lua_pop(L, 2);
    }
  }
  return ExternalReward(player_id) + score;
}

int Context::ExternalReward(int player_id) {
  CHECK_GE(player_id, 0) << "Invalid player Id!";
  double reward = 0;
  if (static_cast<std::size_t>(player_id) < player_rewards_.size()) {
    player_rewards_[player_id] = std::modf(player_rewards_[player_id], &reward);
  }
  return reward;
}

// Adds reward to a player ready for transfer to the server.
void Context::AddScore(int player_id, double reward) {
  CHECK_GE(player_id, 0) << "Invalid player Id!";
  if (static_cast<std::size_t>(player_id) >= player_rewards_.size()) {
    player_rewards_.resize(player_id + 1);
  }
  player_rewards_[player_id] += reward;
}

void Context::AddBots() {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("addBots");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return;
  }
  auto result = lua::Call(L, 1);
  CHECK(result.ok()) << "[addBots] - " << result.error();
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return;
  }
  lua::TableRef table;
  CHECK(Read(L, -1, &table)) << "[addBots] - Failed to addBot table!";
  for (std::size_t i = 0; i < table.ArraySize(); ++i) {
    lua::TableRef bot_table;
    CHECK(table.LookUp(i + 1, &bot_table)) << "Failed to read bot - " << i + 1
                                           << "!";
    std::string bot_name;
    CHECK(bot_table.LookUp("name", &bot_name))
        << "[addBots] - Must supply bot name!";
    double skill = 5.0;
    bot_table.LookUp("skill", &skill);
    std::string team = "free";
    bot_table.LookUp("team", &team);
    Game().Calls()->add_bot(bot_name.c_str(), skill, team.c_str());
  }
  lua_pop(L, result.n_results());
}

bool Context::ReplaceModelName(const char* name, char* new_name,
                               int new_name_size, char* texture_prefix,
                               int texture_prefix_size) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("replaceModelName");
  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, name);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[replaceModelName] - " << result.error();

  if (lua_isnoneornil(L, 1)) {
    CHECK(lua_isnoneornil(L, 2))
        << "[replaceModelName] - Return arg2 (texturePrefix) must be nil if "
           "return arg1 (newModelName) is nil.";
    lua_pop(L, result.n_results());
    return false;
  }

  std::string replacement_name;
  CHECK(lua::Read(L, 1, &replacement_name))
      << "[replaceModelName] - Return arg1 (newModelName) must be a string.";
  CHECK_LT(replacement_name.size(), new_name_size)
      << "[replaceModelName] - Return arg1 (newModelName) is too long.";

  std::string string_prefix;
  if (result.n_results() == 2 && !lua_isnil(L, 2)) {
    CHECK(lua::Read(L, 2, &string_prefix))
        << "[replaceModelName] - Return arg2 (texturePrefix) must be a string.";
    CHECK_LT(string_prefix.size(), texture_prefix_size)
        << "[replaceModelName] - Return arg2 (texturePrefix) is too long.";
  }

  std::copy_n(replacement_name.c_str(), replacement_name.size() + 1, new_name);
  std::copy_n(string_prefix.c_str(), string_prefix.size() + 1, texture_prefix);
  lua_pop(L, result.n_results());
  return true;
}

bool Context::ReplaceTextureName(const char* name, char* new_name,
                                 int max_size) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("replaceTextureName");
  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, name);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[replaceTextureName] - " << result.error();
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }
  std::string replacement_name;
  CHECK(lua::Read(L, -1, &replacement_name))
      << "[replaceTextureName] - New name must be a string.";
  CHECK_LT(replacement_name.size(), max_size)
      << "[replaceTextureName] - New name is too long.";
  std::copy_n(replacement_name.c_str(), replacement_name.size() + 1, new_name);
  lua_pop(L, result.n_results());
  return true;
}

bool Context::LoadTexture(const char* name, unsigned char** pixels, int* width,
                          int* height, void* (*allocator)(int size)) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("loadTexture");
  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, name);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[loadTexture] - " << result.error();
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }
  auto* image_tensor = tensor::LuaTensor<unsigned char>::ReadObject(L, -1);
  CHECK(image_tensor) << "[loadTexture] - Must return ByteTensor.";
  const auto& view = image_tensor->tensor_view();
  CHECK_EQ(3, view.shape().size())
      << "[loadTexture] - Must return ByteTensor shaped HxWx4";
  CHECK_EQ(4, view.shape()[2])
      << "[loadTexture] - Must return ByteTensor shaped HxWx4";
  *height = view.shape()[0];
  *width = view.shape()[1];
  *pixels = static_cast<unsigned char*>(allocator(view.num_elements()));
  unsigned char* pixel_it = *pixels;
  view.ForEach([&pixel_it](unsigned char value) { *pixel_it++ = value; });
  lua_pop(L, result.n_results());
  return true;
}

bool Context::ModifyRgbaTexture(const char* name, unsigned char* data,
                                int width, int height) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("modifyTexture");
  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, name);
  tensor::TensorView<std::uint8_t> view(
      tensor::Layout({static_cast<std::size_t>(height),
                      static_cast<std::size_t>(width), 4}),
      data);
  auto storage_validity = std::make_shared<tensor::StorageValidity>();
  tensor::LuaTensor<std::uint8_t>::CreateObject(L, std::move(view),
                                                storage_validity);
  auto result = lua::Call(L, 3);
  CHECK(result.ok()) << "[modifyTexture] - " << result.error();

  bool modified_texture;
  CHECK(lua::Read(L, -1, &modified_texture))
      << "[modifyTexture] - must return true or false";
  lua_pop(L, result.n_results());
  storage_validity->Invalidate();
  return modified_texture;
}

int Context::CallInit() {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("init");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }
  lua::Push(L, settings_);
  auto result = lua::Call(L, 2);
  if (!result.ok()) {
    error_message_ = result.error();
    return 1;
  }

  int ret_val = 0;
  bool correct_args = result.n_results() == 0 ||
                      (result.n_results() == 1 && lua_isnil(L, 1)) ||
                      (result.n_results() <= 2 && lua::Read(L, 1, &ret_val));
  if (ret_val != 0) {
    if (result.n_results() == 2) {
      error_message_ = lua::ToString(L, 2);
    } else {
      error_message_ = "Error while calling 'init'.";
    }
  }
  lua_pop(L, result.n_results());
  if (!correct_args) {
    error_message_ = "[init] - Must return none, nil, or integer and message\n";
    return 1;
  }
  return ret_val;
}

int Context::MakeScreenMessages(int screen_width, int screen_height,
                                int line_height, int string_buffer_size) {
  screen_messages_.clear();
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("screenMessages");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }

  CHECK_GE(string_buffer_size, 0) << "[screenMessages] - Bad buffer size";

  auto args = lua::TableRef::Create(L);
  args.Insert("width", screen_width);
  args.Insert("height", screen_height);
  args.Insert("line_height", line_height);
  args.Insert("max_string_length", string_buffer_size - 1);
  lua::Push(L, args);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[screenMessages] - " << result.error();
  CHECK(result.n_results() == 1)
      << "[screenMessages] - Must return an array of messages";
  lua::TableRef messages_array;
  lua::Read(L, -1, &messages_array);
  for (std::size_t i = 0, size = messages_array.ArraySize(); i != size; ++i) {
    lua::TableRef message_table;
    CHECK(messages_array.LookUp(i + 1, &message_table))
        << "[screenMessages] - Each message must be a table";
    ScreenMessage message = {};
    CHECK(message_table.LookUp("message", &message.text) &&
          message.text.length() < static_cast<std::size_t>(string_buffer_size))
        << "[screenMessages] - Must contain a string 'message' field and the "
           "message must no longer than "
        << string_buffer_size - 1;
    message_table.LookUp("x", &message.x);
    message_table.LookUp("y", &message.y);
    message_table.LookUp("alignment", &message.align_l0_r1_c2);
    std::fill(message.rgba.begin(), message.rgba.end(), 1.0);
    message_table.LookUp("rgba", &message.rgba);
    message.shadow = true;
    message_table.LookUp("shadow", &message.shadow);
    screen_messages_.push_back(std::move(message));
  }

  lua_pop(L, result.n_results());
  return screen_messages_.size();
}

void Context::GetScreenMessage(int message_id, char* buffer, int* x, int* y,
                               int* align_l0_r1_c2, int* shadow,
                               float rgba[4]) const {
  const auto& screen_message = screen_messages_[message_id];
  const std::string& msg_text = screen_message.text;
  // MakeScreenMessages guarantees message is smaller than string_buffer_size.
  std::copy_n(msg_text.c_str(), msg_text.size() + 1, buffer);
  *x = screen_message.x;
  *y = screen_message.y;
  *align_l0_r1_c2 = screen_message.align_l0_r1_c2;
  *shadow = screen_message.shadow ? 1 : 0;
  std::copy_n(screen_message.rgba.data(), screen_message.rgba.size(), rgba);
}

int Context::MakeFilledRectangles(int screen_width, int screen_height) {
  filled_rectangles_.clear();
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("filledRectangles");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }

  auto args = lua::TableRef::Create(L);
  args.Insert("width", screen_width);
  args.Insert("height", screen_height);
  lua::Push(L, args);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[filledRectangles] - " << result.error();
  CHECK_EQ(1, result.n_results())
      << "[filledRectangles] - Must return an array of rectangles";
  lua::TableRef rectangles_array;
  lua::Read(L, -1, &rectangles_array);
  for (std::size_t i = 0, size = rectangles_array.ArraySize(); i != size; ++i) {
    lua::TableRef rectangle_table;
    CHECK(rectangles_array.LookUp(i + 1, &rectangle_table))
        << "[filledRectangles] - Each message must be a table";
    FilledRectangle filled_rectangle = {};
    CHECK(rectangle_table.LookUp("x", &filled_rectangle.x))
        << "[filledRectangles] - Must supply x";
    CHECK(rectangle_table.LookUp("y", &filled_rectangle.y))
        << "[filledRectangles] - Must supply y";
    CHECK(rectangle_table.LookUp("width", &filled_rectangle.width))
        << "[filledRectangles] - Must supply width";
    CHECK(rectangle_table.LookUp("height", &filled_rectangle.height))
        << "[filledRectangles] - Must supply height";
    CHECK(rectangle_table.LookUp("rgba", &filled_rectangle.rgba))
        << "[filledRectangles] - Must supply rgba";
    filled_rectangles_.push_back(filled_rectangle);
  }

  lua_pop(L, result.n_results());
  return filled_rectangles_.size();
}

void Context::GetFilledRectangle(int rectangle_id, int* x, int* y, int* width,
                                 int* height, float rgba[4]) const {
  const auto& filled_rectangle = filled_rectangles_[rectangle_id];
  *x = filled_rectangle.x;
  *y = filled_rectangle.y;
  *width = filled_rectangle.width;
  *height = filled_rectangle.height;
  std::copy_n(filled_rectangle.rgba.data(), filled_rectangle.rgba.size(), rgba);
}

void Context::MakePk3FromMap(const char* map_path, const char* map_name,
                             bool gen_aas) {
  MapCompileSettings compile_settings;
  compile_settings.generate_aas = gen_aas;
  compile_settings.map_source_location =
      absl::StrCat(ExecutableRunfiles(), "/", map_path);
  compile_settings.use_local_level_cache = use_local_level_cache_;
  compile_settings.use_global_level_cache = use_global_level_cache_;
  compile_settings.level_cache_params = level_cache_params_;
  std::string target = absl::StrCat(TempDirectory(), "/baselab/", map_name);
  CHECK(RunMapCompileFor(ExecutableRunfiles(), target, compile_settings));
}

void Context::CustomPlayerMovement(int mover_id, const float mover_pos[3],
                                   const float player_pos[3],
                                   const float player_vel[3],
                                   float player_pos_delta[3],
                                   float player_vel_delta[3]) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("playerMover");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return;
  }

  std::array<float, 3> float_array3;

  auto args = lua::TableRef::Create(L);
  args.Insert("moverId", mover_id);

  std::copy_n(mover_pos, float_array3.size(), float_array3.data());
  args.Insert("moverPos", float_array3);

  std::copy_n(player_pos, float_array3.size(), float_array3.data());
  args.Insert("playerPos", float_array3);

  std::copy_n(player_vel, float_array3.size(), float_array3.data());
  args.Insert("playerVel", float_array3);

  lua::Push(L, args);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[playerMover] - " << result.error();

  std::array<float, 3> pos_delta = {{0.0f, 0.0f, 0.0f}};
  std::array<float, 3> vel_delta = {{0.0f, 0.0f, 0.0f}};

  CHECK(lua_isnoneornil(L, 1) || lua::Read(L, 1, &pos_delta))
      << "[playerMover] - First return value must be a table containing"
         "player position delta values.";
  CHECK(lua_isnoneornil(L, 2) || lua::Read(L, 2, &vel_delta))
      << "[playerMover] - Second return value must be a table containing"
         "player velocity delta values.";

  std::copy_n(pos_delta.data(), pos_delta.size(), player_pos_delta);
  std::copy_n(vel_delta.data(), vel_delta.size(), player_vel_delta);

  lua_pop(L, result.n_results());
}

void Context::GameEvent(const char* event_name, int count,
                        const float* data) {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("gameEvent");
    // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return;
  }

  lua::Push(L, event_name);
  lua_createtable(L, count, 0);
  for (std::size_t i = 0; i < count; ++i) {
    lua::Push(L, i + 1);
    lua::Push(L, data[i]);
    lua_settable(L, -3);
  }
  auto result = lua::Call(L, 3);
  CHECK(result.ok()) << result.error() << '\n';
  lua_pop(L, result.n_results());
}

}  // namespace lab
}  // namespace deepmind
