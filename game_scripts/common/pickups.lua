local pickups = {}

-- Must match itemType_t in engine/code/game/bg_public.h.
pickups.type = {
    kInvalid = 0,
    kWeapon = 1,
    kAmmo = 2,
    kArmor = 3,
    kHealth = 4,
    kPowerUp = 5,
    kHoldable = 6,
    kPersistant_PowerUp = 7,
    kTeam = 8,
    kReward = 9,
    kGoal = 10
}

-- Must match reward_mv_t in engine/code/game/bg_public.h.
pickups.move_type = {
    kBob = 0,
    kStatic = 1
}

pickups.defaults = {
  apple_reward = {
      name = 'Apple',
      classname = 'apple_reward',
      model = 'models/apple.md3',
      quantity = 1,
      type = pickups.type.kReward
  },
  lemon_reward = {
      name = 'Lemon',
      classname = 'lemon_reward',
      model = 'models/lemon.md3',
      quantity = -1,
      type = pickups.type.kReward
  },
  strawberry_reward = {
      name = 'Strawberry',
      classname = 'strawberry_reward',
      model = 'models/strawberry.md3',
      quantity = 2,
      type = pickups.type.kReward
  },
  fungi_reward = {
      name = 'Fungi',
      classname = 'fungi_reward',
      model = 'models/toadstool.md3',
      quantity = -10,
      type = pickups.type.kReward
  },
  watermelon_goal = {
      name = 'Watermelon',
      classname = 'watermelon_goal',
      model = 'models/watermelon.md3',
      quantity = 20,
      type = pickups.type.kGoal
  },
  goal = {
      name = 'Goal',
      classname = 'goal',
      model = 'models/goal_object_02.md3',
      quantity = 10,
      type = pickups.type.kGoal
  },
  goal_mango = {
      name = 'Mango',
      classname = 'goal',
      model = 'models/mango.md3',
      quantity = 100,
      type = pickups.type.kGoal
  }
}

return pickups
