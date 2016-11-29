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

#include "deepmind/engine/context.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>

#include "deepmind/support/logging.h"
#include "deepmind/engine/lua_random.h"
#include "deepmind/engine/lua_maze_generation.h"
#include "deepmind/engine/lua_text_level_maker.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/lua_tensor.h"

using deepmind::lab::Context;

extern "C" {
static void add_setting(void* userdata, const char* key, const char* value) {
  return static_cast<Context*>(userdata)->AddSetting(key, value);
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

static const char* replace_command_line(void* userdata,
                                        const char* old_commandline) {
  return static_cast<Context*>(userdata)->GetCommandLine(old_commandline);
}

static const char* next_map(void* userdata) {
  return static_cast<Context*>(userdata)->NextMap();
}

static int run_lua_snippet(void* userdata, const char* command) {
  std::size_t command_len = std::strlen(command);
  return static_cast<Context*>(userdata)->RunLuaSnippet(command, command_len);
}

static void set_use_internal_controls(void* userdata, bool v) {
  return static_cast<Context*>(userdata)->SetUseInternalControls(v);
}

static bool get_use_internal_controls(void* userdata) {
  return static_cast<Context*>(userdata)->UseInternalControls();
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
  return static_cast<Context*>(userdata)->UpdateSpawnVars(
      spawn_var_chars, num_spawn_var_chars, spawn_var_offsets, num_spawn_vars);
}

static bool find_item(void* userdata, const char* class_name, int* index) {
  return static_cast<Context*>(userdata)->FindItem(class_name, index);
}

static int item_count(void* userdata) {
  return static_cast<Context*>(userdata)->ItemCount();
}

static bool item(void* userdata, int index,
                 char* item_name, int max_item_name,
                 char* class_name, int max_class_name,
                 char* model_name, int max_model_name,
                 int* quantity, int* type, int* tag) {
  return static_cast<Context*>(userdata)->GetItem(
      index, item_name, max_item_name, class_name, max_class_name, model_name,
      max_model_name, quantity, type, tag);
}

static void clear_items(void* userdata) {
  static_cast<Context*>(userdata)->ClearItems();
}

static bool map_finished(void* userdata) {
  return static_cast<Context*>(userdata)->MapFinished();
}

static void set_map_finished(void* userdata, bool map_finished) {
  static_cast<Context*>(userdata)->SetMapFinished(map_finished);
}

static bool can_pickup(void* userdata, int entity_id) {
  return static_cast<Context*>(userdata)->CanPickup(entity_id);
}

static bool override_pickup(void* userdata, int entity_id, int* respawn) {
  return static_cast<Context*>(userdata)->OverridePickup(entity_id, respawn);
}

static int external_reward(void* userdata, int player_id) {
  return static_cast<Context*>(userdata)->ExternalReward(player_id);
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

static void modify_rgba_texture(void* userdata, const char* name,
                                unsigned char* data, int width, int height) {
  static_cast<Context*>(userdata)->ModifyRgbaTexture(name, data, width, height);
}

static int custom_observation_count(void* userdata) {
  return static_cast<const Context*>(userdata)->CustomObservationCount();
}

static const char* custom_observation_name(void* userdata, int idx) {
  return static_cast<const Context*>(userdata)->CustomObservationName(idx);
}

static void custom_observation_spec(void* userdata, int idx,
                                    EnvCApi_ObservationSpec* spec) {
  static_cast<const Context*>(userdata)->CustomObservationSpec(idx, spec);
}

static void custom_observation(void* userdata, int idx,
                               EnvCApi_Observation* observation) {
  return static_cast<Context*>(userdata)->CustomObservation(idx, observation);
}

void predicted_player_state(void* userdata, const float origin[3],
                            const float velocity[3], const float viewangles[3],
                            float height, int timestamp_msec) {
  return static_cast<Context*>(userdata)->SetPredictPlayerState(
      origin, velocity, viewangles, height, timestamp_msec);
}

int make_screen_messages(void* userdata, int screen_width, int screen_height,
                         int line_height, int string_buffer_size) {
  return static_cast<Context*>(userdata)->MakeScreenMesages(
      screen_width, screen_height, line_height, string_buffer_size);
}

void get_screen_message(void* userdata, int message_id, char* buffer, int* x,
                        int* y, int* align_l0_r1_c2) {
  static_cast<Context*>(userdata)->GetScreenMessage(message_id, buffer, x, y,
                                                    align_l0_r1_c2);
}

}  // extern "C"

namespace deepmind {
namespace lab {
namespace {

class LuaGameModule : public lua::Class<LuaGameModule> {
  friend class Class;
  static const char* ClassName() { return "deepmind.lab.Game"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaGameModule(Context* ctx) : ctx_(ctx) {}

  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"addScore", Member<&LuaGameModule::AddScore>},
        {"finishMap", Member<&LuaGameModule::FinishMap>},
        {"playerInfo", Member<&LuaGameModule::PlayerInfo>},
    };
    Class::Register(L, methods);
  }

 private:
  lua::NResultsOr AddScore(lua_State* L) {
    int player_id = 0;
    double score = 0;
    if (lua::Read(L, 2, &player_id) && lua::Read(L, 3, &score) &&
        0 <= player_id && player_id < 64) {
      ctx_->Calls()->add_score(player_id, score);
      return 0;
    }
    std::string error = "Invalid arguments player_id: ";
    error += lua::ToString(L, 2);
    error += " or reward: ";
    error += lua::ToString(L, 3);
    return std::move(error);
  }

  lua::NResultsOr FinishMap(lua_State* L) {
    ctx_->SetMapFinished(true);
    return 0;
  }

  lua::NResultsOr PlayerInfo(lua_State* L) {
    const auto& pv = ctx_->GetPredictPlayerView();
    auto table = lua::TableRef::Create(L);
    table.Insert("pos", pv.pos);
    table.Insert("vel", pv.vel);
    table.Insert("angles", pv.angles);
    table.Insert("anglesVel", pv.anglesVel);
    table.Insert("height", pv.height);
    lua::Push(L, table);
    return 1;
  }

  Context* ctx_;
};

constexpr char kGameScriptPath[] = "/baselab/game_scripts";
constexpr int kMaxSpawnVars = 64;
constexpr int kMaxSpawnVarChars = 4096;
constexpr double kDefaultEpisodeLengthSeconds = 5 * 30.0;

// If the string "arg" fits into the array pointed to by "dest" (including the
// null terminator), copies the string into the array and returns true;
// otherwise returns false.
bool StringCopy(const std::string& arg, char* dest, std::size_t max_size) {
  auto len = arg.length() + 1;
  if (len <= max_size) {
    std::copy_n(arg.c_str(), len, dest);
    return true;
  } else {
    return false;
  }
}

lua::NResultsOr MapMakerModule(lua_State* L) {
  if (auto* ctx =
          static_cast<Context*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaTextLevelMaker::CreateObject(L, ctx->ExecutableRunfiles());
    return 1;
  } else {
    return "Missing context!";
  }
}

lua::NResultsOr GameModule(lua_State* L) {
  if (auto* ctx =
          static_cast<Context*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaGameModule::Register(L);
    LuaGameModule::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing context!";
  }
}

lua::NResultsOr RandomModule(lua_State* L) {
  if (auto* ctx =
          static_cast<Context*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaRandom::CreateObject(L, ctx->UserPrbg());
    return 1;
  } else {
    return "Missing context!";
  }
}

// Returns the unique value in the range [-180, 180) that is equivalent to
// 'angle', where two values x and y are considered equivalent whenever x - y
// is an integral multiple of 360. (Note: the result may be  meaningless if the
// magnitude of 'angle' is very large.)
double CanonicalAngle360(double angle) {
  const double n = std::floor((angle + 180.0) / 360.0);
  return angle - n * 360.0;
}

}  // namespace

Context::Context(lua::Vm lua_vm, const char* executable_runfiles,
                 const DeepmindCalls* calls, DeepmindHooks* hooks)
    : lua_vm_(std::move(lua_vm)),
      executable_runfiles_(executable_runfiles),
      deepmind_calls_(calls),
      use_internal_controls_(false),
      actions_{},
      map_finished_(false),
      random_seed_(0),
      predicted_player_view_{} {
  CHECK(lua_vm_ != nullptr);
  hooks->add_setting = add_setting;
  hooks->set_script_name = set_script_name;
  hooks->start = start;
  hooks->init = init;
  hooks->replace_command_line = replace_command_line;
  hooks->next_map = next_map;
  hooks->run_lua_snippet = run_lua_snippet;
  hooks->set_use_internal_controls = set_use_internal_controls;
  hooks->get_use_internal_controls = get_use_internal_controls;
  hooks->set_actions = set_actions;
  hooks->get_actions = get_actions;
  hooks->update_spawn_vars = update_spawn_vars;
  hooks->find_item = find_item;
  hooks->item_count = item_count;
  hooks->item = item;
  hooks->clear_items = clear_items;
  hooks->map_finished = map_finished;
  hooks->set_map_finished = set_map_finished;
  hooks->can_pickup = can_pickup;
  hooks->override_pickup = override_pickup;
  hooks->external_reward = external_reward;
  hooks->add_score = add_score;
  hooks->make_random_seed = make_random_seed;
  hooks->has_episode_finished = has_episode_finished;
  hooks->add_bots = add_bots;
  hooks->modify_rgba_texture = modify_rgba_texture;
  hooks->custom_observation_count = custom_observation_count;
  hooks->custom_observation_name = custom_observation_name;
  hooks->custom_observation_spec = custom_observation_spec;
  hooks->custom_observation = custom_observation;
  hooks->predicted_player_state = predicted_player_state;
  hooks->make_screen_messages = make_screen_messages;
  hooks->get_screen_message = get_screen_message;
}

void Context::AddSetting(const char* key, const char* value) {
  settings_.emplace(key, value);
}

lua::NResultsOr Context::PushScript() {
  lua_State* L = lua_vm_.get();
  if (script_name_.empty()) {
    return "Must have level script!";
  }
  if (script_name_.length() > 4 &&
      // size_t pos, size_t len, const char* s, size_t n
      script_name_.compare(script_name_.length() - 4, 4, ".lua", 4) == 0) {
    return lua::PushScriptFile(L, script_name_);
  } else {
    std::string script_path = executable_runfiles_;
    script_path += kGameScriptPath;
    script_path += "/";
    script_path += script_name_;
    script_path += ".lua";
    return lua::PushScriptFile(L, script_path);
  }
}

int Context::SetScriptName(const char* script_name) {
  script_name_ = script_name;
  auto result = PushScript();
  lua_State* L = lua_vm_.get();
  if (result.ok()) {
    lua_pop(L, result.n_results());
    return 0;
  } else {
    std::cerr << "Level not found: " << result.error() << std::endl;
    return 1;
  }
}

int Context::Init() {
  lua_State* L = lua_vm_.get();
  auto result = PushScript();

  if (!result.ok()) {
    std::cerr << result.error() << std::endl;
    return 1;
  }

  std::string scripts_folder = executable_runfiles_;
  scripts_folder += kGameScriptPath;
  lua_vm_.AddPathToSearchers(scripts_folder);

  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.tensor", tensor::LuaTensorConstructors);
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.maze_generation", LuaMazeGeneration::Require);
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.map_maker", &lua::Bind<MapMakerModule>, {this});
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.game", &lua::Bind<GameModule>, {this});
  lua_vm_.AddCModuleToSearchers(
      "dmlab.system.random", &lua::Bind<RandomModule>, {this});

  lua::Push(L, script_name_);
  result = lua::Call(L, 1);
  if (!result.ok()) {
    std::cerr << result.error() << std::endl;
    return 1;
  }
  if (result.n_results() != 1) {
    std::cerr
        << "Lua script must return only a table or userdata with metatable.";
    lua_pop(L, result.n_results());
    return 1;
  }
  if (!lua::Read(L, -1, &script_table_ref_)) {
    auto actual = lua::ToString(L, -1);
    std::cerr << "Lua script must return a table or userdata with metatable. "
                 "Actually returned : '"
              << actual << "'" << std::endl;
    lua_pop(L, result.n_results());
    return 1;
  }
  lua_pop(L, result.n_results());
  int err = CallInit();
  if (err != 0) return err;
  return CallObservationSpec();
}

int Context::Start(int episode, int seed) {
  predicted_player_view_.timestamp_msec = 0;
  random_seed_ = seed;
  EnginePrbg()->seed(seed);
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("start");
  if (!lua_isnil(L, -2)) {
    lua::Push(L, episode);
    lua::Push(L, static_cast<double>(seed));
    auto result = lua::Call(L, 3);
    if (!result.ok()) {
      std::cerr << result.error() << std::endl;
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

int Context::RunLuaSnippet(const char* buff, std::size_t buff_len) {
  lua_State* L = lua_vm_.get();
  int out = 0;
  lua::NResultsOr result = lua::PushScript(L, buff, buff_len, "snippet");
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
  predicted_player_view_.timestamp_msec = 0;
  lua_pop(L, result.n_results());
  return map_name_.c_str();
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

bool Context::UpdateSpawnVars(char* spawn_var_chars, int* num_spawn_var_chars,
                              int spawn_var_offsets[][2], int* num_spawn_vars) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("updateSpawnVars");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return true;
  }

  auto table = lua::TableRef::Create(L);
  for (int i = 0; i < *num_spawn_vars; ++i) {
    table.Insert(std::string(spawn_var_chars + spawn_var_offsets[i][0]),
                 std::string(spawn_var_chars + spawn_var_offsets[i][1]));
  }
  lua::Push(L, table);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << result.error();

  // Nil return so spawn is ignored.
  if (lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  std::unordered_map<std::string, std::string> out_spawn_vars;
  lua::Read(L, -1, &out_spawn_vars);

  *num_spawn_vars = out_spawn_vars.size();
  CHECK_NE(0, *num_spawn_vars) << "Must have spawn vars or return nil. (Make "
                                  "sure all values are strings.)";
  CHECK_LT(*num_spawn_vars, kMaxSpawnVars) << "Too many spawn vars!";
  char* mem = spawn_var_chars;

  auto it = out_spawn_vars.begin();
  for (int i = 0; i < *num_spawn_vars; ++i) {
    const auto& key = it->first;
    const auto& value = it->second;
    ++it;
    std::size_t kl = key.length() + 1;
    std::size_t vl = value.length() + 1;
    *num_spawn_var_chars += kl + vl;
    CHECK_LT(*num_spawn_var_chars, kMaxSpawnVarChars) << "Too large spawn vars";
    std::copy(key.c_str(), key.c_str() + kl, mem);
    spawn_var_offsets[i][0] = std::distance(spawn_var_chars, mem);
    mem += kl;
    std::copy(value.c_str(), value.c_str() + vl, mem);
    spawn_var_offsets[i][1] = std::distance(spawn_var_chars, mem);
    mem += vl;
  }
  lua_pop(L, result.n_results());
  return true;
}

bool Context::FindItem(const char* class_name, int* index) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("createPickup");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, class_name);

  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << result.error();

  // If nothing it returned or that it's nil, don't create item.
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  lua::TableRef table;
  CHECK(Read(L, -1, &table)) << "Failed to read pickup table!";

  PickupItem item = {};
  CHECK(table.LookUp("name", &item.name));
  CHECK(table.LookUp("class_name", &item.class_name));
  CHECK(table.LookUp("model_name", &item.model_name));
  CHECK(table.LookUp("quantity", &item.quantity));
  CHECK(table.LookUp("type", &item.type));

  // Optional tag field.
  table.LookUp("tag", &item.tag);

  items_.push_back(item);
  *index = ItemCount() - 1;

  lua_pop(L, result.n_results());
  return true;
}

bool Context::GetItem(int index, char* item_name, int max_item_name,  //
                      char* class_name, int max_class_name,           //
                      char* model_name, int max_model_name,           //
                      int* quantity, int* type, int* tag) {
  CHECK_GE(index, 0) << "Index out of range!";
  CHECK_LT(index, ItemCount()) << "Index out of range!";

  const auto& item = items_[index];
  CHECK(StringCopy(item.name, item_name, max_item_name));
  CHECK(StringCopy(item.class_name, class_name, max_class_name));
  CHECK(StringCopy(item.model_name, model_name, max_model_name));
  *quantity = item.quantity;
  *type = static_cast<int>(item.type);
  *tag = item.tag;
  return true;
}

bool Context::CanPickup(int entity_id) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("canPickup");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return true;
  }

  lua::Push(L, entity_id);

  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << result.error();

  // If nothing returned or the return is nil, the default.
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return true;
  }

  bool can_pickup = true;
  CHECK(lua::Read(L, -1, &can_pickup))
      << "Failed to read canPickup return value";

  lua_pop(L, result.n_results());
  return can_pickup;
}

bool Context::OverridePickup(int entity_id, int* respawn) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("pickup");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, entity_id);

  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << result.error();

  // If nothing returned or the return is nil, we're not overriding the
  // pickup behaviour.
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  CHECK(lua::Read(L, -1, &respawn)) << "Failed to read the respawn time";

  lua_pop(L, result.n_results());
  return true;
}

int Context::ExternalReward(int player_id) {
  CHECK_GE(player_id, 0) << "Invalid player Id!";
  double reward = 0;
  if (static_cast<std::size_t>(player_id) < player_rewards_.size()) {
    if (player_rewards_[player_id] >= 1.0) {
      player_rewards_[player_id] =
          std::modf(player_rewards_[player_id], &reward);
    }
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
    Calls()->add_bot(bot_name.c_str(), skill, team.c_str());
  }
  lua_pop(L, result.n_results());
}

void Context::ModifyRgbaTexture(const char* name, unsigned char* data,
                                int width, int height) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("modifyTexture");
  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return;
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
  lua_pop(L, result.n_results());
  storage_validity->Invalidate();
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
  lua_pop(L, result.n_results());
  if (!result.ok()) {
    std::cerr << result.error() << '\n';
    return 1;
  }
  return 0;
}

int Context::CallObservationSpec() {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("customObservationSpec");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }
  auto result = lua::Call(L, 1);
  if (!result.ok()) {
    std::cerr << result.error() << '\n';
    return 1;
  }
  lua::TableRef observations;
  lua::Read(L, -1, &observations);
  observation_infos_.clear();
  observation_infos_.reserve(observations.ArraySize());
  for (std::size_t i = 0, c = observations.ArraySize(); i != c; ++i) {
    lua::TableRef observation_info;
    observations.LookUp(i + 1, &observation_info);
    ObservationSpecInfo info;
    if (!observation_info.LookUp("name", &info.name)) {
      std::cerr << "[customObservationSpec] - Missing 'name = <string>'.\n";
      return 1;
    }
    std::string type = "Doubles";
    observation_info.LookUp("type", &type);
    if (type.compare("Bytes") == 0) {
      info.type = EnvCApi_ObservationBytes;
    } else if (type.compare("Doubles") == 0) {
      info.type = EnvCApi_ObservationDoubles;
    } else {
      std::cerr
          << "[customObservationSpec] - Missing 'type = 'Bytes'|'Doubles''.\n";
      return 1;
    }
    if (!observation_info.LookUp("shape", &info.shape)) {
      std::cerr
          << "[customObservationSpec] - Missing 'shape = {<int>, ...}'.\n";
      return 1;
    }
    observation_infos_.push_back(std::move(info));
  }
  lua_pop(L, result.n_results());
  return 0;
}

void Context::CustomObservation(int idx, EnvCApi_Observation* observation) {
  lua_State* L = lua_vm_.get();
  script_table_ref_.PushMemberFunction("customObservation");
  // Function must exist.
  CHECK(!lua_isnil(L, -2))
      << "Observations Spec set but no observation member function";
  const auto& info = observation_infos_[idx];
  lua::Push(L, info.name);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[customObservation] - " << result.error();

  CHECK_EQ(1, result.n_results())
      << "[customObservation] - Must return a "
      << (info.type == EnvCApi_ObservationDoubles ? "DoubleTensor"
                                                  : "ByteTensor");

  const tensor::Layout* layout = nullptr;
  if (info.type == EnvCApi_ObservationDoubles) {
    auto* double_tensor = tensor::LuaTensor<double>::ReadObject(L, -1);
    if (double_tensor != nullptr) {
      const auto& view = double_tensor->tensor_view();
      CHECK(view.IsContiguous())
          << "[customObservation] - Must return a contiguous tensor!";
      layout = &view;
      observation->spec.type = EnvCApi_ObservationDoubles;
      observation->payload.doubles = view.storage() + view.start_offset();
    }
  } else {
    auto* byte_tensor = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
    if (byte_tensor != nullptr) {
      const auto& view = byte_tensor->tensor_view();
      layout = &view;
      CHECK(view.IsContiguous())
          << "[customObservation] - Must return a contiguous tensor!";
      observation->spec.type = EnvCApi_ObservationBytes;
      observation->payload.bytes = view.storage() + view.start_offset();
    }
  }
  CHECK(layout != nullptr)
      << "[customObservation] - Must return a contiguous tensor!\n:"
      << "at idx" << idx << "\n"
      << lua::ToString(L, -1);

  observation_tensor_shape_.resize(layout->shape().size());
  std::copy(layout->shape().begin(), layout->shape().end(),
            observation_tensor_shape_.begin());
  observation->spec.dims = observation_tensor_shape_.size();
  observation->spec.shape = observation_tensor_shape_.data();

  // Prevent observation->payload from being destroyed during pop.
  lua::Read(L, -1, &observation_tensor_);
  lua_pop(L, result.n_results());
}

void Context::CustomObservationSpec(int idx,
                                    EnvCApi_ObservationSpec* spec) const {
  const auto& info = observation_infos_[idx];
  spec->type = info.type;
  spec->dims = info.shape.size();
  spec->shape = info.shape.data();
}

void Context::SetPredictPlayerState(const float pos[3], const float vel[3],
                                    const float angles[3], float height,
                                    int timestamp_msec) {
  PlayerView before = predicted_player_view_;
  std::copy_n(pos, 3, predicted_player_view_.pos.begin());
  std::copy_n(vel, 3, predicted_player_view_.vel.begin());
  std::copy_n(angles, 3, predicted_player_view_.angles.begin());
  predicted_player_view_.height = height;
  predicted_player_view_.timestamp_msec = timestamp_msec;

  int delta_time_msec =
      predicted_player_view_.timestamp_msec - before.timestamp_msec;

  // When delta_time_msec < 3 the velocities become inaccurate.
  if (before.timestamp_msec > 0 && delta_time_msec > 0) {
    double inv_delta_time = 1000.0 / delta_time_msec;
    for (int i : {0, 1, 2}) {
      predicted_player_view_.anglesVel[i] =
          CanonicalAngle360(predicted_player_view_.angles[i] -
                            before.angles[i]) *
          inv_delta_time;
    }
  } else {
    predicted_player_view_.anglesVel.fill(0);
  }
}

int Context::MakeScreenMesages(int screen_width, int screen_height,
                               int line_height,  int string_buffer_size) {
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
  for (std::size_t i = 0, e = messages_array.ArraySize(); i != e; ++i) {
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
    screen_messages_.push_back(std::move(message));
  }

  lua_pop(L, result.n_results());
  return screen_messages_.size();
}

void Context::GetScreenMessage(int message_id, char* buffer, int* x, int* y,
                               int* align_l0_r1_c2) const {
  const auto& screen_message = screen_messages_[message_id];
  const std::string& msg_text = screen_message.text;
  // MakeScreenMesages guarantees message is smaller than string_buffer_size.
  std::copy_n(msg_text.c_str(), msg_text.size() + 1, buffer);
  *x = screen_message.x;
  *y = screen_message.y;
  *align_l0_r1_c2 = screen_message.align_l0_r1_c2;
}

}  // namespace lab
}  // namespace deepmind
