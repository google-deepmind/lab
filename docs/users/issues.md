# Known Issues

Please take note of the following subtleties when modifying *DeepMind Lab* or
adapting it to different platforms.

## Scripting

* Server and client run at different frame rates. This causes a mismatch when
  rewards that are given by scripts are eventually received by players.
* The script supports fractional rewards, but the engine does not. This means
  that only integer changes in reward are consumed by the agent.
* The script API is currently designed to work with single-player levels.

## Compiling

* The LCC bytecode compiler has its own pointer representation (32 bits), which
  may differ from the host's pointer representation. Arguments of trap calls
  need to be translated properly at the VM's syscall boundary.
* The order in which `.asm` files are linked into `.qvm` bytecode libraries is
  important. The current order is taken from ioq3's original `Makefile`.
* OSMesa must be compiled without thread-local storage (TLS) if the environment
  is to be loaded from a shared library repeatedly (e.g. via `dlopen`).
* Loading *DeepMind Lab* repeatedly from a shared library may be the only way to
  reliably instantiate multiple copies of the environment in the same process,
  since the original ioq3 game engine was not designed as a library and is
  effectively not reentrant (e.g. it has a lot of global state and global
  initialization).
* The SDL library cannot be linked statically, since its sound subsystem does
  not shut down cleanly. Linking SDL from a shared object works (as we do for
  the freestanding game binary), as does disabling sound altogether (as we do
  for the library version).

## Threading

The environments are thread compatible but not thread safe. A call to one
environment will not interfere with a call to another but calls to the same
environment from different threads at the same time may interfere.

Individual render modes have their own special exceptions:

*   Software rendering - The environment is fully thread compatible.
*   Hardware rendering - The environment can be only called from the *same*
    thread in which it was constructed.
*   SDL rendering - The environment can only be called from the main thread.

## Randomness and procedural generation

The procedural generation algorithms are not portably deterministic. Different
platforms may produce different random mazes even when given the same random
seed. In particular, some unit tests that compare the result with a hard-coded
expected output may fail on some platforms. (The hardcoded output was produced
by GCC using libstd++ at version 4.9.)
