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

#include "deepmind/level_generation/text_maze_generation/algorithm.h"

#include "deepmind/level_generation/text_maze_generation/flood_fill.h"

namespace deepmind {
namespace lab {
namespace maze_generation {

TextMaze FromCharGrid(const CharGrid& entity_layer) {
  TextMaze result({static_cast<int>(entity_layer.height()),
                   static_cast<int>(entity_layer.width())});
  result.VisitMutable(TextMaze::kEntityLayer,
                      [&entity_layer](int i, int j, char* c) {
                        auto cell = entity_layer.CellAt(i, j);
                        *c = cell != '\0' ? cell : '*';
                      });
  return result;
}

TextMaze FromCharGrid(const CharGrid& entity_layer,
                      const CharGrid& variations_layer) {
  TextMaze result({static_cast<int>(entity_layer.height()),
                   static_cast<int>(entity_layer.width())});
  result.VisitMutable(TextMaze::kEntityLayer,
                      [&entity_layer](int i, int j, char* c) {
                        auto cell = entity_layer.CellAt(i, j);
                        *c = cell != '\0' ? cell : *c;
                      });
  result.VisitMutable(TextMaze::kVariationsLayer,
                      [&variations_layer](int i, int j, char* c) {
                        auto cell = variations_layer.CellAt(i, j);
                        *c = cell != '\0' ? cell : *c;
                      });
  return result;
}

std::vector<std::vector<Pos>> FindRooms(const TextMaze& text_maze,
                                        const std::vector<char>& wall_chars) {
  auto is_wall_char = internal::MakeCharBoolMap(wall_chars);

  std::vector<int> distances;
  const auto& area = text_maze.Area();
  distances.reserve(area.Area());

  // Mark all non-traversable cells with -2 and traversable with -1.
  text_maze.Visit(TextMaze::kEntityLayer,
                  [&distances, &is_wall_char](int i, int j, char c) {
    distances.push_back(is_wall_char[static_cast<unsigned char>(c)] ? -2 : -1);
  });

  // Next sections find corridors, T-junctions and dead-ends and marks them
  // as also non-traversable.

  auto distances_lookup = [&area, &distances](int i, int j) -> int& {
    return distances[internal::DistanceIndex(area, i, j)];
  };

  // Create with a border to save having to do boundary checks.
  std::vector<std::bitset<8>> adjacent_info((area.size.height + 2) *
                                            (area.size.width + 2));

  // Create lookup into adjacency_info.
  int adj_width = area.size.width + 2;
  auto adjacent_lookup =
      [adj_width, &adjacent_info](int i, int j) -> std::bitset<8>& {
    return adjacent_info[(i + 1) * adj_width + j + 1];
  };

  // Fill adjacent_info with connectivity information.
  // The adjencency_info will represent the openings at i, j.
  //  765
  //  4 3
  //  210
  area.Visit([&distances_lookup, &adjacent_lookup](int i, int j) {
    bool is_open = distances_lookup(i, j) == -1;
    adjacent_lookup(i - 1, j - 1)[0] = is_open;
    adjacent_lookup(i + 0, j - 1)[1] = is_open;
    adjacent_lookup(i + 1, j - 1)[2] = is_open;
    adjacent_lookup(i - 1, j + 0)[3] = is_open;
    adjacent_lookup(i + 1, j + 0)[4] = is_open;
    adjacent_lookup(i - 1, j + 1)[5] = is_open;
    adjacent_lookup(i + 0, j + 1)[6] = is_open;
    adjacent_lookup(i + 1, j + 1)[7] = is_open;
  });

  // Remove any T-Junctions or L-bends.
  for (auto& adj : adjacent_info) {
    adj[1] = adj[1] && (adj[0] || adj[2]);
    adj[6] = adj[6] && (adj[5] || adj[7]);
    adj[3] = adj[3] && (adj[0] || adj[5]);
    adj[4] = adj[4] && (adj[2] || adj[7]);
  }

  area.Visit([&distances_lookup, &adjacent_lookup](int i, int j) {
    if (distances_lookup(i, j) != -1) return;
    auto& adjacency_info = adjacent_lookup(i, j);
    if ((!adjacency_info[1] && !adjacency_info[6]) ||
        (!adjacency_info[3] && !adjacency_info[4])) {
      distances_lookup(i, j) = -2;
    }
  });

  // 'distances' now only contains '-1' in cells that are considered rooms.
  std::vector<std::vector<Pos>> result;

  area.Visit([&distances, &area, &result](int i, int j) {
    std::vector<Pos> connected;
    if (internal::FloodFill({i, j}, area, &distances, &connected)) {
      result.push_back(std::move(connected));
    }
  });
  return result;
}

// Generates a random rectangle in bounds.
// This algorithm avoids large area rectangles.
// The short side of the rectangle is chosen uniformly between min_size and
// mid_size (min_size + max_size)/2. The long side of the rectangle is the sum
// of uniform(min_size, mid_size) and uniform(mid_size, max_size) - mid_size.
namespace {
Rectangle MakeRandomRectangle(const Rectangle& bounds, const Size& min_size,
                              const Size& max_size, std::mt19937_64* prbg) {
  auto uniform = [prbg](int min, int max) {
    return min < max ? std::uniform_int_distribution<>(min, max)(*prbg) : min;
  };

  int mid_width = (max_size.width + min_size.width) / 2;
  int mid_height = (max_size.height + min_size.height) / 2;
  int width = uniform(min_size.width, mid_width);
  int height = uniform(min_size.height, mid_height);

  if (uniform(0, 1) == 0) {
    width += uniform(mid_width, max_size.width) - mid_width;
  } else {
    height += uniform(mid_height, max_size.height) - mid_height;
  }

  int row = bounds.pos.row + uniform(0, bounds.size.height - height - 1);
  int col = bounds.pos.col + uniform(0, bounds.size.width - width - 1);
  return Rectangle{{row, col}, {height, width}};
}
}  // namespace

std::vector<Rectangle> MakeSeparateRectangles(
    const Rectangle& bounds, const SeparateRectangleParams& params,
    std::mt19937_64* prbg) {
  std::vector<Rectangle> rects;
  const int target_rect_cells = bounds.Area() * params.density;
  int retries = 0;
  int rect_cells = 0;

  auto grow_rect = [](Rectangle rect) {
    return Rectangle{{rect.pos.row * 2 + 1, rect.pos.col * 2 + 1},
                     {rect.size.height * 2 + 1, rect.size.width * 2 + 1}};
  };

  auto shrink_rect = [](Rectangle rect) {
    return Rectangle{{(rect.pos.row - 1) / 2, (rect.pos.col - 1) / 2},
                     {(rect.size.height - 1) / 2, (rect.size.width - 1) / 2}};
  };

  auto shrink_size = [](Size size) {
    return Size{(size.height - 1) / 2, (size.width - 1) / 2};
  };

  while (retries < params.retry_count && rect_cells < target_rect_cells &&
         (params.max_rects == 0 || rects.size() != params.max_rects)) {
    Rectangle rect = grow_rect(
        MakeRandomRectangle(shrink_rect(bounds), shrink_size(params.min_size),
                            shrink_size(params.max_size), prbg));

    if (std::all_of(rects.begin(), rects.end(),
                    [rect](const Rectangle& rect_other) {
                      return IsSeparate(rect, rect_other);
                    })) {
      rects.push_back(rect);
      rect_cells += rect.Area();
    } else {
      ++retries;
    }
  }
  // As it gets harder to place larger rectangles we shuffle to remove bias.
  std::shuffle(rects.begin(), rects.end(), *prbg);
  return rects;
}

void RemoveDeadEnds(char empty, char wall, const std::vector<char>& wall_chars,
                    TextMaze* text_maze) {
  auto is_wall_char = internal::MakeCharBoolMap(wall_chars);
  is_wall_char[static_cast<unsigned char>(wall)] = true;
  const auto& area = text_maze->Area();
  area.Visit([&area, text_maze, &is_wall_char, empty, wall](int r, int c) {
    Pos pos{r, c};
    while (true) {
      if (text_maze->GetCell(TextMaze::kEntityLayer, pos) != empty) {
        break;
      }
      int empty_count = 0;
      int wall_count = 0;
      int visit_count = 0;
      Pos last_pos;
      area.VisitNeighbours(
          pos, [text_maze, &last_pos, &wall_count, &empty_count, &visit_count,
                &is_wall_char, empty](int look_at_row, int look_at_col) {
            Pos look_at_pos{look_at_row, look_at_col};
            const char cell_value =
                text_maze->GetCell(TextMaze::kEntityLayer, look_at_pos);
            if (cell_value == empty) {
              last_pos = look_at_pos;
              ++empty_count;
            } else if (is_wall_char[cell_value]) {
              ++wall_count;
            }
            ++visit_count;
          });
      if (wall_count + 1 < visit_count) {
        break;
      }
      text_maze->SetCell(TextMaze::kEntityLayer, pos, wall);
      if (empty_count == 0) {
        break;
      }
      pos = last_pos;
    }
  });
}

}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
