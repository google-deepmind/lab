#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_LEVEL_SETTINGS_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_LEVEL_SETTINGS_H_

#include <memory>
#include <string>
#include <vector>

#include "Eigen/Core"
#include "deepmind/level_generation/map_builder/builder.h"

namespace deepmind {
namespace lab {

class Theme {
 public:
  enum class Direction {
    North,
    East,
    South,
    West,
  };

  struct WallArtLocation {
    // Corner placement of wall art.
    Eigen::Vector3d top_left;

    // Corner placement of wall art.
    Eigen::Vector3d bottom_right;

    // Normal of the wall the texture is applied to.
    Eigen::Vector3d interior_direction;

    // Cell to place art.
    Eigen::Vector2i cell;

    // Variation the wall pointing at.
    int variation;

    // Closest direction the interior_direction is pointing to.
    Direction direction;
  };

  struct FloorArtLocation {
    // Floor position to place art.
    Eigen::Vector3d location;

    // Floor cell to place art.
    Eigen::Vector2i cell;

    // Variation of cell at location.
    int variation;
  };

  struct Model {
    // Path to an md3 model relative to the assets directory.
    std::string name;

    // Scale of the model relative to design scale.
    double scale;

    // Angle in degrees to place the model.
    double angle;
  };

  struct FloorDecoration {
    FloorArtLocation location;
    Model model;
  };

  struct Texture {
    // Path to texture relative to the assets directory.
    std::string name;

    // Shape of the source texture in pixels. This ensures the texture matches
    // the brush or patch is being applied to.
    int width;
    int height;

    // Uniformily scale the texture. Set to 1.0 for default scaling.
    double scale;

    // Angle in degrees to place texture on brush or patches.
    double angle;
  };

  struct WallDecoration {
    WallArtLocation location;
    Texture texture;
  };

  virtual ~Theme() = default;

  // Called when generating walls for a level. Returns a wall texture for that
  // variation, direction combination.
  virtual Texture wall(int variation, Direction direction) = 0;

  // Called when generating floors for a level. Returns a floor texture for that
  // variation, direction combination.
  virtual Texture floor(int variation) = 0;

  // Called when generating ceiling for a level. Returns a ceiling texture for
  // that variation, direction combination.
  virtual Texture ceiling(int variation) = 0;

  // Platforms
  //     floor ^
  //  +-----------+  > floor. (One unit high. Thin)
  //  |           |  > platform_tread.
  //  +---------- +
  //  |           |  > platform_riser.
  //  |           |
  //  |           |
  //  |           |
  //  +---------- +
  // Called when generating platforms for a level. Returns a wall texture for
  // that variation, direction combination.
  virtual Texture platform_riser() = 0;

  // Called once for each variation used in the level being generated. Return an
  // appropiate texture for that location.
  virtual Texture platform_tread() = 0;

  virtual std::vector<WallDecoration> WallDecorations(
      const std::vector<WallArtLocation>& wall_locations) = 0;
  virtual std::vector<FloorDecoration> FloorDecorations(
      const std::vector<FloorArtLocation>& floor_locations) = 0;
};

class NullTheme : public Theme {
 public:
  Texture floor(int variation) override {
    return {"map/lab_games/lg_style_01_floor_orange", 1024, 1024, 1.0, 0.0};
  }

  Texture wall(int variation, Direction direction) override {
    return {"map/lab_games/lg_style_01_wall_green", 1024, 1024, 1.0, 0.0};
  }

  Texture ceiling(int variation) override {
    return {"map/lab_games/fake_sky", 1024, 1024, 1.0, 0.0};
  }

  Texture platform_riser() override {
    return {"map/lab_games/lg_style_02_wall_blue", 1024, 1024, 1.0, 0.0};
  }

  Texture platform_tread() override {
    return {"map/black_d", 64, 64, 1.0, 0.0};
  }

  std::vector<WallDecoration> WallDecorations(
      const std::vector<WallArtLocation>& wall_locations) override {
    std::vector<WallDecoration> decs;
    decs.reserve(wall_locations.size());
    for (const auto& location : wall_locations) {
      WallDecoration dec;
      dec.location = location;
      dec.texture =
          Texture{"decal/lab_games/dec_img_style01_001", 1024, 1024, 1.0, 90.0};
      decs.push_back(std::move(dec));
    }
    return decs;
  }

  std::vector<FloorDecoration> FloorDecorations(
      const std::vector<FloorArtLocation>& floor_locations) override {
    std::vector<FloorDecoration> decs;
    decs.reserve(floor_locations.size());
    for (const auto& location : floor_locations) {
      FloorDecoration dec;
      dec.location = location;
      dec.model = Model{"models/hr_tv.md3", 1.0, 0.0};
      decs.push_back(std::move(dec));
    }
    decs.clear();
    return decs;
  }
};

struct TextLevelSettings {
  TextLevelSettings()
      : theme(new NullTheme),
        wall_decal_frequency(0.1),
        floor_object_frequency(0.05),
        cell_size(100.0 / map_builder::kWorldToGameUnits),
        ceiling_scale(1.0),
        light_intensity(1.0),
        ceiling_height(1.0),
        draw_default_layout(true) {}

  std::unique_ptr<Theme> theme;
  std::string skybox_texture_name;
  double wall_decal_frequency;
  double floor_object_frequency;
  double cell_size;
  double ceiling_scale;
  double light_intensity;
  // Decides the height of the maze bounding box, 1.0 by default, can be set
  // from lua side for the multi-floor levels, e.g. platform levels.
  double ceiling_height;
  // Decides whether to draw the default walls and floors, true by default, need
  // to be switch off for platform levels.
  bool draw_default_layout;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_LEVEL_SETTINGS_H_
