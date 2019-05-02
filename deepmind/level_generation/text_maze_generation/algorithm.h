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

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_MAZE_GENERATION_ALGORITHM_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_MAZE_GENERATION_ALGORITHM_H_

#include <cstddef>
#include <random>
#include <vector>

#include "deepmind/level_generation/text_level/char_grid.h"
#include "deepmind/level_generation/text_maze_generation/text_maze.h"

namespace deepmind {
namespace lab {
namespace maze_generation {

// Creates a TextMaze setting the entity layer from a CharGrid.
TextMaze FromCharGrid(const CharGrid& entity_layer);

// Creates a TextMaze setting the entity and variation layers from CharGrids.
TextMaze FromCharGrid(const CharGrid& entity_layer,
                      const CharGrid& variations_layer);

// Returns a vector of all rooms. Each room is a set of connected positions that
// are not corridors, dead-ends or T-junctions.
// 'text_maze' entity layer is examined for rooms.
// 'wall_chars' are characters in text maze that are non-traversable.
std::vector<std::vector<Pos>> FindRooms(const TextMaze& text_maze,
                                        const std::vector<char>& wall_chars);

// Set of parameters used for making separated rectangles.
// See MakeRandomRectangle in implementation to understand the shape and
// distribution of rectangles generated.
struct SeparateRectangleParams {
  // Note the max short side of the rectangle is the mean of (min_size,
  // max_size).
  Size min_size;  // Min size of short side of a rectangle. Shall be odd and
                  // greater than or equal to 3.
  Size max_size;  // Max size of long side of a rectangle. Shall be odd and
                  // greater than or equal to 3.

  // Maximum proportion of the bounds covered in rectangles.
  // Shall be between in range (0, 1.0].
  double density;

  // Maximum number of rectangles to generate. A value of 0 means no limit.
  std::size_t max_rects;

  // Maximum number of attempts to place a random rectangle.
  int retry_count;
};

// Generates non-overlapping rectangles on the odd grid points.
// The rooms are placed within the width and height and according to the params.
// The params.max_size shall be at least two less than bounds.size in height and
// width directions.
std::vector<Rectangle> MakeSeparateRectangles(
    const Rectangle& bounds, const SeparateRectangleParams& params,
    std::mt19937_64* prbg);

// Removes dead-ends from the entity layer of the maze by filling them with
// 'wall'. A dead-end is an cell containing 'empty' next to three or more cells
// that are either containing wall or wall_chars, or out of bounds.
void RemoveDeadEnds(char empty, char wall, const std::vector<char>& wall_chars,
                    TextMaze* text_maze);

// Implements the recursive backtracking maze generation algorithm, starting
// from 'pos' and spreading across space with the same id value, and replacing
// it with 'maze_id'.
void FillWithMaze(         //
    const Pos& pos,        //
    unsigned int maze_id,  //
    TextMaze* text_maze,   //
    std::mt19937_64* prbg);

// Iteratively invokes FillWithMaze for all positions within text_maze with
// id value 'fill_id', assigning sequential id values to each maze sequence
// starting from 'start_id'.
void FillSpaceWithMaze(     //
    unsigned int start_id,  //
    unsigned int fill_id,   //
    TextMaze* text_maze,    //
    std::mt19937_64* prbg);

// Locates connections between adjacent regions in the id layer, placing
// value 'connector' in the relevant positions of the entity layer. At least one
// connection will be identified between each pair of adjacent regions, with
// additional connections created with probability 'extra_probability'.
// Returns a vector with the position and directions of the connections.
std::vector<std::pair<Pos, Vec>> RandomConnectRegions(  //
    char connector,                                     //
    double extra_probability,                           //
    TextMaze* text_maze,                                //
    std::mt19937_64* prbg);

// Simplifies all corridors in 'text_maze' by removing horseshoe bends of a
// given size. Horseshoe bends are meandering sub-paths in a corridor where the
// adjacent segments are collinear and can be reduced to a single segment by
// connecting both ends of the sub-path, without crossing over any other
// corridors.
// For instance, in the case of a corridor linking rooms A and B within an
// example maze:
//
// horseshoe bend of size 1:           corridor with the bend removed:
//
//      *********                           *********
//      ***   ***                           *********
//      *** * *BB                           *******BB
//      *   *  BB                           *      BB
//      AA*******                           AA*******
//      AA*******                           AA*******
//
// The size of the bend is the number of maze cells used to shortcut it.
// Horseshoe bends only contain 4 turns. More complex meandering sub-paths
// can be removed by repeatedly invoking this function:
//
// complex sub-path:         step 1:                   step 2:
//
//      *********                 *********                 *********
//      ***   ***                 ***   ***                 *********
//      *** *   *                 *** * ***                 *********
//      *   *** *                 *** * ***                 *********
//      * ***   *                 *** * ***                 *********
//      *   * ***                 *** * ***                 *********
//      *** * *BB                 *** * *BB                 *******BB
//      *   *  BB                 *   *  BB                 *      BB
//      AA*******                 AA*******                 AA*******
//      AA*******                 AA*******                 AA*******
//
// Returns whether any bends were removed.
bool RemoveHorseshoeBends(                //
    int bend_size,                        //
    char wall,                            //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze);

// Removes horseshoe bends of all sizes in all maze corridors.
void RemoveAllHorseshoeBends(             //
    char wall,                            //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze);

// For each region in 'rooms', attempts to set 'n' random cells to value
// 'entity' in the entity layer of 'text_maze'. This only operates on cells
// currently set to value 'empty'.
void AddNEntitiesToEachRoom(              //
    const std::vector<Rectangle>& rooms,  //
    int n,                                //
    char entity,                          //
    char empty,                           //
    TextMaze* text_maze,                  //
    std::mt19937_64* prbg);

// Attempts to find in 'text_maze' a random path between positions 'from' and
// 'to', while considering as walls the characters in 'wall_chars'. If
// successful, the function returns a vector of the path positions, in order of
// traversal. Otherwise it returns an empty vector.
std::vector<Pos> FindRandomPath(          //
    const Pos& from,                      //
    const Pos& to,                        //
    const std::vector<char>& wall_chars,  //
    TextMaze* text_maze,                  //
    std::mt19937_64* prbg);

}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_MAZE_GENERATION_ALGORITHM_H_
