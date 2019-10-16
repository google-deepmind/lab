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

#include "deepmind/level_generation/text_maze_generation/flood_fill.h"

#include <utility>

namespace deepmind {
namespace lab {
namespace maze_generation {
namespace internal {

bool FloodFill(const Pos goal, const Rectangle& area,
               std::vector<int>* distances, std::vector<Pos>* connected) {
  const int flat_idx = DistanceIndex(area, goal.row, goal.col);
  if (!area.InBounds(goal) || (*distances)[flat_idx] != -1) {
    return false;
  }

  std::vector<Pos> current_points, next_points;
  current_points.push_back(goal);
  int cost = 0;
  (*distances)[flat_idx] = cost;
  while (!current_points.empty()) {
    ++cost;
    for (auto pos : current_points) {
      area.VisitNeighbours(
          pos, [&area, &next_points, distances, cost](int i, int j) {
            auto& distance = (*distances)[DistanceIndex(area, i, j)];
            if (distance == -1) {
              distance = cost;
              next_points.push_back({i, j});
            }
          });
    }
    connected->insert(connected->end(), current_points.begin(),
                      current_points.end());
    current_points.clear();
    std::swap(current_points, next_points);
  }
  return true;
}

}  // namespace internal

int FloodFill::DistanceFrom(Pos pos) const {
  if (area_.InBounds(pos)) {
    int distance = distances_[pos.row * area_.size.width + pos.col];
    return distance >= 0 ? distance : -1;
  } else {
    return -1;
  }
}

FloodFill::FloodFill(const TextMaze& maze, TextMaze::Layer layer, Pos goal,
                     const std::vector<char>& wall_chars)
    : area_(maze.Area()) {
  auto is_wall = internal::MakeCharBoolMap(wall_chars);
  distances_.reserve(maze.Area().Area());
  maze.Visit(layer, [this, &is_wall](int i, int j, int c) {
    distances_.push_back(is_wall[c] ? -2 : -1);
  });
  internal::FloodFill(goal, area_, &distances_, &connected_);
}

std::vector<Pos> FloodFill::ShortestPathFrom(Pos pos,
                                             std::mt19937_64* rng) const {
  std::vector<Pos> result;

  int distance = DistanceFrom(pos);
  if (distance == -1) {
    return result;
  }
  result.reserve(distance + 1);
  result.push_back(pos);
  while (distance--) {
    const auto& next_pos = result.back();
    result.emplace_back();
    int choice = 0;
    area_.VisitNeighbours(
        next_pos, [&result, rng, &choice, distance, this](int i, int j) {
          if (distances_[internal::DistanceIndex(area_, i, j)] == distance) {
            ++choice;
            if (choice == 1 ||
                std::uniform_int_distribution<>(1, choice)(*rng) == 1) {
              result.back() = {i, j};
            }
          }
        });
  }
  return result;
}

}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
