import numpy as np
import deepmind_lab

print("Creating DeepMind Lab environment...")

env = deepmind_lab.Lab(
    "lt_chasm",
    ["RGB_INTERLEAVED"],
    config={"width": "320", "height": "240"}
)

try:
    print("Resetting environment...")
    env.reset(seed=7)

    for i in range(10):
        observation = env.observations()
        action = np.zeros(7, dtype=np.intc)
        reward = env.step(action, num_steps=4)

        rgb = observation["RGB_INTERLEAVED"]

        print(
            f"Step {i + 1}: "
            f"observation shape={rgb.shape}, "
            f"dtype={rgb.dtype}, reward={reward}"
        )

    print("Environment test PASSED!")

finally:
    env.close()
