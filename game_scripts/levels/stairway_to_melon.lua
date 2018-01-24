local factory = require 'factories.seek_avoid_factory'

return factory.createLevelApi{
    mapName = 'stairway_to_melon',
    episodeLengthSeconds = 60
}
