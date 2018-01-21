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

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_ENTITIES_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_ENTITIES_H_

#include <array>
#include <vector>

#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {

// Receive calls from lua_script.
class ContextEntities {
 public:
  struct Entity {
    int entity_id;
    int user_id;
    int type;
    int flags;
    std::array<float, 3> position;
    std::string class_name;
  };

  // Returns an entity module. A pointer to ContextEntities must exist in the up
  // value. [0, 1, e]
  static lua::NResultsOr Module(lua_State* L);

  // Clears all entities and players. (Called at start of entity update.)
  void Clear();

  // Called on each entity every frame.
  void Add(int entity_id, int user_id, int type, int flags,
           const float position[3], const char* classname);

  const std::vector<Entity>& Entities() const { return entities_; }

 private:
  std::vector<Entity> entities_;  // Entities active this frame.
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_ENTITIES_H_
