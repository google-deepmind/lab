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

#include "deepmind/engine/context_entities.h"

#include <algorithm>
#include <string>

#include "deepmind/lua/class.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"

namespace deepmind {
namespace lab {
namespace {

class LuaEntitiesModule : public lua::Class<LuaEntitiesModule> {
  friend class Class;
  static const char* ClassName() { return "deepmind.lab.Entities"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaEntitiesModule(ContextEntities* ctx) : ctx_(ctx) {}

  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"entities", Member<&LuaEntitiesModule::Entities>},
    };
    Class::Register(L, methods);
  }

 private:
  // Returns a list of entities to Lua.
  // [0, 1, -]
  lua::NResultsOr Entities(lua_State* L) {
    constexpr int kEntityNotVisibilityFlag = 0x80;
    lua::TableRef table = lua::TableRef::Create(L);
    int row_idx = 0;
    std::vector<std::string> filter;
    lua::Read(L, 2, &filter);
    for (const auto& row : ctx_->Entities()) {
      if (filter.empty() || std::find(filter.begin(), filter.end(),
                                      row.class_name) != filter.end()) {
        lua::TableRef entity = table.CreateSubTable(++row_idx);
        entity.Insert("entityId", row.entity_id + 1);
        entity.Insert("id", row.user_id);
        entity.Insert("type", row.type);
        entity.Insert("visible", (row.flags & kEntityNotVisibilityFlag) == 0);
        entity.Insert("position", row.position);
        entity.Insert("classname", row.class_name);
      }
    }
    lua::Push(L, table);
    return 1;
  }

  ContextEntities* ctx_;
};

}  // namespace

lua::NResultsOr ContextEntities::Module(lua_State* L) {
  if (auto* ctx = static_cast<ContextEntities*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaEntitiesModule::Register(L);
    LuaEntitiesModule::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing context!";
  }
}

void ContextEntities::Clear() {
  entities_.clear();
}

void ContextEntities::Add(int entity_id, int user_id, int type, int flags,
                          const float position[3], const char* classname) {
  entities_.emplace_back();
  Entity& entity = entities_.back();
  entity.entity_id = entity_id;
  entity.user_id = user_id;
  entity.type = type;
  entity.flags = flags;
  std::copy_n(position, 3, entity.position.begin());
  entity.class_name = classname;
}

}  // namespace lab
}  // namespace deepmind
