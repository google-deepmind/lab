(Switch to: [Lua](lua_api.md) &middot; [Python](python_api.md) &middot;
 [Level Generation](level_generation.md) &middot;
 Tensor &middot; [Text Levels](text_level.md) &middot;
 [Build](build.md) &middot;
 [Known Issues](issues.md))

# Tensor

This is a small tensor library for manipulating numerical data in DeepMind Lab.
The functionality is similar to that of tensors in Torch.

Tensors can share storage. When data is mutated in one tensor, all other tensors
that share that storage observe the mutation.

Copies of a tensor share storage with the source tensor. All [layout
operations](#layout-operations) result in tensors that share storage with the
source tensor.

Tensors with new storage (initially not shared with any other tensor) can only
be created by one of the explicit [creation](#creation) methods described below.

Example:

```Lua
local tensor = require 'dmlab.system.tensor'

-- Create a rank-2 tensor with extents 4 and 5. Tensors constructed with this
-- function are zero-initialized.
> z = tensor.DoubleTensor(4, 5)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 5]
[[0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0],
 [0, 0, 0, 0, 0]]
```

You can create a tensor from a table, too:

```Lua
> tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}}
[dmlab.system.tensor.DoubleTensor]
Shape: [3, 2]
[[1, 2],
 [3, 4],
 [5, 6]]
```

## Types

There are 7 types of tensor:

*   `dmlab.tensor.ByteTensor`
*   `dmlab.tensor.CharTensor`
*   `dmlab.tensor.Int16Tensor`
*   `dmlab.tensor.Int32Tensor`
*   `dmlab.tensor.Int64Tensor`
*   `dmlab.tensor.FloatTensor`
*   `dmlab.tensor.DoubleTensor`

By default use `DoubleTensor`. Other data types may be difficult to use
correctly, because numeric conversions between Lua's number representation and
the representations used by the underlying tensor implementation may be invalid.

## Creation

For each type of tensor there is a corresponding construction function and
conversion method.

Type           | Construction          | Conversion
:------------- | :-------------------- | :----------
`ByteTensor`   | `tensor.ByteTensor`   | `:byte()`
`CharTensor`   | `tensor.CharTensor`   | `:char()`
`Int16Tensor`  | `tensor.Int16Tensor`  | `:int16()`
`Int32Tensor`  | `tensor.Int32Tensor`  | `:int32()`
`Int64Tensor`  | `tensor.Int64Tensor`  | `:int64()`
`FloatTensor`  | `tensor.FloatTensor`  | `:float()`
`DoubleTensor` | `tensor.DoubleTensor` | `:double()`

The construction functions and conversion methods create tensors that own their
storage. The tensor starts in row-major order, where consecutive elements of the
tensor in layout order are contiguous in memory. (Note in particular that even a
conversion to the same type has the effect of creating a compacted, contiguous
copy of the storage.)

External code may be able to create tensors that do not own their storage. See
[Ownership](#ownership) for more details.

### `tensor.DoubleTensor`(*dim1*, *dim2*, ..., *dimK*)

Creates a rank-K tensor with extents dim1, dim2, ..., dimK. We refer to the
tuple (dim1, dim2, ..., dimK) as the *shape* of the tensor.

```Lua
> tensor.DoubleTensor(3, 2)
[dmlab.system.tensor.DoubleTensor]
Shape: [3, 2]
[[0, 0],
 [0, 0],
 [0, 0]]
```

### `tensor.DoubleTensor`{{*val11*, ...}, {*val21*, ...}, ...}

Creates a tensor from the given hierarchy of tables and values. The shape is
implied.

```Lua
> tensor.DoubleTensor{{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}}
[dmlab.system.tensor.DoubleTensor]
Shape: [2, 2, 2]
[[[1, 2],
  [3, 4]],
 [[5, 6],
  [7, 8]]]
```

### Conversion

A new tensor is returned with new storage and of the specified type. The new
tensor has the same shape as the original and contains the same values after
conversion.

**Warning!** Conversion may have undefined behaviour if an element value cannot
be represented in the new type.

```Lua
local z = tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}}
assert(z:byte() == tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}})
assert(z:char() == tensor.CharTensor{{1, 2}, {3, 4}, {5, 6}})
assert(z:int16() == tensor.Int16Tensor{{1, 2}, {3, 4}, {5, 6}})
assert(z:int32() == tensor.Int32Tensor{{1, 2}, {3, 4}, {5, 6}})
assert(z:int64() == tensor.Int64Tensor{{1, 2}, {3, 4}, {5, 6}})
assert(z:float() == tensor.FloatTensor{{1, 2}, {3, 4}, {5, 6}})

-- This call is equivalent to calling z:clone().
assert(z:double() == tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}})
```

### `clone`()

Equivalent to the conversion function of the same type as the source tensor.

```Lua
z1 = tensor.DoubleTensor{...}
z2 = z1:clone()   -- same as z1:double()
```

## Layout Operations

There are operations that do not affect the underlying storage of the tensor,
but create new views on the same underlying data.

### `transpose`(*dim1*, *dim2*)

Swaps `dim1` and `dim2` of the Tensor.

```Lua
> z = tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}}
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [3, 2]
[[1, 2],
 [3, 4],
 [5, 6]]
> z:transpose(1, 2)
[dmlab.system.tensor.DoubleTensor]
Shape: [2, 3]
[[1, 3, 5],
 [2, 4, 6]]
```

### `select`(*dim*, *index*) and `__call`(*index1*, *index2*, ...)

Returns a slice of the tensor at index in dim.

```Lua
> z = tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}}
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [3, 2]
[[1, 2],
 [3, 4],
 [5, 6]]
> z:select(2, 2)
[dmlab.system.tensor.DoubleTensor]
Shape: [3]
[2, 4, 6]
```

`z(index1, index2, ..., indexN)` is equivalent to `z:select(1, index1):select(1,
index2): ... :select(1, indexN)`.

```Lua
> z = tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}}
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [3, 2]
[[1, 2],
 [3, 4],
 [5, 6]]
> z(2)
[dmlab.system.tensor.DoubleTensor]
Shape: [2]
[3, 4]
> z:select(1, 2)
[dmlab.system.tensor.DoubleTensor]
Shape: [2]
[3, 4]
> z(3,1)
[dmlab.system.tensor.DoubleTensor]
Shape: [1]
[5]
```

### `narrow`(*dim*, *index*, *size*)

Returns a new Tensor which is a narrowed version of the current one. The
dimension `dim` is narrowed from `index` to `index + size - 1`.

```Lua
> v = 0
> z = tensor.DoubleTensor(4, 4):apply(function() v = v + 1 return v end)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 4]
[[ 1,  2,  3,  4],
 [ 5,  6,  7,  8],
 [ 9, 10, 11, 12],
 [13, 14, 15, 16]]
> z:narrow(2, 1, 3)
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 3]
[[ 2,  3,  4],
 [ 6,  7,  8],
 [10, 11, 12],
 [14, 15, 16]]
> z:narrow:narrow(1, 3, 2)
[dmlab.system.tensor.DoubleTensor]
Shape: [2, 4]
[[ 9, 10, 11, 12],
 [13, 14, 15, 16]]
```

## Scalar Operations

Scalar operations work on the tensor object and a single scalar argument value.
The respective element-wise binary operation is applied to pairs of each tensor
element and the scalar value.

### `fill`(*value*)

Fills the tensor with value.

```Lua
> z = tensor.DoubleTensor(4, 4)
> z:fill(7)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 4]
[[7, 7, 7, 7],
 [7, 7, 7, 7],
 [7, 7, 7, 7],
 [7, 7, 7, 7]]
```

### `add`(*value*)

```Lua
> z = tensor.DoubleTensor(4, 4)
> z:fill(5)
> z:add(1)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 4]
[[6, 6, 6, 6],
 [6, 6, 6, 6],
 [6, 6, 6, 6],
 [6, 6, 6, 6]]
```

### `mul`(*value*)

```Lua
> z = tensor.DoubleTensor(4, 4)
> z:fill(0.5)
> z:mul(4)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 4]
[[2, 2, 2, 2],
 [2, 2, 2, 2],
 [2, 2, 2, 2],
 [2, 2, 2, 2]]
```

### `sub`(*value*)

```Lua
> z = tensor.DoubleTensor(4, 4)
> z:fill(2)
> z:sub(4)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 4]
[[-2, -2, -2, -2],
 [-2, -2, -2, -2],
 [-2, -2, -2, -2],
 [-2, -2, -2, -2]]
```

### `div`(*value*)

```Lua
> z = tensor.DoubleTensor(4, 4)
> z:fill(-2)
> z:div(2)
> z
[dmlab.system.tensor.DoubleTensor]
Shape: [4, 4]
[[-1, -1, -1, -1],
 [-1, -1, -1, -1],
 [-1, -1, -1, -1],
 [-1, -1, -1, -1]]
```

## Component Operations

Component operations work on two tensors (the object and the argument) that have
the same *number* of elements, but the tensor *shapes* are ignored. The
respective element-wise binary operation is applied consecutively to pairs of
tensor elements visited in their respective layout order.

It is an error if the two tensors do not have the same number of elements or the
same type.

```Lua
> bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[1, 2],
 [3, 4],
 [5, 6]]
> bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
> bt2
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[1, 2, 3],
 [4, 5, 6]]
```

### `cmul`(*value*)

```Lua
> bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
> bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
> bt:cmul(bt2)
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[ 1,  4],
 [ 9, 16],
 [25, 36]]
```

### `cadd`(*value*)

```Lua
> bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
> bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
> bt:cadd(bt2)
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[ 2,  4],
 [ 6,  8],
 [10, 12]]
```

### `cdiv`(*value*)

```Lua
> bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
> bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
> bt:cdiv(bt2)
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[1, 1],
 [1, 1],
 [1, 1]]
```

### `csub`(*value*)

```Lua
> bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
> bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
> bt:csub(bt2)
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[0, 0],
 [0, 0],
 [0, 0]]
```

### `copy`(*value*)

```Lua
> bt = tensor.ByteTensor(3, 2)
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[0, 0],
 [0, 0],
 [0, 0]]
> bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
> bt:copy(bt2)
> bt
[dmlab.system.tensor.ByteTensor]
Shape: [3, 2]
[[1, 2],
 [3, 4],
 [5, 6]]
```

## Comparison

Two tensors are equal iff the type, shape and values are the same.

```Lua
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}

-- The same:
assert(bt == tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}})

-- Values are different:
assert(bt ~= tensor.ByteTensor{{1, 2}, {3, 4}, {7, 8}})

-- Shape is different:
assert(bt ~= tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}})

-- Type is different:
assert(bt ~= tensor.IntTensor{{1, 2}, {3, 4}, {5, 6}})
```

## Ownership

A tensor may either own its storage (jointly with all copies made from it), or
otherwise it is merely a view on an existing piece of storage that it does not
control. In the latter case, the storage which the tensor is viewing may
disappear spontaneously, and the tensor becomes *invalid*. A tensor that owns
its storage can never become invalid.

You can check whether a valid tensor owns its storage by calling
`z:ownsStorage()`. You can create a tensor that definitely owns its storage by
cloning an existing tensor.

Any attempt to use an invalid tensor will result in a Lua error. There is (by
design) no direct way to check a tensor for validity, and library APIs should
document the guaranteed lifetime of non-owning tensors. In that case, you may
wish to clone the non-owning tensor if you need it for longer than the library
guarantees.

```Lua
local my_api = {}

function my_api.call_me_with_a_tensor(z)
  my_api._valid_tensor = z:ownsStorage() and z or z:clone()
end

function my_api.call_me()
  return my_api._valid_tensor
end

return my_api
```

For debugging purposes, you may check for tensor validity using a pcall:

```Lua
local my_api = {}

-- It is assumed some external code calls this function.
function my_api.call_me_with_a_tensor(z)
  my_api._maybe_invalid_copy = z
end

function my_api.call_me()
  if pcall(my_api._maybe_invalid_copy:type()) then
    return my_api._maybe_invalid_copy
  end
  return nil
end

return my_api
```
