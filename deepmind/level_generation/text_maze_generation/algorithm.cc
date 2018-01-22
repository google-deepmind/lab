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

#include <map>

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
            } else if (is_wall_char[static_cast<unsigned char>(cell_value)]) {
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

namespace {

// Contains Left, Right, Down, Up.
std::array<Vec, 4> PathDirections() {
  return {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
}

// Returns a list of visitable directions containing fill_variation from a
// position step_size away.
std::vector<Vec> PossibleDirections(  //
    const TextMaze& text_maze,        //
    const Pos& pos,                   //
    unsigned int fill_id,             //
    int step_size) {
  std::vector<Vec> result;
  auto rect = text_maze.Area();
  for (const auto& direction : PathDirections()) {
    Pos two_step = pos + step_size * direction;
    if (rect.InBounds(two_step) && text_maze.GetCellId(two_step) == fill_id) {
      result.push_back(direction);
    }
  }
  return result;
}

// Visit the id layer at odd positions in a TextMaze
template <typename F>
void VisitOddIds(const TextMaze& text_maze, F&& f) {
  auto rect = text_maze.Area();
  auto next_odd = [](int v) -> int { return v | 1; };
  for (int i = next_odd(rect.pos.row); i < rect.pos.row + rect.size.height;
       i += 2) {
    for (int j = next_odd(rect.pos.col); j < rect.pos.col + rect.size.width;
         j += 2) {
      f(i, j, text_maze.GetCellId({i, j}));
    }
  }
}

bool RemoveHorseshoeBendAtPos(            //
    Pos pos,                              //
    int bend_size,                        //
    char wall,                            //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze) {
  auto is_wall_char = internal::MakeCharBoolMap(wall_chars);
  is_wall_char[static_cast<unsigned char>(wall)] = true;
  auto is_corridor = [text_maze](const Pos pos) {
    return text_maze->GetCell(TextMaze::kEntityLayer, pos) == ' ';
  };

  auto is_wall = [text_maze, &is_wall_char](const Pos pos) {
    return is_wall_char[static_cast<unsigned char>(
        text_maze->GetCell(TextMaze::kEntityLayer, pos))];
  };

  bool bends_removed = false;
  bool found = false;
  do {
    found = false;
    if (!is_wall(pos)) {
      break;
    }
    for (const auto& v_dir : PathDirections()) {
      const Vec u_dir = {v_dir.d_col, -v_dir.d_row};
      //
      // Input configuration:                Output configuration:
      // origin (0, 0) is wall               origin (0, 0) is corridor
      //
      //     v  bend_size                        v
      //     ^   <----->                         ^
      //    2| ?*********?                      2| ?*********?
      //    1| *         *                      1| ***********
      //    0| ? ******* ?                      0| ?         ?
      //    m| ??*******??                      m| ??*******??
      //     ...---------->u                     ...---------->u
      //       nm0123...po                         nm0123...po
      //
      bool ok = true;
      // pos_u0 and pos_um must be wall for u in [0, bend_size -1]
      for (int u = 0; u < bend_size; ++u) {
        Pos pos_u0 = pos + u * u_dir;
        Pos pos_um = pos + u * u_dir - v_dir;
        if (!is_wall(pos_u0) || !is_wall(pos_um)) {
          ok = false;
          break;
        }
      }
      if (!ok) {
        continue;
      }
      // pos_u1 must be corridor for u in [-1, bend_size]
      for (int u = -1; u <= bend_size; ++u) {
        Pos pos_u1 = pos + u * u_dir + v_dir;
        if (!is_corridor(pos_u1)) {
          ok = false;
          break;
        }
      }
      if (!ok) {
        continue;
      }
      // pos_u2 must be wall for u in [-1, bend_size]
      for (int u = -1; u <= bend_size; ++u) {
        Pos pos_u2 = pos + u * u_dir + 2 * v_dir;
        if (!is_wall(pos_u2)) {
          ok = false;
          break;
        }
      }
      if (!ok) {
        continue;
      }
      // pos_n1 and pos_o1 must be wall
      Pos pos_n1 = pos - 2 * u_dir + v_dir;
      Pos pos_o1 = pos + (bend_size + 1) * u_dir + v_dir;
      if (!is_wall(pos_n1) || !is_wall(pos_o1)) {
        continue;
      }

      // pos_m0 and pos_p0 must be corridor
      Pos pos_m0 = pos - u_dir;
      Pos pos_p0 = pos + bend_size * u_dir;
      if (!is_corridor(pos_m0) || !is_corridor(pos_p0)) {
        continue;
      }

      // pull the string
      bends_removed = true;
      for (int u = -1; u <= bend_size; ++u) {
        Pos pos_u1 = pos + u * u_dir + v_dir;
        if (u >= 0 && u < bend_size) {
          Pos pos_u0 = pos + u * u_dir;
          text_maze->SetCell(
              TextMaze::kEntityLayer, pos_u0,
              text_maze->GetCell(TextMaze::kEntityLayer, pos_u1));
        }
        text_maze->SetCell(TextMaze::kEntityLayer, pos_u1, wall);
      }
      // move pos to pos_0m
      pos = pos - v_dir;
      found = true;
      break;
    }
  } while (found);
  return bends_removed;
}

}  // namespace

void FillWithMaze(         //
    const Pos& pos,        //
    unsigned int maze_id,  //
    TextMaze* text_maze,   //
    std::mt19937_64* prbg) {
  std::vector<Pos> stack;
  stack.push_back(pos);
  unsigned int fill_id = text_maze->GetCellId(pos);
  text_maze->SetCell(TextMaze::kEntityLayer, pos, ' ');
  text_maze->SetCellId(pos, maze_id);
  while (!stack.empty()) {
    Pos current = stack.back();
    // Find the possible directions we can two step to.
    auto possible_directions =
        PossibleDirections(*text_maze, current, fill_id, 2 /*step_size*/);
    if (possible_directions.empty()) {
      stack.pop_back();
      continue;
    }
    int direction_id = std::uniform_int_distribution<>(
        0, possible_directions.size() - 1)(*prbg);
    const auto& direction = possible_directions[direction_id];
    Pos one_step = current + direction;
    text_maze->SetCell(TextMaze::kEntityLayer, one_step, ' ');
    text_maze->SetCellId(one_step, maze_id);
    Pos two_step = current + 2 * direction;
    text_maze->SetCell(TextMaze::kEntityLayer, two_step, ' ');
    text_maze->SetCellId(two_step, maze_id);
    stack.push_back(two_step);
  }
}

void FillSpaceWithMaze(     //
    unsigned int start_id,  //
    unsigned int fill_id,   //
    TextMaze* text_maze,    //
    std::mt19937_64* prbg) {
  auto visitor = [&start_id, fill_id, text_maze, prbg](int i, int j,
                                                       unsigned int id) {
    if (id == fill_id) {
      FillWithMaze({i, j}, start_id++, text_maze, prbg);
    }
  };
  VisitOddIds(*text_maze, visitor);
}

std::vector<std::pair<Pos, Vec>> RandomConnectRegions(  //
    char connector,                                     //
    double extra_probability,                           //
    TextMaze* text_maze,                                //
    std::mt19937_64* prbg) {
  // Find all connecting points between regions.
  std::map<std::pair<unsigned int, unsigned int>,
           std::vector<std::pair<Pos, Vec>>>
      cell_ids_connectors;
  auto visitor = [text_maze, &cell_ids_connectors](int i, int j,
                                                   unsigned int id_0) {
    if (id_0 != 0) {
      Pos pos = {i, j};
      for (const auto& direction : PathDirections()) {
        Pos two_step = pos + 2 * direction;
        if (!text_maze->Area().InBounds(two_step)) continue;
        auto id_1 = text_maze->GetCellId(two_step);
        if (id_1 == 0 || id_1 <= id_0) continue;
        Pos one_step = pos + direction;
        cell_ids_connectors[{id_0, id_1}].emplace_back(one_step, direction);
      }
    }
  };
  VisitOddIds(*text_maze, visitor);

  // Connect each region with at least one connecting point.
  // Then add extra connections with a probability of extra_probability.
  std::vector<std::pair<Pos, Vec>> result;
  for (const auto& cell_ids_connector : cell_ids_connectors) {
    const auto& locations = cell_ids_connector.second;
    if (locations.empty()) continue;
    int door = std::uniform_int_distribution<>(0, locations.size() - 1)(*prbg);
    result.push_back(locations[door]);
    text_maze->SetCell(TextMaze::kEntityLayer, locations[door].first,
                       connector);
  }

  for (const auto& cell_ids_connector : cell_ids_connectors) {
    const auto& locations = cell_ids_connector.second;
    if (locations.empty()) continue;
    for (std::size_t loc_id = 0; loc_id != locations.size(); ++loc_id) {
      if (std::uniform_real_distribution<>(0, 1)(*prbg) <= extra_probability) {
        bool next_to_door = false;
        for (auto direction : PathDirections()) {
          Pos one_step = locations[loc_id].first + direction;
          if (text_maze->GetCell(TextMaze::kEntityLayer, one_step) ==
              connector) {
            next_to_door = true;
            break;
          }
        }
        if (!next_to_door) {
          result.push_back(locations[loc_id]);
          text_maze->SetCell(TextMaze::kEntityLayer, locations[loc_id].first,
                             connector);
        }
      }
    }
  }
  return result;
}

bool RemoveHorseshoeBends(                //
    int bend_size,                        //
    char wall,                            //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze) {
  bool bends_removed = false;
  auto visitor = [bend_size, wall, &wall_chars, text_maze, &bends_removed](
                     int i, int j, char value) {
    bends_removed =
        bends_removed || RemoveHorseshoeBendAtPos({i, j}, bend_size, wall,
                                                  wall_chars, text_maze);
  };
  text_maze->Visit(TextMaze::kEntityLayer, visitor);
  return bends_removed;
}

void RemoveAllHorseshoeBends(             //
    char wall,                            //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze) {
  for (int i = 1; i + 3 < text_maze->Area().size.width;) {
    bool bends_removed = RemoveHorseshoeBends(i, wall, wall_chars, text_maze);
    // Removing bends of bend_size > 1 may generate smaller loops.
    // We must start again in this case.
    if (bends_removed && i != 1) {
      i = 1;
    } else {
      ++i;
    }
  }
}

void AddNEntitiesToEachRoom(              //
    const std::vector<Rectangle>& rooms,  //
    int n,                                //
    char entity,                          //
    char empty,                           //
    TextMaze* text_maze,                  //
    std::mt19937_64* prbg) {
  for (const auto& room : rooms) {
    std::vector<Pos> samples;
    text_maze->VisitIntersection(TextMaze::kEntityLayer, room,
                                 [empty, &samples](int i, int j, char value) {
                                   if (value == empty) {
                                     samples.push_back({i, j});
                                   }
                                 });
    std::shuffle(samples.begin(), samples.end(), *prbg);
    for (std::size_t i = 0;
         i < std::min(samples.size(), static_cast<std::size_t>(n)); ++i) {
      text_maze->SetCell(TextMaze::kEntityLayer, samples[i], entity);
    }
  }
}

std::vector<Pos> FindRandomPath(          //
    const Pos& from,                      //
    const Pos& to,                        //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze,                  //
    std::mt19937_64* prbg) {
  // Set the Id of all visitable locations to 1, 0 otherwise.
  auto is_wall_char = internal::MakeCharBoolMap(wall_chars);
  text_maze->Visit(TextMaze::kEntityLayer, [text_maze, is_wall_char](
                                               int i, int j, char cell) {
    text_maze->SetCellId(
        {i, j}, is_wall_char[static_cast<unsigned char>(cell)] ? 0 : 1);
  });
  std::vector<Pos> path;
  path.push_back(from);
  text_maze->SetCellId(from, 0);
  if (from == to) {
    return path;
  }
  while (!path.empty()) {
    std::vector<Pos> candidates;
    const auto& pos = path.back();
    for (const auto& direction : PathDirections()) {
      auto candidate = pos + direction;
      if (candidate == to) {
        path.push_back(candidate);
        return path;
      }
      if (text_maze->GetCellId(candidate) != 0) {
        candidates.push_back(candidate);
      }
    }
    if (candidates.empty()) {
      path.pop_back();
      continue;
    }
    int candidate_id =
        std::uniform_int_distribution<>(0, candidates.size() - 1)(*prbg);
    Pos candidate = candidates[candidate_id];
    path.push_back(candidate);
    text_maze->SetCellId(candidate, 0);
  }
  return path;
}

}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
