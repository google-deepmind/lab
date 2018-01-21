// Copyright (C) 2017 Google Inc.
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

#include "deepmind/engine/context_game.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>
#include <vector>

#include "deepmind/lua/class.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"
#include "deepmind/util/files.h"

namespace deepmind {
namespace lab {
namespace {

class StorageFreeChar : public tensor::StorageValidity {
 public:
  StorageFreeChar(char* data) : StorageValidity(kOwnsStorage), data_(data) {}
  ~StorageFreeChar() { free(data_); }
  unsigned char* data() { return reinterpret_cast<unsigned char*>(data_); }

 private:
  char* data_;
};

class StorageString : public tensor::StorageValidity {
 public:
  StorageString(std::string data)
      : StorageValidity(kOwnsStorage), data_(std::move(data)) {}
  unsigned char* data() { return reinterpret_cast<unsigned char*>(&data_[0]); }

 private:
  std::string data_;
};

class LuaGameModule : public lua::Class<LuaGameModule> {
  friend class Class;
  static const char* ClassName() { return "deepmind.lab.Game"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaGameModule(ContextGame* ctx) : ctx_(ctx) {}

  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"addScore", Member<&LuaGameModule::AddScore>},
        {"finishMap", Member<&LuaGameModule::FinishMap>},
        {"playerInfo", Member<&LuaGameModule::PlayerInfo>},
        {"episodeTimeSeconds", Member<&LuaGameModule::EpisodeTimeSeconds>},
        {"tempFolder", Member<&LuaGameModule::TempFolder>},
        {"runFiles", Member<&LuaGameModule::ExecutableRunfiles>},
        {"raycast", Member<&LuaGameModule::Raycast>},
        {"inFov", Member<&LuaGameModule::InFov>},
        {"loadFileToByteTensor", Member<&LuaGameModule::LoadFileToByteTensor>},
        {"loadFileToString", Member<&LuaGameModule::LoadFileToString>},
        {"copyFileToLocation", Member<&LuaGameModule::CopyFileToLocation>},
        {"renderCustomView", Member<&LuaGameModule::RenderCustomView>},
        {"screenShape", Member<&LuaGameModule::ScreenShape>},
    };
    Class::Register(L, methods);
  }

 private:
  lua::NResultsOr AddScore(lua_State* L) {
    int player_id = 0;
    double score = 0;
    if (lua::Read(L, 2, &player_id) && lua::Read(L, 3, &score) &&
        0 <= player_id && player_id < 64) {
      ctx_->Calls()->add_score(player_id, score);
      return 0;
    }
    std::string error = "Invalid arguments player_id: ";
    error += lua::ToString(L, 2);
    error += " or reward: ";
    error += lua::ToString(L, 3);
    return std::move(error);
  }

  lua::NResultsOr Raycast(lua_State* L) {
    std::array<float, 3> start, end;
    if (lua::Read(L, 2, &start) && lua::Read(L, 3, &end)) {
      lua::Push(L, ctx_->Calls()->raycast(start.data(), end.data()));
      return 1;
    }
    return "Must provide start and end coordinates";
  }

  lua::NResultsOr InFov(lua_State* L) {
    std::array<float, 3> start, end, angles;
    float fov = 360.0f;
    if (lua::Read(L, 2, &start) && lua::Read(L, 3, &end) &&
        lua::Read(L, 4, &angles)) {
      lua::Read(L, 5, &fov);
      lua::Push(L, ctx_->Calls()->in_fov(start.data(), end.data(),
                                         angles.data(), fov));
      return 1;
    } else {
      return "Must provide start, end coordinates and orientation angles";
    }
  }

  lua::NResultsOr ScreenShape(lua_State* L) {
    int screen_width, screen_height, max_width, max_height;
    ctx_->Calls()->screen_shape(&screen_width, &screen_height, &max_width,
                                &max_height);
    auto table = lua::TableRef::Create(L);
    auto screen = table.CreateSubTable("window");
    screen.Insert("width", screen_width);
    screen.Insert("height", screen_height);
    auto buffer = table.CreateSubTable("buffer");
    buffer.Insert("width", max_width);
    buffer.Insert("height", max_height);
    lua::Push(L, table);
    return 1;
  }

  lua::NResultsOr RenderCustomView(lua_State* L) {
    lua::TableRef args;
    if (!lua::Read(L, 2, &args)) {
      return "[customView] - Must call with table containing "
             "'width', 'height', 'pos' and 'look'.";
    }

    int requested_width, requested_height;
    std::array<float, 3> pos, eye;

    if (!args.LookUp("width", &requested_width))
      return "[customView] - Missing named arg 'width'";
    if (!args.LookUp("height", &requested_height))
      return "[customView] - Missing named arg 'height'";
    if (!args.LookUp("look", &eye))
      return "[customView] - Missing named arg 'look'";
    if (!args.LookUp("pos", &pos))
      return "[customView] - Missing named arg 'pos'";

    bool render_player = true;
    args.LookUp("renderPlayer", &render_player);

    int width, height, buff_width, buff_height;
    ctx_->Calls()->screen_shape(&width, &height, &buff_width, &buff_height);
    requested_width = std::min(buff_width, requested_width);
    requested_height = std::min(buff_height, requested_height);

    ctx_->SetCustomView(requested_width, requested_height, pos, eye,
                        render_player);
    tensor::ShapeVector shape = {static_cast<std::size_t>(requested_height),
                                 static_cast<std::size_t>(requested_width), 3};
    std::vector<unsigned char> storage(tensor::Layout::num_elements(shape));
    ctx_->Calls()->render_custom_view(requested_width, requested_height,
                                      storage.data());
    tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(shape),
                                                   std::move(storage));
    return 1;
  }

  lua::NResultsOr FinishMap(lua_State* L) {
    ctx_->SetMapFinished(true);
    return 0;
  }

  lua::NResultsOr PlayerInfo(lua_State* L) {
    const auto& pv = ctx_->GetPlayerView();
    auto table = lua::TableRef::Create(L);
    table.Insert("pos", pv.pos);
    table.Insert("vel", pv.vel);
    table.Insert("angles", pv.angles);
    table.Insert("anglesVel", pv.anglesVel);
    table.Insert("height", pv.height);
    table.Insert("playerId", pv.player_id + 1);
    table.Insert("teamScore", pv.team_score);
    table.Insert("otherTeamScore", pv.other_team_score);
    lua::Push(L, table);
    return 1;
  }

  lua::NResultsOr EpisodeTimeSeconds(lua_State* L) {
    lua::Push(L, ctx_->Calls()->total_time_seconds());
    return 1;
  }

  lua::NResultsOr TempFolder(lua_State* L) {
    lua::Push(L, ctx_->TempFolder());
    return 1;
  }

  lua::NResultsOr ExecutableRunfiles(lua_State* L) {
    lua::Push(L, ctx_->ExecutableRunfiles());
    return 1;
  }

  lua::NResultsOr LoadFileToString(lua_State* L) {
    std::string file_name;
    if (!lua::Read(L, -1, &file_name)) {
      return "Must supply file name.";
    }
    if (ctx_->FileReaderOverride()) {
      size_t size = 0;
      char* buff = nullptr;
      if (!ctx_->FileReaderOverride()(file_name.c_str(), &buff, &size)) {
        return "File not found!";
      }
      lua_pushlstring(L, buff, size);
      free(buff);
    } else {
      std::string contents;
      if (!util::GetContents(file_name, &contents)) {
        return "File not found!";
      }
      lua::Push(L, contents);
    }
    return 1;
  }

  lua::NResultsOr LoadFileToByteTensor(lua_State* L) {
    std::string file_name;
    if (!lua::Read(L, 2, &file_name)) {
      return "Must supply file name.";
    }

    if (ctx_->FileReaderOverride()) {
      size_t size = 0;
      char* buff = nullptr;
      if (!ctx_->FileReaderOverride()(file_name.c_str(), &buff, &size)) {
        return "File not found!";
      }

      auto storage = std::make_shared<StorageFreeChar>(buff);
      tensor::TensorView<unsigned char> tensor_view(tensor::Layout({size}),
                                                    storage->data());
      tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(tensor_view),
                                                     std::move(storage));
    } else {
      std::string data;
      if (!util::GetContents(file_name, &data)) {
        return "File not found!";
      }
      auto size = data.size();
      auto storage = std::make_shared<StorageString>(std::move(data));
      tensor::TensorView<unsigned char> tensor_view(tensor::Layout({size}),
                                                    storage->data());

      tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(tensor_view),
                                                     std::move(storage));
    }
    return 1;
  }

  lua::NResultsOr CopyFileToLocation(lua_State* L) {
    std::string file_name_from, file_name_to;
    if (!lua::Read(L, 2, &file_name_from)) {
      return "Must supply from file name.";
    }
    if (!lua::Read(L, 3, &file_name_to)) {
      return "Must supply from file name to.";
    }

    if (ctx_->FileReaderOverride()) {
      size_t size = 0;
      char* buff = nullptr;
      if (!ctx_->FileReaderOverride()(file_name_from.c_str(), &buff, &size)) {
        return "File not found!";
      }
      bool success =
          util::SetContents(file_name_to, absl::string_view(buff, size));
      free(buff);
      if (!success) {
        return "Failed to write file";
      }
    } else {
      std::string contents;
      if (!util::GetContents(file_name_from, &contents)) {
        return "File not found!";
      }
      bool success = util::SetContents(file_name_to, contents);
      if (!success) {
        return "Failed to write file";
      }
    }
    return 1;
  }

  ContextGame* ctx_;
};

// Returns the unique value in the range [-180, 180) that is equivalent to
// 'angle', where two values x and y are considered equivalent whenever x - y
// is an integral multiple of 360. (Note: the result may be  meaningless if the
// magnitude of 'angle' is very large.)
double CanonicalAngle360(double angle) {
  const double n = std::floor((angle + 180.0) * (1.0 / 360.0));
  return angle - n * 360.0;
}

}  // namespace

int ContextGame::Init() {
  temp_folder_owned_ = temp_folder_.empty();
  if (temp_folder_owned_) {
    temp_folder_ = util::GetTempDirectory() + "/dmlab_temp_folder_XXXXXX";
    char* temp_folder_result = mkdtemp(&temp_folder_[0]);
    if (temp_folder_result == nullptr) {
      std::cerr << "Failed to create temp folder\n";
      return 1;
    }
  }
  return 0;
}

ContextGame::~ContextGame() {
  if (!temp_folder_.empty() && temp_folder_owned_) {
    util::RemoveDirectory(temp_folder_);
  }
}

lua::NResultsOr ContextGame::Module(lua_State* L) {
  if (auto* ctx =
          static_cast<ContextGame*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaGameModule::Register(L);
    LuaGameModule::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing context!";
  }
}

void ContextGame::SetPlayerState(const float pos[3], const float vel[3],
                                 const float angles[3], float height,
                                 int team_score, int other_team_score,
                                 int player_id, int timestamp_msec) {
  PlayerView before = player_view_;
  std::copy_n(pos, 3, player_view_.pos.begin());
  std::copy_n(vel, 3, player_view_.vel.begin());
  std::copy_n(angles, 3, player_view_.angles.begin());
  player_view_.height = height;
  player_view_.timestamp_msec = timestamp_msec;
  player_view_.player_id = player_id;
  player_view_.team_score = team_score;
  player_view_.other_team_score = other_team_score;
  int delta_time_msec = player_view_.timestamp_msec - before.timestamp_msec;

  // When delta_time_msec < 3 the velocities become inaccurate.
  if (before.timestamp_msec > 0 && delta_time_msec > 0) {
    double inv_delta_time = 1000.0 / delta_time_msec;
    for (int i : {0, 1, 2}) {
      player_view_.anglesVel[i] =
          CanonicalAngle360(player_view_.angles[i] - before.angles[i]) *
          inv_delta_time;
    }
  } else {
    player_view_.anglesVel.fill(0);
  }
}

void ContextGame::GetCustomView(int* width, int* height, float position[3],
                                float view_angles[3],
                                bool* render_player) const {
  std::copy_n(camera_position_.data(), 3, position);
  std::copy_n(camera_view_angles_.data(), 3, view_angles);
  *width = camera_width_;
  *height = camera_height_;
  *render_player = camera_render_player_;
}

void ContextGame::SetCustomView(int width, int height,
                                const std::array<float, 3>& pos,
                                const std::array<float, 3>& eye,
                                bool render_player) {
  camera_height_ = height;
  camera_width_ = width;
  camera_position_ = pos;
  camera_view_angles_ = eye;
  camera_render_player_ = render_player;
}

}  // namespace lab
}  // namespace deepmind
