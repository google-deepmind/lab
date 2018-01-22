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

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_MAZE_GENERATION_TEXT_MAZE_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_MAZE_GENERATION_TEXT_MAZE_H_

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

namespace deepmind {
namespace lab {
namespace maze_generation {

struct Vec {
  int d_row;
  int d_col;
};

inline Vec operator*(int scl, const Vec& vec) {
  return {scl * vec.d_row, scl * vec.d_col};
}

struct Pos {
  int row;
  int col;
};

inline Pos operator+(const Pos& pos, const Vec& vec) {
  return {pos.row + vec.d_row, pos.col + vec.d_col};
}

inline Pos operator-(const Pos& pos, const Vec& vec) {
  return {pos.row - vec.d_row, pos.col - vec.d_col};
}

inline bool operator==(const Pos& op0, const Pos& op1) {
  return op0.row == op1.row && op0.col == op1.col;
}

struct Size {
  int height;
  int width;
};

struct Rectangle {
  Pos pos;
  Size size;

  int Area() const { return size.height * size.width; }

  // Visit all i, j in rectangle.
  template <typename F>
  void Visit(F&& f) const {
    for (int i = pos.row; i < pos.row + size.height; ++i) {
      for (int j = pos.col; j < pos.col + size.width; ++j) {
        f(i, j);
      }
    }
  }

  // Visit all i, j adjacent to point.
  template <typename F>
  void VisitNeighbours(const Pos& point, F&& f) const {
    if (0 <= point.row - 1) {
      f(point.row - 1, point.col);
    }
    if (point.row + 1 < pos.row + size.height) {
      f(point.row + 1, point.col);
    }
    if (0 <= point.col - 1) {
      f(point.row, point.col - 1);
    }
    if (point.col + 1 < pos.col + size.width) {
      f(point.row, point.col + 1);
    }
  }

  bool InBounds(const Pos& point) const {
    return (pos.row <= point.row && point.row < pos.row + size.height &&
            pos.col <= point.col && point.col < pos.col + size.width);
  }
};

// An empty intersection has zero Area().
inline Rectangle Overlap(const Rectangle& lhs, const Rectangle& rhs) {
  const int col = std::max(lhs.pos.col, rhs.pos.col);
  const int row = std::max(lhs.pos.row, rhs.pos.row);
  const int width =
      std::min(lhs.pos.col + lhs.size.width, rhs.pos.col + rhs.size.width) -
      col;
  const int height =
      std::min(lhs.pos.row + lhs.size.height, rhs.pos.row + rhs.size.height) -
      row;
  return {{row, col}, {height > 0 ? height : 0, width > 0 ? width : 0}};
}

// Returns whether 'lhs' and 'rhs' are not overlapping.
inline bool IsSeparate(const Rectangle& lhs, const Rectangle& rhs) {
  return (lhs.pos.row + lhs.size.height <= rhs.pos.row ||
          rhs.pos.row + rhs.size.height <= lhs.pos.row ||
          lhs.pos.col + lhs.size.width <= rhs.pos.col ||
          rhs.pos.col + rhs.size.width <= lhs.pos.col);
}

// Wrapper around strings that represent the entity layer and variations layer,
// allowing mutable access to characters (cells) in these strings.
class TextMaze {
 public:
  // Selects which layer to apply operations to.
  enum Layer { kEntityLayer, kVariationsLayer };

  // Creates an entity and variation layer of a text level with extents of
  // 'extents'. Each layer is constructed as a new-line separated block of text.
  // Each layer starts filled with a default character. For the entity layer it
  // is '*' and for the variations layer it is '.'.
  explicit TextMaze(Size extents);

  // Calls f(i, j, cell) for each cell (i, j) in the intersection of the maze
  // and rect.
  template <typename F>
  void VisitIntersection(Layer layer, const Rectangle& rect, F&& f) const {
    const auto& text = text_[layer];
    Overlap(Area(), rect).Visit([this, &text, &f](int i, int j) {
      f(i, j, text[ToTextIdx(i, j)]);
    });
  }

  // Mutable variant of VisitIntersection.
  template <typename F>
  void VisitMutableIntersection(Layer layer, const Rectangle& rect, F&& f) {
    auto& text = text_[layer];
    Overlap(Area(), rect).Visit([this, &text, &f](int i, int j) {
      f(i, j, &text[ToTextIdx(i, j)]);
    });
  }

  // Calls f(i, j, cell) for each cell (i, j).
  template <typename F>
  void Visit(Layer layer, F&& f) const {
    VisitIntersection(layer, Area(), std::forward<F>(f));
  }

  // Mutable variant of Visit.
  template <typename F>
  void VisitMutable(Layer layer, F&& f) {
    VisitMutableIntersection(layer, Area(), std::forward<F>(f));
  }

  // Returns the character at position pos in layer layer, or '\0' of pos is out
  // of bounds of the maze.
  char GetCell(Layer layer, Pos pos) const {
    if (Area().InBounds(pos)) {
      return text_[layer][ToTextIdx(pos.row, pos.col)];
    } else {
      return '\0';
    }
  }

  // Sets the character at position 'pos' in layer 'layer' to 'value' if pos is
  // within bounds of the maze; otherwise there is no effect.
  void SetCell(Layer layer, Pos pos, char value) {
    if (Area().InBounds(pos)) {
      text_[layer][ToTextIdx(pos.row, pos.col)] = value;
    }
  }

  // Sets the characters within the rectangle 'rect' to 'value' for the subset
  // of 'rect' that's within the bounds of the maze.
  void FillRect(Layer layer, const Rectangle& rect, char value) {
    VisitMutableIntersection(layer, rect, [value](int i, int j, char* c) {
        *c = value;
    });
  }

  // Returns the id at position pos, or 0 of pos is out of bounds of the maze.
  unsigned int GetCellId(Pos pos) const {
    if (Area().InBounds(pos)) {
      return ids_[ToIdIdx(pos.row, pos.col)];
    } else {
      return 0;
    }
  }

  // Sets the character at position 'pos' in layer 'layer' to 'value' if pos is
  // within bounds of the maze; otherwise there is no effect.
  void SetCellId(Pos pos, unsigned int id) {
    if (Area().InBounds(pos)) {
      ids_[ToIdIdx(pos.row, pos.col)] = id;
    }
  }

  // Perform 'rotation' number of clockwise rotations on the maze.
  // If 'rotation' is negative, rotate counterclockwise instead.
  // A 'rotation' of any multiple of 4 returns a copy of the passed in maze.
  // Returns a rotated copy of the maze. Return-by-value is reasonable because
  // of the move operator on TextMaze.
  TextMaze Rotate(int rotation) const;

  // Copy the 'layer' of 'maze' into the position start at 'pos'.
  // Clamp the copy to the bounds of the current maze.
  void Paste(Layer layer, Pos pos, const TextMaze& maze) {
    Rectangle rect{pos, maze.Area().size};
    VisitMutableIntersection(layer, rect, [&](int i, int j, char* c) {
        *c = maze.GetCell(layer, Pos{i - pos.row, j - pos.col});
    });
  }

  // Returns text associated with the 'layer'.
  const std::string& Text(Layer layer) const { return text_[layer]; }

  // Area representing mutable cells of the grid.
  const Rectangle& Area() const { return area_; }

 private:
  // Translates grid coordinates to the linear character position in the layer
  // text string. Use only when (i, j) is within bounds.
  // (j is allowed to be area_.size.width for setting new-lines.)
  int ToTextIdx(int i, int j) const { return i * (area_.size.width + 1) + j; }

  // Translates grid coordinates to the linear id position. Use only when (i, j)
  // is within bounds.
  int ToIdIdx(int i, int j) const { return i * area_.size.width + j; }

  Rectangle area_;
  std::array<std::string, 2> text_;
  std::vector<unsigned int> ids_;
};

}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_MAZE_GENERATION_TEXT_MAZE_H_
