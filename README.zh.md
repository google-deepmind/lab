# <img src="/docs/template/logo.png" alt="DeepMind Lab">

<p align="center">
  <a href="README.md">English</a> · <b>简体中文</b>
</p>

*DeepMind Lab* 是一个基于 id Software 的 [Quake III Arena](https://github.com/id-Software/Quake-III-Arena)（通过 [ioquake3](https://github.com/ioquake/ioq3) 及[其他开源软件](#upstream-sources)）构建的 3D 学习与研究环境。

<div align="center">
  <a href="https://www.youtube.com/watch?v=M40rN7afngY" target="_blank">
    <img src="http://img.youtube.com/vi/M40rN7afngY/0.jpg"
         alt="DeepMind Lab - Nav Maze Level 1"
         width="240" height="180" border="10" />
  </a>
  <a href="https://www.youtube.com/watch?v=gC_e8AHzvOw" target="_blank">
    <img src="http://img.youtube.com/vi/gC_e8AHzvOw/0.jpg"
         alt="DeepMind Lab - Stairway to Melon Level"
         width="240" height="180" border="10" />
  </a>
  <a href="https://www.youtube.com/watch?v=7syZ42HWhHE" target="_blank">
    <img src="http://img.youtube.com/vi/7syZ42HWhHE/0.jpg"
         alt="DeepMind Lab - Laser Tag Space Bounce Level (Hard)"
         width="240" height="180" border="10" />
  </a>
  <br /><br />
</div>

*DeepMind Lab* 为学习型智能体（Learning Agents）提供了一套极具挑战性的 3D 导航与解谜任务套件。其核心目标是作为人工智能研究的试验平台，特别是针对深度强化学习（Deep Reinforcement Learning）领域。

## 关于 (About)

免责声明：这不是 Google 的官方产品。

如果您在学术研究中使用 *DeepMind Lab* 并希望引用该环境，建议引用 [DeepMind Lab 论文](https://arxiv.org/abs/1612.03801)。

您可以通过电子邮箱 [lab@deepmind.com](mailto:lab@deepmind.com) 与我们取得联系。

## 在 Linux 上快速上手 (Getting started on Linux)

* 从 [bazel.io 获取 Bazel](https://docs.bazel.build/versions/master/install.html)。

* 克隆 DeepMind Lab 仓库，例如执行：

```shell
$ git clone https://github.com/deepmind/lab
$ cd lab
```

运行随机智能体的实时示例：

```shell
lab$ bazel run :python_random_agent --define graphics=sdl -- \
               --length=10000 --width=640 --height=480
```

此处提供了[更详尽的构建文档](/docs/users/build.md)，其中包含在缺少依赖时的安装指引。

若要开启编译器优化，请为各个 `bazel build`、`bazel test` 与 `bazel run` 命令添加 `--compilation_mode=opt` 标志（简写为 `-c opt`）。为保持简洁，本文示例中省略了该标志，但在关注性能的实际训练与评估场景中应始终启用。

### 以人类玩家身份游玩 (Play as a human)

若要使用人类输入控件测试游戏，请运行：

```shell
lab$ bazel run :game -- --level_script=tests/empty_room_test --level_setting=logToStdErr=true
# 或者：
lab$ bazel run :game -- -l tests/empty_room_test -s logToStdErr=true
```

保持 `logToStdErr` 设置为关闭状态以禁用绝大部分日志输出。

通过为关注的观测项添加 `--observation OBSERVATION_NAME` 标志，可以在每一步输出环境暴露的观测值：

```shell
lab$ bazel run :game -- --level_script=lt_chasm --observation VEL.TRANS --observation VEL.ROT
```

### 训练智能体 (Train an agent)

*DeepMind Lab* 在 [`python/random_agent.py`](python/random_agent.py) 中附带了一个示例随机智能体，可作为实现自主学习智能体的起点。若要让该智能体与 DeepMind Lab 交互以开展训练，请运行：

```shell
lab$ bazel run :python_random_agent
```

智能体与环境之间的交互使用 [Python API](/docs/users/python_api.md)。我们还提供了针对 DeepMind 通用强化学习接口 "[dm_env](https://github.com/deepmind/dm_env)" 的绑定，以及构建独立 PIP 安装包的方法；详情请参阅[独立文档](python/pip_package/README.md)。

*DeepMind Lab* 内置了实现不同任务的[多种关卡](/docs/levels.md)。这些任务可以通过 Lua 脚本进行配置，具体说明请参阅 [Lua API](/docs/developers/reference/lua_api.md)。

-----------------

## 上游开源软件源 (Upstream sources)

*DeepMind Lab* 基于 *ioquake3* 游戏引擎构建，并使用 *q3map2* 和 *bspc* 工具进行地图创建。源自这些项目的缺陷修复与代码清理建议优先在上游项目中修复，然后再合并回 *DeepMind Lab*。

* *bspc* 取自 [github.com/TTimo/bspc](https://github.com/TTimo/bspc)，提交版本为 d9a372db3fb6163bc49ead41c76c801a3d14cf80。此处几乎没有任何本地改动，但我们将该代码与 ioq3 主体代码进行了整合，未直接使用其 `deps` 目录中的副本。该部分代码预计保持稳定。

* *q3map2* 取自 [github.com/TTimo/GtkRadiant](https://github.com/TTimo/GtkRadiant)，提交版本为 d3d00345c542c8d7cc74e2e8a577bdf76f79c701。进行了少量本地修改以增加同步机制。该部分代码预计同样保持稳定。

* *ioquake3* 取自 [github.com/ioquake/ioq3](https://github.com/ioquake/ioq3)，提交版本为 29db64070aa0bae49953bddbedbed5e317af48ba。该代码包含了大量修改与新增功能。我们的目标是不定期合并上游的变更。

我们非常感谢这些开源仓库的维护者为维护高质量代码库所付出的辛勤努力。

## 外部依赖、先决条件与移植说明 (External dependencies, prerequisites and porting notes)

*DeepMind Lab* 目前仅以源代码形式分发。它依赖若干外部软件库，我们以多种不同方式提供这些依赖：

 * `zlib`、`glib`、`libxml2`、`jpeg` 和 `png` 库作为外部 Bazel 源进行引用，并提供了相应的 Bazel BUILD 文件。依赖代码本身具备良好的可移植性，但我们提供的 BUILD 规则专门针对 Linux x86 架构。若在其他平台上进行构建，通常需要对这些 BUILD 文件进行修改。

 * 本代码包内包含了消息摘要算法（位于 [`//third_party/md`](third_party/md)），均取自各自对应 RFC 的参考实现。[`//third_party/rl_api`](third_party/rl_api) 中包含了一个“通用强化学习 API（generic reinforcement learning API）”，该 API 同样由 *DeepMind Lab* 作者创建。该部分代码具备良好的可移植性。

 * 本代码包内包含了 EGL 头文件（位于 `//third_party/GL/{`[`EGL`](third_party/GL/EGL)`,`[`KHR`](third_party/GL/KHR)`}`），取自 Khronos OpenGL/OpenGL ES XML API Registry（位于 [www.khronos.org/registry/EGL](http://www.khronos.org/registry/EGL/)）。头文件经过微调以移除 EGL 对 X 的依赖。

 * 运行还需要若干其他库，这些库未随本仓库打包，必须预先安装在您的系统中：
   * SDL 2
   * gettext（`glib` 所需依赖）
   * OpenGL：人类交互游玩需要硬件驱动程序与库以支持硬件加速。机器学习智能体所使用的无头（headless）库可以根据 `--define headless=...` 构建设置，选择通过 EGL 或 GLX 进行硬件加速渲染，或通过 OSMesa 进行软件渲染。
   * Python 2.7（其他版本可能亦可运行）并配备 NumPy、PIL（少数测试要求 NumPy 版本不低于 1.8），或 Python 3（不低于 3.5）并配备 NumPy 和 Pillow。

构建规则使用了一些特定于 GCC 的编译器选项。如果您的编译器无法识别某些标志（通常为特定警告抑制标志），您可能需要修改这些标志。这类编译器警告通常较为嘈杂，但并不影响正常构建。

---

> **翻译说明（Translation Disclaimer）**
> 本文档由社区志愿者（[@JasonYeYuhe](https://github.com/JasonYeYuhe)）人工翻译并维护，旨在降低中文开发者的阅读门槛。最后同步日期：2026年10月18日。若译文与英文原版存在任何出入或滞后，请以 [英文原版 (README.md)](README.md) 为准。欢迎通过 PR 协助改进译文！
