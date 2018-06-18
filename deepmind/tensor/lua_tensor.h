// Copyright (C) 2016-2019 Google Inc.
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
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "deepmind/engine/lua_random.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/tensor_view.h"
#include "deepmind/util/file_reader.h"
#include "public/file_reader_types.h"

namespace deepmind {
namespace lab {
namespace tensor {

// Registers all LuaTensor classes.
// Must be called before using LuaTensors.
// [0, 0, -]
void LuaTensorRegister(lua_State* L);

// Returns a table of LuaTensor constructors.
// Must be called with Lua upvalue pointing to a DeepMindReadOnlyFileSystem.
// [1, 0, -]
int LuaTensorConstructors(lua_State* L);

// This type describes the validity of a unit of shared storage.
class StorageValidity {
 public:
  enum Tag {
    kInvalid,     // The tensor_view_ is invalid.
    kValid,       // The tensor_view_ is valid now but may become invalid in the
                  // future.
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
  LuaTensor(ShapeVector shape, std::vector<T> storage)
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
        {"tostring", &Class::template Member<&LuaTensor<T>::ToString>},
        {"size", &Class::template Member<&LuaTensor<T>::Size>},
        {"shape", &Class::template Member<&LuaTensor<T>::Shape>},
        {"reshape", &Class::template Member<&LuaTensor<T>::Reshape>},
        {"clone", &Class::template Member<&LuaTensor<T>::Clone>},
        {"sum", &Class::template Member<&LuaTensor<T>::Sum>},
        {"product", &Class::template Member<&LuaTensor<T>::Product>},
        {"lengthSquared",
         &Class::template Member<&LuaTensor<T>::LengthSquared>},
        {"dot", &Class::template Member<&LuaTensor<T>::DotProduct>},
        {"val", &Class::template Member<&LuaTensor<T>::Val>},
        {"transpose", &Class::template Member<&LuaTensor<T>::Transpose>},
        {"select", &Class::template Member<&LuaTensor<T>::Select>},
        {"narrow", &Class::template Member<&LuaTensor<T>::Narrow>},
        {"reverse", &Class::template Member<&LuaTensor<T>::Reverse>},
        {"apply", &Class::template Member<&LuaTensor<T>::Apply>},
        {"applyIndexed", &Class::template Member<&LuaTensor<T>::ApplyIndexed>},
        {"clamp", &Class::template Member<&LuaTensor<T>::Clamp>},
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
        {"mmul", &Class::template Member<&LuaTensor<T>::MMul>},
        {"floor",
         &Class::template Member<&LuaTensor<T>::UnaryOp<&View::Floor>>},
        {"ceil", &Class::template Member<&LuaTensor<T>::UnaryOp<&View::Ceil>>},
        {"round",
         &Class::template Member<&LuaTensor<T>::UnaryOp<&View::Round>>},
        {"shuffle", &Class::template Member<&LuaTensor<T>::Shuffle>},
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
  static bool ReadTable(const lua::TableRef& table,
                        ShapeVector::const_iterator shape_begin,
                        ShapeVector::const_iterator shape_end,
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
            !ReadTable(subtable, shape_begin + 1, shape_end, values)) {
          return false;
        }
      }
      return true;
    }
  }

  // Used by 'Create' to find the shape of a given Lua table.
  // Returns whether shape could be implied from the table.
  static bool ReadTableShape(const lua::TableRef& table, ShapeVector* shape) {
    auto table_size = table.ArraySize();
    if (shape->size() == 20 || table_size == 0) {
      shape->clear();
      return false;
    }
    shape->push_back(table_size);
    lua::TableRef next;
    if (table.LookUp(1, &next)) {
      return ReadTableShape(next, shape);
    }
    return true;
  }

  // Used by 'CreateFromRange' to construct a range from a given Lua table, in
  // one of these 3 forms:
  // - '{lower_bound, upper_bound, stride}'
  // - '{lower_bound, upper_bound}', where the stride is assumed to be 1
  // - '{upper_bound}', where the lower bound is also assumed to be 1
  // The range bounds and stride are output through the corresponing pointer
  // parameters.
  // Returns whether range could be implied from the table.
  static bool ReadTableRange(const lua::TableRef& table, T* lower_bound,
                             T* upper_bound, T* stride) {
    std::size_t i = 1;
    *lower_bound = 1;
    *stride = 1;
    switch (table.ArraySize()) {
      default:
        return false;
      case 3:
        if (!table.LookUp(3, stride)) return false;
        // fallthrough
      case 2:
        if (!table.LookUp(i++, lower_bound)) return false;
        // fallthrough
      case 1:
        if (!table.LookUp(i, upper_bound)) return false;
    }
    return true;
  }

  // Creates a tensor from the given hierarchy of tables and values. The shape
  // is implied.
  static lua::NResultsOr CreateFromTableValues(lua_State* L,
                                               const lua::TableRef& table) {
    ShapeVector shape;
    std::vector<T> storage;
    if (ReadTableShape(table, &shape)) {
      storage.reserve(Layout::num_elements(shape));
      if (ReadTable(table, shape.begin(), shape.end(), &storage)) {
        LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
        return 1;
      }
    }
    return "[Tensor.CreateFromTableValues] Failed to read table in to Tensor.";
  }

  // Creates a rank-1 tensor. Ranges can be declared three ways:
  //
  // *   {'from', 'to', 'step'}
  // *   {'from', 'to'} -- Equivalent to { from, to, 1}
  // *   {'to'} -- Equivalent to {1, to, 1}
  //
  // The number of elements will be floor(('to' - 'from') / 'step') + 1.
  // Elements are generated by starting with 'from' and advancing by 'step' for
  // each subsequent value. If 'step' is zero or the number of elements
  // generated is not positive then an error is thrown.
  //
  // Examples:
  // {5} -> generates {1, 2, 3, 4, 5}
  // {3, 7} -> generates {3, 4, 5, 6, 7}
  // {1, 2, 0.5} -> generates {1, 1.5, 2}
  // {7, 3, -1} -> generates {7, 6, 5, 4, 3}
  // {3, 9, 3} -> generates {3, 6, 9}
  // {3, 14, 3} -> generates {3, 6, 9, 12}
  static lua::NResultsOr CreateFromRange(lua_State* L,
                                         const lua::TableRef& range) {
    ShapeVector shape;
    std::vector<T> storage;
    T lower_bound, upper_bound, stride;
    if (!ReadTableRange(range, &lower_bound, &upper_bound, &stride)) {
      return "[Tensor.CreateFromRange] Failed to read Tensor range.";
    }
    if (stride == 0) {
      return "[Tensor.CreateFromRange] Step size must not be zero.";
    }
    std::ptrdiff_t n = std::floor((upper_bound - lower_bound) / stride);
    if (n < 0) {
      return "[Tensor.CreateFromRange] Invalid Tensor range.";
    }
    shape.push_back(n + 1);
    storage.reserve(n + 1);
    std::generate_n(std::back_inserter(storage), n + 1,
                    [&lower_bound, stride]() {
                      auto prev = lower_bound;
                      lower_bound += stride;
                      return prev;
                    });
    LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
    return 1;
  }

  // Creates a rank-N tensor with extents dim1, dim2, ..., dimN. We refer to the
  // tuple (dim1, dim2, ..., dimN) as the shape of the tensor. N is the number
  // of integer arguments passed to the tensor.
  static lua::NResultsOr CreateFromArgs(lua_State* L) {
    int top = lua_gettop(L);
    ShapeVector shape;
    shape.reserve(top);
    for (int i = 0; i < top; ++i) {
      int dim;
      if (lua::Read(L, i + 1, &dim) && dim > 0) {
        shape.push_back(dim);
      } else {
        return "[Tensor.CreateFromArgs] Failed to read Tensor shape.";
      }
    }
    std::vector<T> storage(Layout::num_elements(shape));
    LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
    return 1;
  }

  // Creates a rank-1 tensor by reading bytes within a file. The file is read in
  // system local endian order.
  //
  // Keyword Args:
  //
  // *   'name': Name of the file to read. Must be a valid filename.
  // *   'byteOffset': Offset from the beginning of the file at which reading
  //     starts. Optional, default 0.
  // *   'numElements': Number of elements to read starting at the offset.
  //     Optional, defaults to largest number of elements that is available from
  //     the given offset.
  //
  // If the offset is outside of the range [0, file size] or if reading count
  // values plus the offset would exceed the size of the file an error is
  // thrown.
  static lua::NResultsOr CreateFromFile(lua_State* L, lua::TableRef file_args) {
    const DeepMindReadOnlyFileSystem* fs = nullptr;
    if (IsTypeMismatch(lua::Read(L, lua_upvalueindex(1), &fs))) {
      return "[Tensor.CreateFromFile] Invalid filesystem in upvalue";
    }
    if (fs == nullptr) {
      return "[Tensor.CreateFromFile] Missing filesystem in upvalue";
    }
    ShapeVector shape;
    std::vector<T> storage;
    std::size_t offset = 0;
    std::string name;
    if (!file_args.LookUp("name", &name)) {
      return "[Tensor.CreateFromFile] Field 'name' must exist and be a string.";
    }

    if (IsTypeMismatch(file_args.LookUp("byteOffset", &offset))) {
      return "[Tensor.CreateFromFile] 'byteOffset' must be a non-negative "
             "integral value.";
    }

    util::FileReader ifs(fs, name.c_str());

    if (!ifs.Success()) {
      return absl::StrCat(
          "[Tensor.CreateFromFile] Failed to open file, name: ", name);
    }

    std::size_t file_size;
    if (!ifs.GetSize(&file_size)) {
      return absl::StrCat(
          "[Tensor.CreateFromFile] Failed to read file, name: ", name);
    }

    if (offset > file_size) {
      return absl::StrCat(
          "[Tensor.CreateFromFile] Must supply 'byteOffset' "
          "within file size, name: ",
          name, ", offset: ", offset, ", file size: ", file_size);
    }

    const std::size_t max_num_elements = (file_size - offset) / sizeof(T);
    std::size_t num_elements = max_num_elements;

    auto parse_result = file_args.LookUp("numElements", &num_elements);
    if (IsTypeMismatch(parse_result)) {
      return absl::StrCat("[Tensor.CreateFromFile] 'numElements' must be a "
                          "non-negative integral value.");
    } else if (IsFound(parse_result) && num_elements > max_num_elements) {
      return absl::StrCat(
          "[Tensor.CreateFromFile] Attempted to read past end of file, name: ",
          name, ", numElements: ", num_elements, ", max numElements: ",
          max_num_elements, ", offset: ", offset, ", file size: ", file_size);
    }

    storage.resize(num_elements);
    if (!ifs.Read(offset, sizeof(T) * num_elements,
                  reinterpret_cast<char*>(storage.data()))) {
      return absl::StrCat(
          "[Tensor.CreateFromFile] Failed to read file, name: ", name);
    }
    shape.push_back(num_elements);
    LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
    return 1;
  }

  // Creates a LuaTensor<T> and returns it on the stack.
  // If called with value arguments, Tensor(s1, s2, s3, ...),
  // it will create a zeroed tensor of shape (s1, s2, s3, ...).
  // If called with a Lua array, Tensor{{v1, v2}, {v3, v4}, ...},
  // it will create a tensor matching the shape of the tables passed in.
  // If called with a range parameter, Tensor{range = {s1, ...}}
  // it will create a rank-1 tensor with the values in the range.
  // If called with a file parameter, Tensor{file = {name=<filename>, ...}}
  // it will create a rank-1 tensor with the values read from the file.
  // Fails if the shape is inconsistent or contains values that cannot be read.
  static lua::NResultsOr Create(lua_State* L) {
    lua::TableRef table;
    if (lua::Read(L, 1, &table)) {
      if (lua_gettop(L) != 1) {
        return "[Tensor.Create] 'Must only pass one argument for table "
               "construction.";
      }
      auto keys = table.Keys<std::string>();
      if (keys.empty()) {
        if (table.ArraySize() == 0) {
          LuaTensor::CreateObject(L, ShapeVector{}, std::vector<T>{});
          return 1;
        } else {
          return CreateFromTableValues(L, table);
        }
      } else if (keys.size() == 1) {
        if (keys.front() == "range") {
          lua::TableRef range;
          if (table.LookUp("range", &range)) {
            return CreateFromRange(L, range);
          } else {
            return "[Tensor.Create] 'range' must contain a table.";
          }
        } else if (keys.front() == "file") {
          lua::TableRef file_args;
          if (table.LookUp("file", &file_args)) {
            return CreateFromFile(L, file_args);
          } else {
            return "[Tensor.Create] 'file' must contain a table.";
          }
        } else {
          return "[Tensor.Create] Named constructor must be 'range' or 'file'";
        }
      } else {
        return "[Tensor.Create] Must supply only one named contructor.";
      }
    } else {
      return CreateFromArgs(L);
    }
  }

  lua::NResultsOr Index(lua_State* L) {
    int top = lua_gettop(L);

    View result = tensor_view_;
    for (int i = 2; i <= top; ++i) {
      int index;
      if (!lua::Read(L, i, &index) || index < 1 ||
          !result.Select(0, index - 1)) {
        return "Invalid Index!";
      }
    }
    LuaTensor::CreateObject(L, std::move(result), storage_validity_);
    return 1;
  }

  static void ToLuaTable(lua_State* L, const tensor::TensorView<T>& view) {
    const auto& shape = view.shape();
    if (shape.empty()) {
      lua_createtable(L, 0, 0);
      return;
    }
    lua_createtable(L, shape.front(), 0);
    if (shape.size() == 1) {
      std::size_t i = 0;
      view.ForEach([&i, L](T value) {
        lua::Push(L, ++i);
        lua::Push(L, value);
        lua_settable(L, -3);
      });
    } else {
      for (std::size_t i = 0; i < shape[0]; ++i) {
        lua::Push(L, i + 1);
        tensor::TensorView<T> new_view = view;
        new_view.Select(0, i);
        ToLuaTable(L, new_view);
        lua_settable(L, -3);
      }
    }
  }

  lua::NResultsOr Val(lua_State* L) {
    const auto& shape = tensor_view_.shape();
    if (shape.size() == 1 && shape.front() == 1) {
      T& val = tensor_view_.mutable_storage()[tensor_view_.start_offset()];
      if (lua_gettop(L) == 2) {
        if (!lua::Read(L, 2, &val)) {
          return "Failed to assign value.";
        }
      }
      lua::Push(L, val);
      return 1;
    } else {
      if (lua_gettop(L) == 2) {
        lua::TableRef table;
        if (!lua::Read(L, 2, &table)) {
          return "Failed read table shape.";
        }
        ShapeVector table_shape;
        if (!ReadTableShape(table, &table_shape)) {
          return "Failed read table shape.";
        }
        if (shape.size() != table_shape.size() ||
            !std::equal(table_shape.begin(), table_shape.end(),
                        shape.begin())) {
          return "Shape must match tensor shape.";
        }
        std::vector<T> values;
        if (!ReadTable(table, table_shape.begin(), table_shape.end(),
                       &values)) {
          return "Failed to read values from tables";
        }
        int i = 0;
        tensor_view_.ForEachMutable([&values, &i](T* value) {
          *value = values[i];
          ++i;
        });
      }
      ToLuaTable(L, tensor_view_);
      return 1;
    }
  }

  lua::NResultsOr Select(lua_State* L) {
    std::size_t dim;
    std::size_t index;
    auto result = tensor_view_;
    if (lua::Read(L, 2, &dim) && lua::Read(L, 3, &index) &&
        result.Select(dim - 1, index - 1)) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    }
    return absl::StrCat("Must contain 1 based dim, index, received: ",
                        lua::ToString(L, 2), ", ", lua::ToString(L, 3));
  }

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
    return absl::StrCat(
        "Must contain 1 based dim, index, size received: ", lua::ToString(L, 2),
        ", ", lua::ToString(L, 3), ", ", lua::ToString(L, 4));
  }

  lua::NResultsOr Reverse(lua_State* L) {
    std::size_t dim;
    auto result = tensor_view_;
    if (lua::Read(L, 2, &dim) && result.Reverse(dim - 1)) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    }
    return absl::StrCat("Must contain 1 based dim received: ",
                        lua::ToString(L, 2));
  }

  lua::NResultsOr ApplyIndexed(lua_State* L) {
    lua::NResultsOr err = 0;
    tensor_view_.ForEachIndexedMutable(
        [L, &err](const ShapeVector& index, T* value) {
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
    lua_settop(L, 1);
    return 1;
  }

  // Clamps all values to the interval [arg1, arg2]; arg1 must not exceed arg2.
  // If either argument is not found, then clamping does not occour on that
  // side.
  lua::NResultsOr Clamp(lua_State* L) {
    T min_value = std::numeric_limits<T>::lowest(),
      max_value = std::numeric_limits<T>::max();
    if (IsTypeMismatch(lua::Read(L, 2, &min_value)) ||
        IsTypeMismatch(lua::Read(L, 3, &max_value))) {
      return "TypeMismatch Arg1 must be a nil or valid min value and Arg2 must "
             "nil or a valid max value.";
    }
    if (max_value < min_value) {
      return "Arg1 (min value) must not exceed Arg2 (max value).";
    }
    if (min_value != std::numeric_limits<T>::lowest() &&
        max_value != std::numeric_limits<T>::max()) {
      tensor_view_.ForEachMutable([min_value, max_value](T* value) {
        *value = std::max(std::min(*value, max_value), min_value);
      });
    } else if (min_value != std::numeric_limits<T>::lowest()) {
      tensor_view_.ForEachMutable([min_value](T* value) {
        *value = std::max(min_value, *value);
      });
    } else if (max_value != std::numeric_limits<T>::max()) {
      tensor_view_.ForEachMutable([max_value](T* value) {
        *value = std::min(*value, max_value);
      });
    }
    lua_settop(L, 1);
    return 1;
  }

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
    lua_settop(L, 1);
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

  lua::NResultsOr Type(lua_State* L) {
    lua::Push(L, ClassName());
    return 1;
  }

  lua::NResultsOr Size(lua_State* L) {
    lua::Push(L, tensor_view_.num_elements());
    return 1;
  }

  lua::NResultsOr Shape(lua_State* L) {
    lua::Push(L, tensor_view_.shape());
    return 1;
  }

  // Call with an array of integers. The tensor must be contiguous and the
  // number of elements in the new shape must match that of the original
  // otherwise an error is raised.
  lua::NResultsOr Reshape(lua_State* L) {
    auto result = tensor_view_;
    ShapeVector new_shape;
    if (lua::Read(L, -1, &new_shape) && result.Reshape(std::move(new_shape))) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    } else {
      return "Must be called on a contiguous tensor with a matching element "
             "count.";
    }
  }

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

  lua::NResultsOr Sum(lua_State* L) {
    lua::Push(L, tensor_view_.template Sum<double>());
    return 1;
  }

  lua::NResultsOr Product(lua_State* L) {
    lua::Push(L, tensor_view_.template Product<double>());
    return 1;
  }

  lua::NResultsOr LengthSquared(lua_State* L) {
    lua::Push(L, tensor_view_.template LengthSquared<double>());
    return 1;
  }

  lua::NResultsOr DotProduct(lua_State* L) {
    if (LuaTensor* rhs = LuaTensor::ReadObject(L, 2)) {
      double result;
      if (tensor_view_.DotProduct(rhs->tensor_view_, &result)) {
        lua_settop(L, 0);
        lua::Push(L, result);
        return 1;
      }
    }
    return absl::StrCat("Must call with same sized tensor, received: ",
                        lua::ToString(L, 2));
  }

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

  // Returns self on to the stack, after the operation is applied in-place.
  template <void (View::*Op)()>
  lua::NResultsOr UnaryOp(lua_State* L) {
    (tensor_view_.*Op)();
    return 1;
  }

  // Returns self on to the stack, after the operation is applied in-place.
  template <void (View::*Op)(double)>
  lua::NResultsOr ScalarOp(lua_State* L) {
    std::vector<double> values;
    double value;
    if (lua::Read(L, 2, &value)) {
      (tensor_view_.*Op)(value);
      lua_settop(L, 1);
      return 1;
    } else if (lua::Read(L, 2, &values)) {
      const auto& shape = tensor_view_.shape();
      if (!shape.empty() && values.size() == shape.back()) {
        for (std::size_t i = 0; i < values.size(); ++i) {
          auto new_view = tensor_view_;
          new_view.Select(shape.size() - 1, i);
          (new_view.*Op)(values[i]);
        }
        lua_settop(L, 1);
        return 1;
      }
    }
    return absl::StrCat(
        "Must call with number or an array that matches last "
        "dimension received: ",
        lua::ToString(L, 2));
  }

  // Returns self on to the stack, after the operation is applied in place.
  template <bool (View::*Op)(const View&)>
  lua::NResultsOr ViewOp(lua_State* L) {
    if (LuaTensor* rhs = LuaTensor::ReadObject(L, 2)) {
      if ((tensor_view_.*Op)(rhs->tensor_view_)) {
        lua_settop(L, 1);
        return 1;
      }
    }
    return absl::StrCat("Must call with same sized tensor, received: ",
                        lua::ToString(L, 2));
  }

  // Returns transposed tensor.
  lua::NResultsOr Transpose(lua_State* L) {
    std::size_t index_from;
    std::size_t index_to;
    auto result = tensor_view_;
    if (lua::Read(L, 2, &index_from) && lua::Read(L, 3, &index_to) &&
        result.Transpose(index_from - 1, index_to - 1)) {
      LuaTensor::CreateObject(L, std::move(result), storage_validity_);
      return 1;
    }
    return absl::StrCat("Must contain 1 based indexes, received: ",
                        lua::ToString(L, 2), ", ", lua::ToString(L, 3));
  }

  // Retrieves a tensor operand 'rhs' from the top of the stack and computes
  // the matrix product self * rhs, returning the result on to the stack.
  // Fails if any of the operands is not a rank-2 tensor, or their respective
  // dimensions are not product-compatible (#colums(self) != #rows(rhs)).
  lua::NResultsOr MMul(lua_State* L) {
    if (LuaTensor* rhs = LuaTensor::ReadObject(L, 2)) {
      const auto& lhs_shape = tensor_view().shape();
      const auto& rhs_shape = rhs->tensor_view().shape();
      if (lhs_shape.size() != 2) {
        return "LHS is not a matrix";
      }
      if (rhs_shape.size() != 2) {
        return "RHS is not a matrix";
      }
      ShapeVector shape = {lhs_shape[0], rhs_shape[1]};
      std::vector<T> storage(Layout::num_elements(shape));
      auto* prod =
          LuaTensor::CreateObject(L, std::move(shape), std::move(storage));
      if (!prod->mutable_tensor_view()->MMul(tensor_view(),
                                             rhs->tensor_view())) {
        return "Incorrect matrix dimensions";
      }
      return 1;
    }
    return absl::StrCat("Must contain 1 RHS tensor of type ", ClassName(),
                        ", received: ", lua::ToString(L, 2));
  }

  // Retrieves a random bit generator (random) from the top of the stack and
  // shuffles the elements of self (in-place) using a permutation computed with
  // such generator.
  // Fails if self is not a rank-1 tensor or if the parameter provided is not
  // a generator.
  // Returns self on to the stack.
  lua::NResultsOr Shuffle(lua_State* L) {
    LuaRandom* random = LuaRandom::ReadObject(L, 2);
    if (random && tensor_view_.Shuffle(random->GetPrbg())) {
      lua_settop(L, 1);
      return 1;
    }
    return absl::StrCat(
        "Must call on a rank-1 Tensor with random number generator, received: ",
        lua::ToString(L, 2));
  }

  lua::NResultsOr ToString(lua_State* L) {
    int max_elements = 1024;
    if (IsTypeMismatch(lua::Read(L, 2, &max_elements))) {
      return "Invalid number of elements passed to function.";
    }
    if (max_elements < 0) {
      max_elements = tensor_view_.num_elements();
    }
    std::ostringstream ss;
    ss << "[" << ClassName() << "]\n";
    tensor_view_.PrintToStream(max_elements, &ss);
    lua::Push(L, ss.str());
    return 1;
  }

  static LuaTensor* ReadObject(lua_State* L, int idx) {
    LuaTensor* const tensor = Class::ReadObject(L, idx);
    return (tensor && tensor->IsValid()) ? tensor : nullptr;
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
