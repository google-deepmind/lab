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

#ifndef DML_DEEPMIND_LUA_CLASS_H_
#define DML_DEEPMIND_LUA_CLASS_H_

#include <new>
#include <utility>

#include "absl/log/check.h"
#include "absl/strings/str_cat.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {
namespace lua {

// Helper class template to expose C++ classes to Lua. To expose a C++ class X
// to Lua, X should derive from lua::Class<X> and provide a name for the
// metatable which Lua uses to identify this class:
//
//   static const char* ClassName() {
//     return "deepmind.lab.UniqueClassName";
//   }
//
// Classes deriving from a specialization of Class shall be final.
//
// Member functions, as well as static factory functions, need to be registered
// with a running Lua VM separately. Non-static member functions need to have
// type "NResultsOr(lua_State*)", and for a given member function f, the
// function Member<&X::f> should be passed to the static Register function for
// registration with the Lua VM. When called from Lua, the bound function first
// attempts to load the class instance from the Lua stack, checking its
// metatable name, and then invokes the corresponding member function on it (but
// see below for ways to customize this behaviour).
//
// Example:
//
//   class X : public lua::Class<X> {
//     static const char* ClassName() { return "example.X"; }
//
//     // Optional validation.
//     static X* ReadObject(lua_State* L, int idx) {
//         X* x = Class::ReadObject(L, idx);
//         return (x != nullptr && x->IsValid())? x : nullptr;
//     }
//
//     static Register(lua_State* L) {
//       const lua::Class::Reg methods[] = {"foo", Member<&X::f>};
//       lua::Class::Register(L, methods);
//     }
//
//     LuaNResultsOr f(lua_State* L);
//     bool IsValid();
//   };
//
// The host code should call "X::Register(L);" to register the class and its
// member functions. It should also push an object of type X to the Lua stack,
// or register a factory function that can be used from Lua to create instances.
// Given an instace "x" in Lua, the code "x:foo(a, b, c)" calls the instance's
// member function "f", and the arguments are available on the Lua stack. The
// function should pop off the arguments, place its results on the Lua stack,
// and return the number of results, or an error.
//
// The optional ReadObject function can be defined in X to add further checking:
// The ReadObject function in the base class checks whether the given stack
// position contains userdata whose metadata matches X::ClassName(). If X
// provides its own version of ReadObject, it may call the base version first to
// obtain the validated class instance, but it may then perform further checking
// and return null if the object is deemed "invalid" in some higher sense. Use
// this feature with great care.
template <typename T>
class Class {
 public:
  // Allocates memory for and creates a new instance of this class, forwarding
  // arguments to the constructor. The newly allocated instance will be on top
  // of the Lua stack, and a pointer to the class is returned from this
  // function.
  template <typename... Args>
  static inline T* CreateObject(lua_State* L, Args&&... args);

  struct Reg {
    const char* first;
    lua_CFunction second;
  };

  template <typename Regs>
  static inline void Register(lua_State* L, const Regs& members);

  // Attempts to read a T* from the Lua stack. The stack needs to contain
  // userdata at the given position, and the name of the userdata's metadata has
  // to be T::ClassName().
  //
  // See above for a discussion on optionally providing a function of the same
  // name and signature in the class T itself.
  static T* ReadObject(lua_State* L, int idx) {
    return ReadUDT<T>(L, idx, T::ClassName());
  }

  // Portability note. This has not got C-language linkage but seems to work.
  template <NResultsOr (T::*Function)(lua_State*)>
  static inline int Member(lua_State* L);

  // Matches lua::Read signature for reading as part of larger structures. Must
  // be called without the 'lua' namespace as it must use argument dependent
  // lookup.
  friend ReadResult Read(lua_State* L, int idx, T** out) {
    if (lua_isnoneornil(L, idx)) {
      return ReadNotFound();
    }
    auto* result = lua::Class<T>::ReadObject(L, idx);
    if (result != nullptr) {
      *out = result;
      return ReadFound();
    } else {
      return ReadTypeMismatch();
    }
  }

 private:
  // Destroys this class by calling the destructor, invoked from the Lua "__gc"
  // method.
  static int Destroy(lua_State* L) {
    // Note that we do not call T::ReadObject here.
    if (T* t = ReadObject(L, 1)) {
      t->~T();
    }
    return 0;
  }
};

template <typename T>
template <typename... Args>
T* Class<T>::CreateObject(lua_State* L, Args&&... args) {
  void* luaNodeMemory = lua_newuserdata(L, sizeof(T));
  luaL_getmetatable(L, T::ClassName());
  CHECK(!lua_isnil(L, -1)) << T::ClassName() << " has not been registered.";
  lua_setmetatable(L, -2);
  return ::new (luaNodeMemory) T(std::forward<Args>(args)...);
}

template <typename T>
template <typename Regs>
void Class<T>::Register(lua_State* L, const Regs& members) {
  luaL_newmetatable(L, T::ClassName());

  // Push __index function pointing at self.
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, &Class::Destroy);
  lua_setfield(L, -2, "__gc");

  for (const auto& member : members) {
    Push(L, member.first);
    // Place method name in up-value to aid diagnostics.
    lua_pushvalue(L, -1);
    lua_pushcclosure(L, member.second, 1);
    lua_settable(L, -3);
  }

  lua_pop(L, 1);
}

// This is the Class member version of Member (lua/bind.h).
template <typename T>
template <NResultsOr (T::*Function)(lua_State*)>
int Class<T>::Member(lua_State* L) {
  {
    if (T* t = T::ReadObject(L, 1)) {
      // Allow T to provide a custom implementation of ReadObject.
      auto result_or = (*t.*Function)(L);
      if (result_or.ok()) {
        return result_or.n_results();
      } else {
        Push(L, absl::StrCat("[", T::ClassName(), ".",
                             lua::ToString(L, lua_upvalueindex(1)), "] - ",
                             result_or.error()));
      }
    } else if (ReadObject(L, 1) != nullptr) {
      // Fall back to LuaClass::ReadObject to check if there is an object of
      // type T on the stack at all. If yes, then the object failed to pass the
      // user-defined validation, and we can produce an appropriate diagnostic.
      Push(L, absl::StrCat("Trying to access invalidated object of type: '",
                           T::ClassName(), "' with method '",
                           lua::ToString(L, lua_upvalueindex(1)), "'."));
    } else {
      Push(L, absl::StrCat("First argument must be an object of type: '",
                           T::ClassName(),
                           "'\nDid you forget to use ':' when calling '",
                           lua::ToString(L, lua_upvalueindex(1)), "'?\n",
                           "Argument received: '", lua::ToString(L, 1), "'"));
    }
  }
  // "lua_error" performs a longjmp, which is not allowed in C++ except in
  // specific situations. We take care that no objects with non-trivial
  // destructors exist when lua_error is called.
  return lua_error(L);
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_CLASS_H_
