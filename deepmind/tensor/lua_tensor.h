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

#ifndef DML_DEEPMIND_TENSOR_LUA_TENSOR_H_
#define DML_DEEPMIND_TENSOR_LUA_TENSOR_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/tensor_view.h"

namespace deepmind {
namespace lab {
namespace tensor {

// Registers all LuaTensor classes.
// Must be called before using LuaTensors.
// [0, 0, -]
void LuaTensorRegister(lua_State* L);

// Returns a table of LuaTensor constructors.
// [1, 0, -]
int LuaTensorConstructors(lua_State* L);

// This type describes the validity of a unit of shared storage.
class StorageValidity {
 public:
  enum Tag {
    kInvalid,     // The tensor_view_ is valid now but may become invalid in the
                  // future.
    kValid,       // The tensor_view_ is invalid.
    kOwnsStorage  // The tensor_view_ is valid and always will be.
  };
  explicit StorageValidity(Tag tag = kValid) : tag_(tag) {}

  // It is guaranteed that if the storage is owned that it cannot be
  // invalidated.
  void Invalidate() {
    assert(tag_ == kValid);
    tag_ = kInvalid;
  }
  bool IsValid() { return tag_ != kInvalid; }
  bool OwnsStorage() { return tag_ == kOwnsStorage; }

 private:
  Tag tag_;
};

template <typename T>
class StorageVector : public StorageValidity {
 public:
  template <typename... Args>
  StorageVector(Args&&... args)
      : StorageValidity(kOwnsStorage), data_(std::forward<Args>(args)...) {}
  std::vector<T>* mutable_data() { return &data_; }

 private:
  std::vector<T> data_;
};

// Lua bindings for a TensorView<T>.
// See tensor.md for details on how to use this from Lua.
// The storage within tensor_view has a life time according to the
// StorageValidity.
template <typename T>
class LuaTensor : public lua::Class<LuaTensor<T>> {
  using Class = typename lua::Class<LuaTensor<T>>;
  using View = TensorView<T>;
  friend Class;
  static const char* ClassName();

 public:
  // Creates a new LuaTensor with given tensor_view.
  // 'storage_validity' is shared with any future creation of TensorViews which
  // contain the same TensorView storage pointer as 'tensor_view'.
  // 'storage_validity' must be marked invalid if tensor_view's storage
  // is destroyed before all instances of LuaTensor are.
  // A LuaTensor can own the data if the StorageValidity also holds the
  // data pointed to by tensor_view. This can be done using a StorageVector.
  LuaTensor(View tensor_view, std::shared_ptr<StorageValidity> storage_validity)
      : tensor_view_(std::move(tensor_view)),
        storage_validity_(std::move(storage_validity)) {}

  // Create an owning LuaTensor from shape and storage.
  LuaTensor(std::vector<std::size_t> shape, std::vector<T> storage)
      : tensor_view_(Layout(std::move(shape)), storage.data()),
        storage_validity_(
            std::make_shared<StorageVector<T>>(std::move(storage))) {}

  static void Register(lua_State* L) {
    const typename Class::Reg methods[] = {
        {"type", &Class::template Member<&LuaTensor<T>::Type>},
        {"ownsStorage", &Class::template Member<&LuaTensor<T>::OwnsStorage>},
        {"__tostring", &Class::template Member<&LuaTensor<T>::ToString>},
        {"__call", &Class::template Member<&LuaTensor<T>::Index>},
        {"__eq", &Class::template Member<&LuaTensor<T>::Equal>},
        {"shape", &Class::template Member<&LuaTensor<T>::Shape>},
        {"clone", &Class::template Member<&LuaTensor<T>::Clone>},
        {"val", &Class::template Member<&LuaTensor<T>::Val>},
        {"transpose", &Class::template Member<&LuaTensor<T>::Transpose>},
        {"select", &Class::template Member<&LuaTensor<T>::Select>},
        {"narrow", &Class::template Member<&LuaTensor<T>::Narrow>},
        {"apply", &Class::template Member<&LuaTensor<T>::Apply>},
        {"applyIndexed", &Class::template Member<&LuaTensor<T>::ApplyIndexed>},
        {"fill",
         &Class::template Member<&LuaTensor<T>::ScalarOp<&View::Assign>>},
        {"mul", &Class::template Member<&LuaTensor<T>::ScalarOp<&View::Mul>>},
        {"add", &Class::template Member<&LuaTensor<T>::ScalarOp<&View::Add>>},
        {"div", &Class::template Member<&LuaTensor<T>::ScalarOp<&View::Div>>},
        {"sub", &Class::template Member<&LuaTensor<T>::ScalarOp<&View::Sub>>},
        {"copy",
         &Class::template Member<&LuaTensor<T>::ViewOp<&View::CAssign>>},
        {"cmul", &Class::template Member<&LuaTensor<T>::ViewOp<&View::CMul>>},
        {"cadd", &Class::template Member<&LuaTensor<T>::ViewOp<&View::CAdd>>},
        {"cdiv", &Class::template Member<&LuaTensor<T>::ViewOp<&View::CDiv>>},
        {"csub", &Class::template Member<&LuaTensor<T>::ViewOp<&View::CSub>>},
        {"byte", &Class::template Member<&LuaTensor<T>::Convert<uint8_t>>},
        {"char", &Class::template Member<&LuaTensor<T>::Convert<int8_t>>},
        {"int16", &Class::template Member<&LuaTensor<T>::Convert<int16_t>>},
        {"int32", &Class::template Member<&LuaTensor<T>::Convert<int32_t>>},
        {"int64", &Class::template Member<&LuaTensor<T>::Convert<int64_t>>},
        {"float", &Class::template Member<&LuaTensor<T>::Convert<float>>},
        {"double", &Class::template Member<&LuaTensor<T>::Convert<double>>}};
    Class::Register(L, methods);
  }

  // Reads a table according to shape into *values.
  // Returns whether the table's values match the shape.
  // Used by Create to read the values from a table into *values.
  // Returns whether all values are the correct type and match the given shape.
  // ['shape_begin', 'shape_end') must form a valid range.
  static bool ReadTable(lua::TableRef table,
                        std::vector<std::size_t>::const_iterator shape_begin,
                        std::vector<std::size_t>::const_iterator shape_end,
                        std::vector<T>* values) {
    if (shape_begin == shape_end) return false;
    if (shape_begin + 1 == shape_end) {
      for (std::size_t i = 0; i < *shape_begin; ++i) {
        values->emplace_back();
        if (!table.LookUp(i + 1, &values->back())) {
          return false;
        }
      }
      return true;
    } else {
      lua::TableRef subtable;
      for (std::size_t i = 0; i < *shape_begin; ++i) {
        if (!table.LookUp(i + 1, &subtable) ||
            !ReadTable(std::move(subtable), shape_begin + 1, shape_end,
                       values)) {
          return false;
        }
      }
      return true;
    }
  }

  // Used by 'Create' to find the shape of a given Lua table.
  // Returns whether shape could be implied from the table.
  static bool ReadTableShape(const lua::TableRef& table,
                             std::vector<std::size_t>* shape) {
    auto table_size = table.ArraySize();
    if (shape->size() == 20 || table_size == 0) {
      shape->clear();
      return false;
    }
    shape->push_back(table_size);
    lua::TableRef next;
    if (table.LookUp(1, &next)) {
      return ReadTableShape(std::move(next), shape);
    }
    return true;
  }

  // Creates a LuaTensor<T> and returns it on the stack.
  // If called with value arguments, Tensor(s1, s2, s3, ...),
  // it will create a zeroed tensor of shape (s1, s2, s3, ...).
  // If called with a Lua array, Tensor{{v1, v2}, {v3, v4}, ...},
  // it will create a tensor matching the shape of the tables passed in.
  // Fails if the shape is inconsistent or contains values that cannot be read.
  // [1, (n|1), e]
  static lua::NResultsOr Create(lua_State* L) {
    int top = lua_gettop(L);
    lua::TableRef table;
    if (lua::Read(L, 1, &table)) {
      std::vector<std::size_t> shape;
      std::vector<T> storage;
      if (table.ArraySize() == 0) {
        LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
        return 1;
      }
      if (ReadTableShape(table, &shape)) {
        storage.reserve(Layout::num_elements(shape));
        if (ReadTable(std::move(table), shape.begin(), shape.end(), &storage)) {
          LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
          return 1;
        }
      }
      return "[Tensor.CreateFromTable] Failed to read table in to Tensor.";
    } else {
      std::vector<std::size_t> shape;
      shape.reserve(top);
      for (int i = 0; i < top; ++i) {
        int dim;
        if (lua::Read(L, i + 1, &dim) && dim > 0) {
          shape.push_back(dim);
        } else {
          return "[Tensor.CreateFromShape] Failed to read Tensor shape.";
        }
      }
      std::vector<T> storage(Layout::num_elements(shape));
      LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
      return 1;
    }
  }

  lua::NResultsOr Index(lua_State* L) {
    int top = lua_gettop(L);

    View result = tensor_view_;
    for (int i = 2; i <= top; ++i) {
      int index;
      if (!lua::Read(L, i, &index) || index < 1 ||
          !result.Select(0, index - 1)) {
        return "[Tensor.Index] Invalid Index!";
      }
    }
    LuaTensor::CreateObject(L, std::move(result), storage_validity_);
    return 1;
  }

  lua::NResultsOr Val(lua_State* L) {
    if (tensor_view_.shape().size() == 1 && tensor_view_.shape().front() == 1) {
      T& val = tensor_view_.mutable_storage()[tensor_view_.start_offset()];
      if (lua_gettop(L) == 2) {
        if (!lua::Read(L, 2, &val)) {
          return "[Tensor.Val] failed to assign value.";
        }
      }
      lua::Push(L, val);
      return 1;
    } else {
      return "[Tensor.Val] 'val' can only be called on an element";
    }
  }

  // [1, 3, e]
  lua::NResultsOr Select(lua_State* L) {
    std::size_t dim;
    std::size_t index;
    auto result = tensor_view_;
    if (lua::Read(L, 2, &dim) && lua::Read(L, 3, &index) &&
        result.Select(dim - 1, index - 1)) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    }
    return "[Tensor.Select] Must contain 1 based dim, index, received: " +
           lua::ToString(L, 2) + ", " + lua::ToString(L, 3);
  }

  // [1, 3, e]
  lua::NResultsOr Narrow(lua_State* L) {
    std::size_t dim;
    std::size_t index;
    std::size_t size;
    auto result = tensor_view_;
    if (lua::Read(L, 2, &dim) && lua::Read(L, 3, &index) &&
        lua::Read(L, 4, &size) && result.Narrow(dim - 1, index - 1, size)) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    }
    return "[Tensor.Narrow] Must contain 1 based dim, index, size "
           "recieved: " +
           lua::ToString(L, 2) + ", " + lua::ToString(L, 3) + ", " +
           lua::ToString(L, 4);
  }

  // [1, 2, e]
  lua::NResultsOr ApplyIndexed(lua_State* L) {
    lua::NResultsOr err = 0;
    tensor_view_.ForEachIndexedMutable(
        [L, &err](const std::vector<std::size_t>& index, T* value) {
          lua_pushvalue(L, 2);
          lua::Push(L, *value);
          // Convert index to 1 based.
          lua_createtable(L, index.size(), 0);
          for (std::size_t i = 0; i < index.size(); ++i) {
            lua::Push(L, i + 1);
            lua::Push(L, index[i] + 1);
            lua_settable(L, -3);
          }
          auto result = lua::Call(L, 2);
          bool keep_going = true;
          if (result.ok()) {
            if (result.n_results() > 0) {
              lua::Read(L, -result.n_results(), value);
            }
            if (result.n_results() > 1) {
              lua::Read(L, -result.n_results() + 1, &keep_going);
            }
          } else {
            err = std::move(result);
            return false;
          }
          lua_pop(L, result.n_results());
          return keep_going;
        });
    if (!err.ok()) {
      lua_pop(L, err.n_results());
      return err;
    }
    lua_pop(L, lua_gettop(L) - 1);
    return 1;
  }

  // [1, 2, e]
  lua::NResultsOr Apply(lua_State* L) {
    lua::NResultsOr err = 0;
    tensor_view_.ForEachMutable([L, &err](T* value) {
      lua_pushvalue(L, 2);
      lua::Push(L, *value);
      auto result = lua::Call(L, 1);
      bool keep_going = true;
      if (result.ok()) {
        if (result.n_results() > 0) {
          lua::Read(L, -result.n_results(), value);
        }
        if (result.n_results() > 1) {
          lua::Read(L, -result.n_results() + 1, &keep_going);
        }
      } else {
        err = std::move(result);
        return false;
      }
      lua_pop(L, result.n_results());
      return keep_going;
    });
    if (!err.ok()) {
      lua_pop(L, err.n_results());
      return err;
    }
    lua_pop(L, lua_gettop(L) - 1);
    return 1;
  }

  // [1, 1, e]
  template <typename U>
  lua::NResultsOr Convert(lua_State* L) {
    std::vector<U> storage;
    storage.reserve(tensor_view_.num_elements());
    tensor_view_.ForEach([&storage](T value) {
      storage.push_back(static_cast<U>(value));
      return true;
    });
    LuaTensor<U>::CreateObject(L, tensor_view_.shape(), std::move(storage));
    return 1;
  }

  // [1, 0, -]
  lua::NResultsOr Type(lua_State* L) {
    lua::Push(L, ClassName());
    return 1;
  }

  // [1, 0, -]
  lua::NResultsOr Shape(lua_State* L) {
    lua::Push(L, tensor_view_.shape());
    return 1;
  }

  // [1, 0, -]
  lua::NResultsOr Clone(lua_State* L) {
    std::vector<T> storage;
    storage.reserve(tensor_view_.num_elements());
    tensor_view_.ForEach([&storage](T val) {
      storage.push_back(val);
      return true;
    });
    LuaTensor::CreateObject(L, tensor_view_.shape(), std::move(storage));
    return 1;
  }

  // [1, 1, -]
  lua::NResultsOr Equal(lua_State* L) {
    bool result = false;
    // Equal self?
    if (lua_rawequal(L, 1, 2)) {
      result = true;
      // Is same type.
    } else if (LuaTensor* rhs = LuaTensor::ReadObject(L, 2)) {
      // Contains same values.
      result = tensor_view() == rhs->tensor_view();
    } else {
      result = false;
    }
    lua::Push(L, result);
    return 1;
  }

  // Returns self on to the stack.
  // [1, 1, e]
  template <void (View::*Op)(double)>
  lua::NResultsOr ScalarOp(lua_State* L) {
    double value;
    if (lua::Read(L, 2, &value)) {
      (tensor_view_.*Op)(value);
      lua_pop(L, lua_gettop(L) - 1);
      return 1;
    }
    return "[Tensor.ScalerOp] Must call with number, recieved: " +
           lua::ToString(L, 2);
  }

  // Returns self on to the stack.
  // [1, 1, e]
  template <bool (View::*Op)(const View&)>
  lua::NResultsOr ViewOp(lua_State* L) {
    if (LuaTensor* rhs = LuaTensor::ReadObject(L, 2)) {
      if ((tensor_view_.*Op)(rhs->tensor_view_)) {
        lua_pop(L, lua_gettop(L) - 1);
        return 1;
      }
    }
    return "[Tensor.ViewOp] Must call with same sized tensor, recieved: " +
           lua::ToString(L, 2);
  }

  // Returns self on to the stack.
  // [1, 2, e]
  lua::NResultsOr Transpose(lua_State* L) {
    std::size_t index_from;
    std::size_t index_to;
    auto result = tensor_view_;
    if (lua::Read(L, 2, &index_from) && lua::Read(L, 3, &index_to) &&
        result.Transpose(index_from - 1, index_to - 1)) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    }
    return "[Tensor.Transpose] Must contain 1 based indexes, recieved: " +
           lua::ToString(L, 2) + ", " + lua::ToString(L, 3);
  }

  lua::NResultsOr ToString(lua_State* L) {
    std::ostringstream ss;
    ss << "[" << ClassName() << "]\n";
    ss << tensor_view_;
    lua::Push(L, ss.str());
    return 1;
  }

  static LuaTensor* ReadObject(lua_State* L, int idx) {
    LuaTensor* tensor = Class::ReadObject(L, idx);
    if (tensor && tensor->IsValid()) {
      return tensor;
    }
    return nullptr;
  }

  lua::NResultsOr OwnsStorage(lua_State* L) {
    lua::Push(L, OwnsStorage());
    return 1;
  }

  bool IsValid() { return storage_validity_->IsValid(); }
  bool OwnsStorage() { return storage_validity_->OwnsStorage(); }

  const View& tensor_view() const { return tensor_view_; }
  View* mutable_tensor_view() { return &tensor_view_; }

 private:
  // This is a tensor view.
  View tensor_view_;

  // 'storage_validity_' tracks the validity of the tensor_view_'s storage.
  // See StorageValidity for more details.
  std::shared_ptr<StorageValidity> storage_validity_;
};

template <>
inline const char* LuaTensor<std::uint8_t>::ClassName() {
  return "deepmind.lab.tensor.ByteTensor";
}

template <>
inline const char* LuaTensor<std::int8_t>::ClassName() {
  return "deepmind.lab.tensor.CharTensor";
}

template <>
inline const char* LuaTensor<std::int16_t>::ClassName() {
  return "deepmind.lab.tensor.Int16Tensor";
}

template <>
inline const char* LuaTensor<std::int32_t>::ClassName() {
  return "deepmind.lab.tensor.Int32Tensor";
}

template <>
inline const char* LuaTensor<std::int64_t>::ClassName() {
  return "deepmind.lab.tensor.Int64Tensor";
}

template <>
inline const char* LuaTensor<float>::ClassName() {
  return "deepmind.lab.tensor.FloatTensor";
}

template <>
inline const char* LuaTensor<double>::ClassName() {
  return "deepmind.lab.tensor.DoubleTensor";
}

}  // namespace tensor
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_TENSOR_LUA_TENSOR_H_
