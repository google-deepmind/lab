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

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_PICKUPS_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_PICKUPS_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "deepmind/include/deepmind_calls.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/table_ref.h"

namespace deepmind {
namespace lab {

class ContextPickups {
 public:
  // Parameters for a custom pickup item.
  using EntityInstance = absl::flat_hash_map<std::string, std::string>;

  void SetScriptTableRef(lua::TableRef script_table_ref) {
    script_table_ref_ = std::move(script_table_ref);
  }

  // Returns an entity module. A pointer to ContextPickups must exist in the up
  // value. [0, 1, e]
  static lua::NResultsOr Module(lua_State* L);

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

  // Clears extra_spawn_vars_ and reads new spawn vars from lua.
  // Returns number of spawn vars created.
  int MakeExtraEntities();

  // Read specific spawn var from extra_spawn_vars_. Shall be called after
  // with entity_index in range [0, MakeExtraEntities()).
  void ReadExtraEntity(            //
      int entity_index,            //
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
      int* tag,            //
      int* move_type) const;

  // Clear the current list of registered items. Called just before loading a
  // new map.
  void ClearItems() { items_.clear(); }

  // Returns whether we can pickup the specified entity id. By default this
  // returns true.
  bool CanPickup(int entity_id, int player_id);

  // Customization point for overriding the entity's pickup behaviour. Also
  // allows for modifying the default respawn time for the entity.
  // Returns true if the pickup behaviour has been overridden by the user,
  // otherwise calls the default pickup behaviour based on the item type.
  bool OverridePickup(int entity_id, int* respawn, int player_id);

  // Adds spawn_entity to list of entities to be spawned next frame.
  void SpawnDynamicEntity(EntityInstance spawn_entity);

  // The number of dynamic entities to spawn during next game update.
  int DynamicSpawnEntityCount() const { return dynamic_spawn_entities_.size(); }

  // Return a set of spawn vars describing an entity to be dynamically spawned;
  // entity_index must be in the range [0, DynamicSpawnEntityCount()).
  void ReadDynamicSpawnEntity(int entity_index, char* spawn_var_chars,
                              int* num_spawn_var_chars,
                              int spawn_var_offsets[][2],
                              int* num_spawn_vars) const;

  // Clear list of entities to be spawned next frame.
  void ClearDynamicSpawnEntities() { dynamic_spawn_entities_.clear(); }

  // Reads from Lua a list of classnames which must be dynamically loadable;
  // returns the number of classnames read.
  int RegisterDynamicItems();

  // Fetches one dynamically loadable classname; item_index must be in the range
  // [0, RegisterDynamicItems()).
  void ReadDynamicItemName(int item_index, char* item_name) const;

 private:
  // Parameters for a custom pickup item.
  struct PickupItem {
    std::string name;        // Name that will show when picking up item.
    std::string class_name;  // Class name to spawn entity as. Must be unique.
    std::string model_name;  // Model for pickup item.
    int quantity;            // Amount to award on pickup.
    int type;                // Type of pickup. E.g. health, ammo, frags etc.
                             // Must match itemType_t in bg_public.h
    int tag;                 // Tag used in conjunction with type. E.g.
                             // determine which weapon to award.
    int move_type;           // Used to determine how the pickup moves.
  };

  lua::TableRef script_table_ref_;

  // Array of current custom pickup items. Reset each episode.
  std::vector<PickupItem> items_;

  // Array of extra spawn vars for this level.
  std::vector<EntityInstance> extra_entities_;

  // Array of dynamically spawned entities_
  std::vector<EntityInstance> dynamic_spawn_entities_;

  // Array of classnames of items that may be dynamically spawned.
  std::vector<std::string> dynamic_items_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_PICKUPS_H_
