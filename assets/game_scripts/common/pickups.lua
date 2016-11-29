local pickups = {}

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

pickups.defaults = {
  apple_reward = {
      name = 'Apple',
      class_name = 'apple_reward',
      model_name = 'models/apple.md3',
      quantity = 1,
      type = pickups.type.kReward
  },
  lemon_reward = {
      name = 'Lemon',
      class_name = 'lemon_reward',
      model_name = 'models/lemon.md3',
      quantity = -1,
      type = pickups.type.kReward
  },
  strawberry_reward = {
      name = 'Strawberry',
      class_name = 'strawberry_reward',
      model_name = 'models/strawberry.md3',
      quantity = 2,
      type = pickups.type.kReward
  },
  fungi_reward = {
      name = 'Fungi',
      class_name = 'fungi_reward',
      model_name = 'models/toadstool.md3',
      quantity = -10,
      type = pickups.type.kReward
  },
  watermelon_goal = {
      name = 'Watermelon',
      class_name = 'watermelon_goal',
      model_name = 'models/watermelon.md3',
      quantity = 20,
      type = pickups.type.kGoal
  },
  goal = {
      name = 'Goal',
      class_name = 'goal',
      model_name = 'models/goal_object_02.md3',
      quantity = 10,
      type = pickups.type.kGoal
  },
  goal_mango = {
      name = 'Mango',
      class_name = 'goal',
      model_name = 'models/mango.md3',
      quantity = 100,
      type = pickups.type.kGoal
  }
}

return pickups
