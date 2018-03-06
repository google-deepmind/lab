#ifndef DML_DEEPMIND_LUA_STACK_RESETTER_H_
#define DML_DEEPMIND_LUA_STACK_RESETTER_H_

#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {
namespace lua {

// On construction stores the current Lua stack position.
// On destruction returns Lua stack to the position it was constructed in.
//
// Example:
//
//  {
//    StackResetter stack_resetter(L);
//    PushLuaFunctionOnToStack();
//    auto result = Call(L, 0);
//    if (result.n_results() > 0) {
//      return true; // No need to call lua_pop(L, result.n_results());
//    }
//  }
class StackResetter {
 public:
  // 'L' is stored along with current stack size.
  explicit StackResetter(lua_State* L)
      : lua_state_(L), stack_size_(lua_gettop(lua_state_)) {}
  ~StackResetter() { lua_settop(lua_state_, stack_size_); }

  StackResetter& operator=(const StackResetter&) = delete;
  StackResetter(const StackResetter&) = delete;

 private:
  lua_State* lua_state_;
  int stack_size_;
};

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_STACK_RESETTER_H_
