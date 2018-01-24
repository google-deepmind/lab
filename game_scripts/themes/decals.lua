local NUMBER_OF_STYLES = 4
local NUMBER_OF_IMAGES_PER_STYLE = 20
local DEFAULT_WALL_DECALS = {}
local DEFAULT_WALL_IMAGES = {}
for i = 1, NUMBER_OF_STYLES do
  for j = 1, NUMBER_OF_IMAGES_PER_STYLE do
    local img = string.format('decal/lab_games/dec_img_style%02d_%03d', i, j)
    DEFAULT_WALL_DECALS[#DEFAULT_WALL_DECALS + 1] = {
        tex = img .. '_nonsolid'
    }
    DEFAULT_WALL_IMAGES[#DEFAULT_WALL_IMAGES + 1] = 'textures/' .. img
  end
end

return {
  decals = DEFAULT_WALL_DECALS,
  images = DEFAULT_WALL_IMAGES,
}
