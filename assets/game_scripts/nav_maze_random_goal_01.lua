local factory = require 'factories.random_goal_factory'

local entityLayer = [[
*********************
*   *   *           *
* * * ***** ***** * *
* * * *   *     * * *
* * * *   *     * * *
* *   *   *     * * *
* *****   * *   * * *
*     *   * *   * * *
***** * *********** *
*                   *
*********************
]]

return factory.createLevelApi{
    mapName = 'nav_maze_random_goal_01',
    entityLayer = entityLayer,
    episodeLengthSeconds = 60
}
