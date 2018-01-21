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
//
// Internal data structure that holds the state of our ML system.

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_H_

#include <array>
#include <cstddef>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "deepmind/engine/context_entities.h"
#include "deepmind/engine/context_events.h"
#include "deepmind/engine/context_game.h"
#include "deepmind/engine/context_observations.h"
#include "deepmind/engine/context_pickups.h"
#include "deepmind/include/deepmind_calls.h"
#include "deepmind/include/deepmind_hooks.h"
#include "deepmind/include/deepmind_model_getters.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/model_getters.h"
#include "public/level_cache_types.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {

// This is the userdata in DeepmindContext. It contains the Lua VM and
// methods for handling callbacks from DeepMind Lab.
class Context {
 public:
  // Constructed with a Lua VM.
  // 'executable_runfiles' path to where DeepMind Lab assets are stored.
  // 'calls' allow the context to call into the engine. (Owned by engine.)
  // 'hooks' allow the engine to call into the context.
  // 'file_reader_override' an optional function for reading from the file
  // system. If set, a call returns whether the file 'file_name' was read
  // successfully and if so 'buff' points to the content and 'size' contains the
  // size of the file and after use 'buff' must be freed with 'free'. Otherwise
  // returns false.
  // 'temp_folder' optional folder to store temporary objects.
  Context(
      lua::Vm lua_vm,
      const char* executable_runfiles,
      const DeepmindCalls* calls,
      DeepmindHooks* hooks,
      bool (*file_reader_override)(const char* file_name, char** buff,
                                   std::size_t* size),
      const char* temp_folder);

  // Inserts 'key' 'value' into settings_.
  // Must be called before Init.
  void AddSetting(const char* key, const char* value);

  // 'script_name': name of a Lua file; this script is run during first call to
  // Init.
  // Must be called before Init.
  // Returns zero if successful and non-zero on error.
  int SetScriptName(std::string script_name);

  // Runs the script named script_name_ and stores the result in
  // script_table_ref_.
  // Calls "init" member function on the script_table_ref_ with settings_.
  // Returns zero if successful and non-zero on error.
  int Init();

  // Calls "start" member function on the script_table_ref_ with episode and
  // seed.
  // Returns zero if successful and non-zero on error.
  int Start(int episode, int seed);

  // Calls "mapLoaded" member function on the script_table_ref_ .
  // Returns zero if successful and non-zero on error.
  int MapLoaded();

  // The return value is only valid until the next call to GetCommandLine().
  // Must be called after Init.
  const char* GetCommandLine(const char* old_commandline);

  // The return value is only valid until the next call to NextMap().
  // Must be called after Init.
  const char* NextMap();

  // Returns the game mode from script. The return value must be a valid value
  // of the gametype_t enum (see code/game/bg_public.h). If no matching callback
  // is implemented in Lua, returns GT_FFA ("free for all").
  int GameType();

  // Returns the team selection for a player.
  // '\0' - No selection.
  // 'p' - Free.
  // 'r' - Red Team.
  // 'b' - Blue Team.
  // 's' - Spectator.
  // Called by the engine with each player's chosen ID and name.
  char TeamSelect(int player_id, const char* player_name);

  // The script is called with the script_table_ref pushed on the stack.
  // Runs the contents in the lua_vm_. If the script returns an integer this
  // function will return it, too, else it returns 0.
  int RunLuaSnippet(const char* buf, std::size_t buf_len);

  // Sets the current actions applied from the controller.
  void SetActions(                    //
      double look_down_up,            //
      double look_left_right,         //
      signed char move_back_forward,  //
      signed char strafe_left_right,  //
      signed char crouch_jump,        //
      int buttons_down);

  // Retrieves the current actions applied by the controller. These values are
  // initially those set by SetActions, but they may be modified by the user
  // if a Lua callback named "modifyControl" is provided. If provided, the
  // callback is called with a single argument table containing six key-value
  // pairs for the six actions ("lookDownUp", "lookLeftRight", ...). It is
  // expected to return a table with the same keys containing the (new) values
  // of the six actions.
  void GetActions(                     //
      double* look_down_up,            //
      double* look_left_right,         //
      signed char* move_back_forward,  //
      signed char* strafe_left_right,  //
      signed char* crouch_jump,        //
      int* buttons_down);

  // This returns whether we are running a native app and the internal
  // controller will call SetActions.
  bool NativeApp() { return native_app_; }

  // Sets whether we are running a native app and the internal controller will
  // call SetActions.
  void SetNativeApp(bool v) { native_app_ = v; }

  // Adds all the bots specified in the script. Called on each map load.
  void AddBots();

  // Finds a model by model_name, and registers this item with the Context's
  // model array.
  bool FindModel(const char* model_name);

  // Return the accessor API for currently selected model.
  void GetModelGetters(DeepmindModelGetters* model_getters, void** model_data);

  // Clear the current list of registered models. Called just before loading a
  // new map.
  void ClearModel() { model_.reset(); }

  // Returns whether we should finish the episode. Called at the end of every
  // frame.
  bool HasEpisodeFinished(double elapsed_episode_time_seconds);

  // Customization point for overriding the value of a reward.
  //
  // * 'optional_reason' - The reason is either a nullptr or a string containing
  //   the reason this reward is being awarded.
  //
  // * 'player_id' Is the player the reward applies to.
  //
  // * 'team' is the team id the player belongs to.
  //
  // * 'optional_other_player_id' is a nullptr or the other player involved in
  //   the reward.
  //
  // * 'optional_origin' is either a nullptr or 3 floats containing the
  //   location of the reward.
  //
  // Returns the modified reward combined with the reward provided by
  // 'ExternalReward'.
  int RewardOverride(const char* optional_reason, int player_id, int team,
                     const int* optional_other_player_id,
                     const float* optional_origin, int score);

  // Adds the given reward for the specified player. The reward is accumulated
  // temporarily until it is harvested by ExternalReward.
  void AddScore(int player_id, double reward);

  // Path to where DeepMind Lab assets are stored.
  const std::string& ExecutableRunfiles() const {
    return Game().ExecutableRunfiles();
  }

  const std::string& TempDirectory() const { return Game().TempFolder(); }

  // Returns a new random seed on each call. Internally uses 'engine_prbg_' to
  // generate new positive integers.
  int MakeRandomSeed();

  std::mt19937_64* UserPrbg() { return &user_prbg_; }

  std::mt19937_64* EnginePrbg() { return &engine_prbg_; }

  // Modify a texture after loading.
  void ModifyRgbaTexture(const char* name, unsigned char* data, int width,
                         int height);

  // Calls script to retrieve a list of screen messages. The message returned
  // from the script shall be strictly smaller than buffer_size, since the
  // buffer needs space for the null padding. 'screen_width' and 'screen_height'
  // are the size of the screen.
  int MakeScreenMessages(int screen_width, int screen_height, int line_height,
                         int string_buffer_size);

  // Retrieve screen message. 'buffer' is filled with a null terminated string.
  // The room in the buffer is 'string_buffer_size' from the MakeScreenMessage
  // command. 'x' and 'y' are the screen coordinates in terms of the screen
  // 'height' and 'width' also from the MakeScreenMesages.
  // 'message_id' shall be greater than or equal to zero and less than what
  // was returned by the last call of MakeScreenMesages.
  // 'align_l0_r1_c2' is how the text is horizontally aligned. '0' for left,
  // '1' for right and '2' for center.  'shadow' is whether to render a black
  // offset drop shadow. 'rgba' is the color and alpha of the text.
  void GetScreenMessage(int message_id, char* buffer, int* x, int* y,
                        int* align_l0_r1_c2, int* shadow, float rgba[4]) const;

  // Calls script to retrieve a list of filled rectangles. 'screen_width' and
  // 'screen_height' are the size of the screen.
  int MakeFilledRectangles(int screen_width, int screen_height);

  // Retrieve filled rectangle.
  // 'rectangle_id' shall be greater than or equal to zero and less than what
  // was returned by the last call of MakeFilledRectangles.
  // 'x', 'y' is the position and 'width' and 'height' is the size of the
  // rectangle in screen-coordinates. They shall be all greater or equal to
  // zero. (Off-screen rendering is allowed.)
  // 'rgba' is the color and alpha of the rendered rectangle. 'rgba' values
  // shall be in the range [0, 1].
  // The parts of the rectangles that are out of bounds are not rendered.
  void GetFilledRectangle(int rectangle_id, int* x, int* y, int* width,
                          int* height, float rgba[4]) const;


  // Generates a pk3 from the map in `map_path` named `map_name`.
  // `gen_aas` should be set if bots are used with level.
  void MakePk3FromMap(const char* map_path, const char* map_name, bool gen_aas);

  // Sets which level caches to use. See MapCompileSettings in compile_map.h.
  void SetLevelCacheSetting(bool local, bool global,
                            DeepMindLabLevelCacheParams level_cache_params) {
    use_local_level_cache_ = local;
    use_global_level_cache_ = global;
    level_cache_params_ = level_cache_params;
  }

  bool UseLocalLevelCache() const { return use_local_level_cache_; }
  bool UseGlobalLevelCache() const { return use_global_level_cache_; }
  DeepMindLabLevelCacheParams LevelCacheParams() const {
    return level_cache_params_;
  }

  const char* ErrorMessage() const { return error_message_.c_str(); }

  // Sets current error message. 'message' shall be a null terminated string.
  void SetErrorMessage(const char* message) {
    error_message_ = std::string(message);
  }

  // Sets whether there are alternative cameras. This will make the server send
  // all entities, so they are visible to all cameras.
  void SetHasAltCameras(bool has_alt_cameras) {
    has_alt_cameras_ = has_alt_cameras;
  }

  // Returns whether the server sends all entities, so they are visible to all
  // cameras.
  bool HasAltCameras() const { return has_alt_cameras_; }

  const ContextGame& Game() const { return game_; }
  ContextGame* MutableGame() { return &game_; }

  const ContextEvents& Events() const { return events_; }
  ContextEvents* MutableEvents() { return &events_; }

  const ContextObservations& Observations() const { return observations_; }
  ContextObservations* MutableObservations() { return &observations_; }

  const ContextPickups& Pickups() const { return pickups_; }
  ContextPickups* MutablePickups() { return &pickups_; }

  const ContextEntities& GameEntities() const { return game_entities_; }
  ContextEntities* MutableGameEntities() { return &game_entities_; }

 private:
  // Message to be placed on screen.
  struct ScreenMessage {
    std::string text;
    int x;
    int y;
    int align_l0_r1_c2;
    std::array<float, 4> rgba;
    bool shadow;
  };

  struct FilledRectangle {
    int x;
    int y;
    int width;
    int height;
    std::array<float, 4> rgba;
  };

  // Current action state.
  struct Actions {
    double look_down_up;
    double look_left_right;
    signed char move_back_forward;
    signed char strafe_left_right;
    signed char crouch_jump;
    int buttons_down;
  };

  // Subtracts the integral part from the stashed reward (see AddScore) and
  // returns that integral part. The remaining stashed reward is smaller than
  // one in magnitude. The returned (integral) value is suitable for the game
  // server, which only deals in integral reward increments.
  int ExternalReward(int player_id);

  int CallInit();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  // Pushes script_name_ (see Init above) on to Lua stack ready for calling.
  // Fails if there is a syntax error in the script. Can be used for early
  // detection of syntax errors, too.
  // [1, 0, e]
  lua::NResultsOr PushScript();

  // The context's Lua VM. The top of the stack of the VM is zero before and
  // after any call.
  lua::Vm lua_vm_;

  // Path to the executable's assets.
  std::string executable_runfiles_;

  // The settings to run the script with.
  std::unordered_map<std::string, std::string> settings_;

  // The name of the script to run on first Init.
  std::string script_path_;

  // The result of the script that was run when Init was first called.
  lua::TableRef script_table_ref_;

  // Calls into the engine.
  const DeepmindCalls* deepmind_calls_;

  // Cached command-line to enable returning a pointer to its contents.
  std::string command_line_;

  // Cached map name to enable returning a pointer to its contents.
  std::string map_name_;

  // Stores whether we are running a native app and the internal controller will
  // call SetActions.
  bool native_app_;

  // Current actions to apply when lab is advanced.
  Actions actions_;

  // Current custom pickup model. Reset each episode.
  std::string model_name_;
  std::unique_ptr<Model> model_;

  // Transient reward stash for each player. Rewards are added with AddScore and
  // removed by ExternalReward.
  std::vector<double> player_rewards_;

  // A pseudo-random-bit generator for exclusive use by users.
  std::mt19937_64 user_prbg_;

  // A pseudo-random-bit generator for exclusive use of the engine. Seeded each
  // episode with the episode start seed.
  std::mt19937_64 engine_prbg_;

  // A list of screen messages to display this frame.
  std::vector<ScreenMessage> screen_messages_;

  // A list of filled rectangles to display this frame.
  std::vector<FilledRectangle> filled_rectangles_;

  bool use_local_level_cache_;
  bool use_global_level_cache_;

  // Callbacks for fetching/writing levels to cache.
  DeepMindLabLevelCacheParams level_cache_params_;

  // Last error message.
  std::string error_message_;

  // An object for storing and retrieving events.
  ContextEvents events_;

  // An object for calling into the engine.
  ContextGame game_;

  // An object for storing and retrieving custom observations.
  ContextObservations observations_;

  // An object for interacting with pickups.
  ContextPickups pickups_;

  // An object for retrieving information about in game entities.
  ContextEntities game_entities_;

  // When enabled all entities are forced to be rendered.
  bool has_alt_cameras_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_H_
