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

#include "deepmind/engine/context_pickups.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"

namespace deepmind {
namespace lab {
namespace {

constexpr int kMaxSpawnVars = 64;
constexpr int kMaxSpawnVarChars = 4096;
constexpr int kMaxPickupChars = 256;

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

void ReadSpawnVars(const ContextPickups::EntityInstance& spawn_vars,
                   char* spawn_var_chars, int* num_spawn_var_chars,
                   int spawn_var_offsets[][2], int* num_spawn_vars) {
  *num_spawn_var_chars = 0;
  *num_spawn_vars = spawn_vars.size();
  CHECK_NE(0, *num_spawn_vars) << "Must have spawn vars or return nil. (Make "
                                  "sure all values are strings.)";
  CHECK_LT(*num_spawn_vars, kMaxSpawnVars) << "Too many spawn vars!";
  char* mem = spawn_var_chars;
  auto it = spawn_vars.begin();
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
}

class LuaPickupsModule : public lua::Class<LuaPickupsModule> {
  friend class Class;
  static const char* ClassName() { return "deepmind.lab.Pickups"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaPickupsModule(ContextPickups* ctx) : ctx_(ctx) {}

  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"spawn", Member<&LuaPickupsModule::Spawn>},
    };
    Class::Register(L, methods);
  }

 private:
  // Spawn entity at position
  // [0, 1, -]
  lua::NResultsOr Spawn(lua_State* L) {
    ContextPickups::EntityInstance spawn_entity;
    if (lua::Read(L, 2, &spawn_entity)) {
      ctx_->SpawnDynamicEntity(std::move(spawn_entity));
      return 0;
    } else {
      return "[pickups.spawn] - Must be called with a string-string Lua table.";
    }
  }

  ContextPickups* ctx_;
};

}  // namespace

lua::NResultsOr ContextPickups::Module(lua_State* L) {
  if (auto* ctx = static_cast<ContextPickups*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaPickupsModule::Register(L);
    LuaPickupsModule::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing context!";
  }
}

void ContextPickups::SpawnDynamicEntity(EntityInstance spawn_entity) {
  dynamic_spawn_entities_.push_back(std::move(spawn_entity));
}

void ContextPickups::ReadDynamicSpawnEntity(int entity_index,
                                            char* spawn_var_chars,
                                            int* num_spawn_var_chars,
                                            int spawn_var_offsets[][2],
                                            int* num_spawn_vars) const {
  ReadSpawnVars(dynamic_spawn_entities_[entity_index], spawn_var_chars,
                num_spawn_var_chars, spawn_var_offsets, num_spawn_vars);
}

bool ContextPickups::UpdateSpawnVars(char* spawn_var_chars,
                                     int* num_spawn_var_chars,
                                     int spawn_var_offsets[][2],
                                     int* num_spawn_vars) {
  lua_State* L = script_table_ref_.LuaState();
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

  EntityInstance entity;
  lua::Read(L, -1, &entity);
  lua_pop(L, result.n_results());
  ReadSpawnVars(entity, spawn_var_chars, num_spawn_var_chars,
                spawn_var_offsets, num_spawn_vars);
  return true;
}

// Clears extra_spawn_vars_ and reads new entities from Lua.
// Returns number of entities created.
int ContextPickups::MakeExtraEntities() {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("extraEntities");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }
  auto result = lua::Call(L, 1);
  CHECK(result.ok()) << result.error();
  // Nil return so no spawns returned.
  if (lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return 0;
  }
  extra_entities_.clear();
  CHECK(lua::Read(L, -1, &extra_entities_))
      << "[extraEntities] - Invalid return value";
  lua_pop(L, result.n_results());
  return extra_entities_.size();
}

// Clears extra_spawn_vars_ and reads new entities from Lua.
// Returns number of entities created.
int ContextPickups::RegisterDynamicItems() {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("registerDynamicItems");
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }
  auto result = lua::Call(L, 1);
  CHECK(result.ok()) << result.error();
  // Nil return so no spawns returned.
  if (lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return 0;
  }
  dynamic_items_.clear();
  CHECK(lua::Read(L, -1, &dynamic_items_))
      << "[extraEntities] - Invalid return value";
  lua_pop(L, result.n_results());
  return dynamic_items_.size();
}

void ContextPickups::ReadDynamicItemName(int item_index,
                                         char* item_name) const {
  const auto& item = dynamic_items_[item_index];
  std::size_t length = item.length() + 1;
  CHECK_LE(length, kMaxPickupChars) << "Too long pickup name! - " << item;
  std::copy(item.c_str(), item.c_str() + length, item_name);
}

// Read specific spawn var from extra_spawn_vars_. Shall be called after
// with spawn_var_index in range [0, MakeExtraSpawnVars()).
void ContextPickups::ReadExtraEntity(  //
    int entity_index,                  //
    char* spawn_var_chars,             //
    int* num_spawn_var_chars,          //
    int spawn_var_offsets[][2],        //
    int* num_spawn_vars) {
  CHECK(0 <= entity_index && entity_index < extra_entities_.size());
  ReadSpawnVars(extra_entities_[entity_index], spawn_var_chars,
                num_spawn_var_chars, spawn_var_offsets, num_spawn_vars);
}

bool ContextPickups::FindItem(const char* class_name, int* index) {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("createPickup");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, class_name);

  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << result.error();

  // If no description is returned or the description is nil, don't create item.
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  lua::TableRef table;
  CHECK(Read(L, -1, &table)) << "Failed to read pickup table!";

  PickupItem item = {};
  CHECK(table.LookUp("name", &item.name));
  CHECK(table.LookUp("classname", &item.class_name));
  CHECK(table.LookUp("model", &item.model_name));
  CHECK(table.LookUp("quantity", &item.quantity));
  CHECK(table.LookUp("type", &item.type));
  table.LookUp("typeTag", &item.tag);

  // Optional fields.
  if (!table.LookUp("moveType", &item.move_type)) {
    // Legacy name.
    table.LookUp("tag", &item.move_type);
  }
  items_.push_back(item);
  *index = ItemCount() - 1;

  lua_pop(L, result.n_results());
  return true;
}

bool ContextPickups::GetItem(int index, char* item_name, int max_item_name,  //
                             char* class_name, int max_class_name,           //
                             char* model_name, int max_model_name,           //
                             int* quantity, int* type, int* tag,
                             int* move_type) const {
  CHECK_GE(index, 0) << "Index out of range!";
  CHECK_LT(index, ItemCount()) << "Index out of range!";

  const auto& item = items_[index];
  CHECK(StringCopy(item.name, item_name, max_item_name));
  CHECK(StringCopy(item.class_name, class_name, max_class_name));
  CHECK(StringCopy(item.model_name, model_name, max_model_name));
  *quantity = item.quantity;
  *type = static_cast<int>(item.type);
  *tag = item.tag;
  *move_type = item.move_type;
  return true;
}

bool ContextPickups::CanPickup(int entity_id, int player_id) {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("canPickup");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return true;
  }

  lua::Push(L, entity_id);
  lua::Push(L, player_id + 1);

  auto result = lua::Call(L, 3);
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

bool ContextPickups::OverridePickup(int entity_id, int* respawn,
                                    int player_id) {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("pickup");

  // Check function exists.
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return false;
  }

  lua::Push(L, entity_id);
  lua::Push(L, player_id + 1);

  auto result = lua::Call(L, 3);
  CHECK(result.ok()) << result.error();

  // If nothing returned or the return is nil, we're not overriding the
  // pickup behaviour.
  if (result.n_results() == 0 || lua_isnil(L, -1)) {
    lua_pop(L, result.n_results());
    return false;
  }

  CHECK(lua::Read(L, -1, respawn)) << "Failed to read the respawn time";

  lua_pop(L, result.n_results());
  return true;
}

}  // namespace lab
}  // namespace deepmind
