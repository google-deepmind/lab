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
//
// Internal data structure that holds the state of our ML system.

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_H_

#include <array>
#include <cstddef>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "deepmind/include/deepmind_calls.h"
#include "deepmind/include/deepmind_hooks.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm.h"

namespace deepmind {
namespace lab {

// Parameters for a custom pickup item.
struct PickupItem {
  std::string name;        // Name that will show when picking up item.
  std::string class_name;  // Class name to spawn entity as. Must be unique.
  std::string model_name;  // Model for pickup item.
  int quantity;            // Amount to award on pickup.
  int type;                // Type of pickup. E.g. health, ammo, frags etc.
                           // Must match itemType_t in bg_public.h
  int tag;                 // Tag used in conjunction with type. E.g. determine
                           // which weapon to award, or if a goal should bob and
                           // rotate.
};

// Represents a player's state in world units.
struct PlayerView {
  std::array<double, 3> pos;        // Position (forward, left, up).
  std::array<double, 3> vel;        // World velocity (forward, left, up).
  std::array<double, 3> angles;     // Orientation degrees (pitch, yaw, roll).
  std::array<double, 3> anglesVel;  // Angular velocity in degrees.
  int timestamp_msec;               // Engine time in msec of the view.
  double height;                    // View height.
};

// This is the userdata in DeepmindContext. It contains the Lua VM and
// methods for handling callbacks from DeepMind Lab.
class Context {
 public:
  // Constructed with a Lua VM.
  // 'executable_runfiles' path to where DeepMind Lab assets are stored.
  // 'calls' allow the context to call into the engine. (Owned by engine.)
  // 'hooks' allow the engine to call into the context.
  Context(lua::Vm lua_vm, const char* executable_runfiles,
          const DeepmindCalls* calls, DeepmindHooks* hooks);

  // Inserts 'key' 'value' into settings_.
  // Must be called before Init.
  void AddSetting(const char* key, const char* value);

  // 'script_name': name of a Lua file; this script is ran during first call to
  // Init.
  // Must be called before Init.
  // Returns zero if successful and non-zero on error.
  int SetScriptName(const char* script_name);

  // Runs the script named script_name_ and stores the result in
  // script_table_ref_.
  // Calls "init" member function on the script_table_ref_ with settings_.
  // Returns zero if successful and non-zero on error.
  int Init();

  // Calls "start" member function on the script_table_ref_ with episode and
  // seed.
  // Returns zero if successful and non-zero on error.
  int Start(int episode, int seed);

  // The return value is only valid until the next call to GetCommandLine().
  // Must be called after Init.
  const char* GetCommandLine(const char* old_commandline);

  // The return value is only valid until the next call to NextMap().
  // Must be called after Init.
  const char* NextMap();

  // The script is called with the script_table_ref pushed on the the stack.
  // Runs the contents in the lua_vm_. If the script returns an integer this
  // function will return it too, else it returns 0.
  int RunLuaSnippet(const char* buf, std::size_t buf_len);

  // Sets the current actions applied from the controller.
  void SetActions(                    //
      double look_down_up,            //
      double look_left_right,         //
      signed char move_back_forward,  //
      signed char strafe_left_right,  //
      signed char crouch_jump,        //
      int buttons_down);

  // Retrieves the current actions applied by the controller.
  void GetActions(                     //
      double* look_down_up,            //
      double* look_left_right,         //
      signed char* move_back_forward,  //
      signed char* strafe_left_right,  //
      signed char* crouch_jump,        //
      int* buttons_down);

  // This returns whether the internal controller will call SetActions.
  bool UseInternalControls() { return use_internal_controls_; }

  // Sets whether the internal controller will call SetActions.
  void SetUseInternalControls(bool v) { use_internal_controls_ = v; }

  // Allows Lua to replace contents of this c-style dictionary.
  // 'spawn_var_chars' is pointing at the memory holding the strings.
  // '*num_spawn_var_chars' is the total length of all strings and nulls.
  // 'spawn_var_offsets' is the  key, value offsets in 'spawn_var_chars'.
  // '*num_spawn_vars' is the number of valid spawn_var_offsets.
  //
  // So the dictionary { key0=value0, key1=value1, key2=value2 } would be
  // represented as:
  // spawn_var_chars[4096] = "key0\0value0\0key1\0value1\0key2\0value2";
  // *num_spawn_var_chars = 36
  // spawn_var_offsets[64][2] = {{0,5}, {12,17}, {24,29}};
  // *num_spawn_vars = 3
  // The update will not increase *num_spawn_var_chars to greater than 4096.
  // and not increase *num_spawn_vars to greater than 64.
  bool UpdateSpawnVars(            //
      char* spawn_var_chars,       //
      int* num_spawn_var_chars,    //
      int spawn_var_offsets[][2],  //
      int* num_spawn_vars);

  // Finds a pickup item by class_name, and registers this item with the
  // Context's item array. This is so all the VMs can update their respective
  // item lists by iterating through that array during update.
  // Returns whether the item was found, and if so, writes the index at which
  // the item now resides to *index.
  bool FindItem(const char* class_name, int* index);

  // Adds all the bots specified in the script. Called on each map load.
  void AddBots();

  // Get the current number of registered items.
  int ItemCount() const { return items_.size(); }

  // Get an item at a particular index and fill in the various buffers
  // provided.
  // Returns whether the operation succeeded.
  bool GetItem(            //
      int index,           //
      char* item_name,     //
      int max_item_name,   //
      char* class_name,    //
      int max_class_name,  //
      char* model_name,    //
      int max_model_name,  //
      int* quantity,       //
      int* type,           //
      int* tag);

  // Clear the current list of registered items. Called just before loading a
  // new map.
  void ClearItems() { items_.clear(); }

  // Returns whether we should finish the current map.
  bool MapFinished() const { return map_finished_; }

  // Sets whether the current map should finish.
  void SetMapFinished(bool map_finished) { map_finished_ = map_finished; }

  // Returns whether we should finish the episode. Called at the end of every
  // frame.
  bool HasEpisodeFinished(double elapsed_episode_time_seconds);

  // Returns whether we can pickup the specified entity id. By default this
  // returns true.
  bool CanPickup(int entity_id);

  // Customization point for overriding the entity's pickup behaviour. Also
  // allows for modifying the default respawn time for the entity.
  // Returns true if the pickup behaviour has been overridden by the user,
  // otherwise calls the default pickup behaviour based on the item type.
  bool OverridePickup(int entity_id, int* respawn);

  // Subtracts the integral part from the stashed reward (see AddScore) and
  // returns that integral part. The remaining stashed reward is smaller than
  // one in magnitude. The returned (integral) value is suitable for the game
  // server, which only deals in integral reward increments.
  int ExternalReward(int player_id);

  // Adds the given reward for the specified player. The reward is accumulated
  // temporarily until it is harvested by ExternalReward.
  void AddScore(int player_id, double reward);

  // Path to where DeepMind Lab assets are stored.
  const std::string& ExecutableRunfiles() const { return executable_runfiles_; }

  // Returns a new random seed on each call. Internally uses 'engine_prbg_' to
  // generate new positive integers.
  int MakeRandomSeed();

  const DeepmindCalls* Calls() const { return deepmind_calls_; }

  // Gets the seed this episode was launched with.
  int GetEpisodeSeed() const { return random_seed_; }

  std::mt19937_64* UserPrbg() { return &user_prbg_; }

  std::mt19937_64* EnginePrbg() { return &engine_prbg_; }

  // Modify a texture after loading.
  void ModifyRgbaTexture(const char* name, unsigned char* data, int width,
                         int height);

  // Script observation count.
  int CustomObservationCount() const { return observation_infos_.size(); }

  // Script observation name.
  const char* CustomObservationName(int idx) const {
    return observation_infos_[idx].name.c_str();
  }

  // Script observation spec.
  void CustomObservationSpec(int idx, EnvCApi_ObservationSpec* spec) const;

  // Script observation.
  void CustomObservation(int idx, EnvCApi_Observation* obs);

  // Set latest predicted player state.
  void SetPredictPlayerState(const float pos[3], const float vel[3],
                             const float angles[3], float height,
                             int timestamp_msec);

  // Get latest predicted player view. (This is where the game renders from.)
  const PlayerView GetPredictPlayerView() {
    return predicted_player_view_;
  }

  // Calls script to retrieve a list of screen messages. The message returned
  // from the script shall be strictly smaller than buffer_size, since the
  // buffer needs space for the null padding. 'screen_width' and 'screen_height'
  // are the size of the screen.
  int MakeScreenMesages(int screen_width, int screen_height, int line_height,
                        int string_buffer_size);

  // Retrieve screen message. 'buffer' is filled with a null terminated string.
  // The room in the buffer is 'string_buffer_size' from the MakeScreenMessage
  // command. 'x' and 'y' are the screen coordinates in terms of the screen
  // 'height' and 'width' also in from the MakeScreenMesages.
  // 'message_id' shall be greater than or equal to zero and less then what
  // was returned by the last call of MakeScreenMesages.
  // 'align_l0_r1_c2' is how the text is horizontally aligned. '0' for left,
  // '1' for right and '2' for center.
  void GetScreenMessage(int message_id, char* buffer, int* x, int* y,
                        int* align_l0_r1_c2) const;

 private:
  // Message to be placed on screen.
  struct ScreenMessage {
    std::string text;
    int x;
    int y;
    int align_l0_r1_c2;
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

  // Entry for a custom observation spec.
  struct ObservationSpecInfo {
    std::string name;
    EnvCApi_ObservationType type;
    std::vector<int> shape;
  };

  int CallInit();

  int CallObservationSpec();

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

  // The name of the script ran on first Init.
  std::string script_name_;

  // The result of the script that was run when Init was first called.
  lua::TableRef script_table_ref_;

  // Calls into the engine.
  const DeepmindCalls* deepmind_calls_;

  // Cached command-line to enable returning a pointer to its contents.
  std::string command_line_;

  // Cached map name to enable returning a pointer to its contents.
  std::string map_name_;

  // Stores whether the internal controller will call SetActions.
  bool use_internal_controls_;

  // Current actions to apply when lab is advanced.
  Actions actions_;

  // Array of current custom pickup items. Reset each episode.
  std::vector<PickupItem> items_;

  // Flag that can be set from the game to finish the current map.
  bool map_finished_;

  // Transient reward stash for each player. Rewards are added with AddScore and
  // removed by ExternalReward.
  std::vector<double> player_rewards_;

  // Random seed used for this episode.
  int random_seed_;

  // A pseudo-random-bit generator for exclusive use by users.
  std::mt19937_64 user_prbg_;

  // A pseudo-random-bit generator for exclusive use of the engine. Seeded each
  // episode with 'random_seed_'.
  std::mt19937_64 engine_prbg_;

  // Storage of supplementary observation types from script.
  std::vector<ObservationSpecInfo> observation_infos_;

  // Used to hold the EnvCApi_ObservationSpec::shape values until the next call
  // of observation.
  std::vector<int> observation_tensor_shape_;

  // Used to hold a reference to the observation tensor until the next call of
  // observation.
  lua::TableRef observation_tensor_;

  PlayerView predicted_player_view_;

  // A list of screen messages to display this frame.
  std::vector<ScreenMessage> screen_messages_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_H_
