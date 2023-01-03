# Description:
#   DeepMind Lab, a machine-learning research environment
#   forked from ioquake/ioq3.

licenses(["restricted"])

exports_files(["LICENSE"])

config_setting(
    name = "is_linux",
    constraint_values = ["@platforms//os:linux"],
)

config_setting(
    name = "is_macos",
    constraint_values = ["@platforms//os:macos"],
)

CODE_DIR = "engine/code"

ARCH_VAR = "-DARCH_STRING=\\\"x86_64\\\""

STANDALONE_VAR = "-DSTANDALONE"

cc_library(
    name = "qcommon_hdrs",
    hdrs = [
        CODE_DIR + "/qcommon/json.h",
        CODE_DIR + "/qcommon/puff.h",
        CODE_DIR + "/qcommon/q_platform.h",
        CODE_DIR + "/qcommon/q_shared.h",
        CODE_DIR + "/qcommon/surfaceflags.h",
    ],
    copts = [
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    textual_hdrs = [
        CODE_DIR + "/qcommon/cm_polylib.h",
        CODE_DIR + "/qcommon/cm_public.h",
        CODE_DIR + "/qcommon/qcommon.h",
        CODE_DIR + "/qcommon/qfiles.h",
    ],
)

cc_binary(
    name = "q3asm",
    srcs = [
        CODE_DIR + "/tools/asm/cmdlib.c",
        CODE_DIR + "/tools/asm/cmdlib.h",
        CODE_DIR + "/tools/asm/mathlib.h",
        CODE_DIR + "/tools/asm/opstrings.h",
        CODE_DIR + "/tools/asm/q3asm.c",
    ],
    copts = [
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    deps = [":qcommon_hdrs"],
)

cc_binary(
    name = "lburg",
    srcs = glob(
        [
            CODE_DIR + "/tools/lcc/lburg/*.c",
            CODE_DIR + "/tools/lcc/lburg/*.h",
        ],
    ),
)

genrule(
    name = "lcc_dagcheck",
    srcs = [CODE_DIR + "/tools/lcc/src/dagcheck.md"],
    outs = [CODE_DIR + "/tools/lcc/src/dagcheck.c"],
    cmd = "$(location lburg) $< > \"$@\"",
    tools = [":lburg"],
)

cc_binary(
    name = "q3rcc",
    srcs = [
        CODE_DIR + "/tools/lcc/src/dagcheck.c",
        CODE_DIR + "/tools/lcc/src/c.h",
        CODE_DIR + "/tools/lcc/src/config.h",
        CODE_DIR + "/tools/lcc/src/token.h",
    ] + glob([CODE_DIR + "/tools/lcc/src/*.c"]),
    copts = [
        "-I" + CODE_DIR + "/tools/lcc/src",
        "-w",
    ],
)

cc_binary(
    name = "q3cpp",
    srcs = glob(
        [
            CODE_DIR + "/tools/lcc/cpp/*.c",
            CODE_DIR + "/tools/lcc/cpp/*.h",
        ],
    ),
    copts = ["-Wno-unused-result"],
)

cc_binary(
    name = "q3lcc",
    srcs = [
        CODE_DIR + "/tools/lcc/etc/bytecode.c",
        CODE_DIR + "/tools/lcc/etc/lcc.c",
    ],
    copts = [ARCH_VAR],
    deps = [":qcommon_hdrs"],
)

ASM_SOURCES = [
    "cgame/cg_main",
    "cgame/cg_consolecmds",
    "cgame/cg_draw",
    "cgame/cg_drawtools",
    "cgame/cg_effects",
    "cgame/cg_ents",
    "cgame/cg_event",
    "cgame/cg_info",
    "cgame/cg_localents",
    "cgame/cg_marks",
    "cgame/cg_particles",
    "cgame/cg_players",
    "cgame/cg_playerstate",
    "cgame/cg_predict",
    "cgame/cg_scoreboard",
    "cgame/cg_servercmds",
    "cgame/cg_snapshot",
    "cgame/cg_view",
    "cgame/cg_weapons",
    "game/g_main",
    "game/ai_chat",
    "game/ai_cmd",
    "game/ai_dmnet",
    "game/ai_dmq3",
    "game/ai_main",
    "game/ai_team",
    "game/ai_vcmd",
    "game/bg_lib",
    "game/bg_misc",
    "game/bg_pmove",
    "game/bg_slidemove",
    "game/g_active",
    "game/g_arenas",
    "game/g_bot",
    "game/g_client",
    "game/g_cmds",
    "game/g_combat",
    "game/g_items",
    "game/g_mem",
    "game/g_misc",
    "game/g_missile",
    "game/g_mover",
    "game/g_session",
    "game/g_spawn",
    "game/g_svcmds",
    "game/g_target",
    "game/g_team",
    "game/g_trigger",
    "game/g_utils",
    "game/g_weapon",
    "q3_ui/ui_addbots",
    "q3_ui/ui_atoms",
    "q3_ui/ui_cdkey",
    "q3_ui/ui_cinematics",
    "q3_ui/ui_confirm",
    "q3_ui/ui_connect",
    "q3_ui/ui_controls2",
    "q3_ui/ui_credits",
    "q3_ui/ui_demo2",
    "q3_ui/ui_display",
    "q3_ui/ui_gameinfo",
    "q3_ui/ui_ingame",
    "q3_ui/ui_loadconfig",
    "q3_ui/ui_main",
    "q3_ui/ui_menu",
    "q3_ui/ui_mfield",
    "q3_ui/ui_mods",
    "q3_ui/ui_network",
    "q3_ui/ui_options",
    "q3_ui/ui_playermodel",
    "q3_ui/ui_players",
    "q3_ui/ui_playersettings",
    "q3_ui/ui_preferences",
    "q3_ui/ui_qmenu",
    "q3_ui/ui_removebots",
    "q3_ui/ui_saveconfig",
    "q3_ui/ui_serverinfo",
    "q3_ui/ui_servers2",
    "q3_ui/ui_setup",
    "q3_ui/ui_sound",
    "q3_ui/ui_sparena",
    "q3_ui/ui_specifyserver",
    "q3_ui/ui_splevel",
    "q3_ui/ui_sppostgame",
    "q3_ui/ui_spreset",
    "q3_ui/ui_spskill",
    "q3_ui/ui_startserver",
    "q3_ui/ui_team",
    "q3_ui/ui_teamorders",
    "q3_ui/ui_video",
    "qcommon/q_math",
    "qcommon/q_shared",
    "ui/ui_main",
    "ui/ui_atoms",
    "ui/ui_gameinfo",
    "ui/ui_players",
    "ui/ui_shared",
    "deepmind/dm_local",
]

filegroup(
    name = "qvm_src",
    srcs = [CODE_DIR + "/" + asm_source + ".c" for asm_source in ASM_SOURCES],
)

filegroup(
    name = "qvm_headers",
    srcs = [
        "engine/ui/menudef.h",
        CODE_DIR + "/deepmind/dm_local.h",
        CODE_DIR + "/deepmind/dm_public.h",
    ] + glob([
        CODE_DIR + "/botlib/*.h",
        CODE_DIR + "/cgame/*.h",
        CODE_DIR + "/client/*.h",
        CODE_DIR + "/game/*.h",
        CODE_DIR + "/q3_ui/*.h",
        CODE_DIR + "/qcommon/*.h",
        CODE_DIR + "/renderercommon/*.h",
        CODE_DIR + "/ui/*.h",
    ]),
)

genrule(
    name = "qvm_asms",
    srcs = [
        ":qvm_headers",
        ":qvm_src",
    ],
    outs = [CODE_DIR + "/" + asm_source + ".asm" for asm_source in ASM_SOURCES],
    cmd = "for FILE in $(locations :qvm_src); do " +
          "  $(location :q3lcc) -DSTANDALONE \"$$FILE\" -o \"$(@D)/$${FILE/.c/.asm}\"; " +
          "done",
    tools = [
        ":q3cpp",
        ":q3lcc",
        ":q3rcc",
    ],
)

CGAME_QVM_ASMS = [
    "cgame/cg_main",
    "game/bg_misc",
    "game/bg_pmove",
    "game/bg_slidemove",
    "game/bg_lib",
    "cgame/cg_consolecmds",
    "cgame/cg_draw",
    "cgame/cg_drawtools",
    "cgame/cg_effects",
    "cgame/cg_ents",
    "cgame/cg_event",
    "cgame/cg_info",
    "cgame/cg_localents",
    "cgame/cg_marks",
    "cgame/cg_particles",
    "cgame/cg_players",
    "cgame/cg_playerstate",
    "cgame/cg_predict",
    "cgame/cg_scoreboard",
    "cgame/cg_servercmds",
    "cgame/cg_snapshot",
    "cgame/cg_view",
    "cgame/cg_weapons",
    "qcommon/q_math",
    "qcommon/q_shared",
    "deepmind/dm_local",
    "cgame/cg_syscalls",
]

QAGAME_QVM_ASMS = [
    "game/g_main",
    "game/ai_chat",
    "game/ai_cmd",
    "game/ai_dmnet",
    "game/ai_dmq3",
    "game/ai_main",
    "game/ai_team",
    "game/ai_vcmd",
    "game/bg_misc",
    "game/bg_pmove",
    "game/bg_slidemove",
    "game/bg_lib",
    "game/g_active",
    "game/g_arenas",
    "game/g_bot",
    "game/g_client",
    "game/g_cmds",
    "game/g_combat",
    "game/g_items",
    "game/g_mem",
    "game/g_misc",
    "game/g_missile",
    "game/g_mover",
    "game/g_session",
    "game/g_spawn",
    "game/g_svcmds",
    "game/g_target",
    "game/g_team",
    "game/g_trigger",
    "game/g_utils",
    "game/g_weapon",
    "qcommon/q_math",
    "qcommon/q_shared",
    "deepmind/dm_local",
    "game/g_syscalls",
]

Q3UI_QVM_ASMS = [
    "q3_ui/ui_main",
    "game/bg_misc",
    "game/bg_lib",
    "q3_ui/ui_addbots",
    "q3_ui/ui_atoms",
    "q3_ui/ui_cdkey",
    "q3_ui/ui_cinematics",
    "q3_ui/ui_confirm",
    "q3_ui/ui_connect",
    "q3_ui/ui_controls2",
    "q3_ui/ui_credits",
    "q3_ui/ui_demo2",
    "q3_ui/ui_display",
    "q3_ui/ui_gameinfo",
    "q3_ui/ui_ingame",
    "q3_ui/ui_loadconfig",
    "q3_ui/ui_menu",
    "q3_ui/ui_mfield",
    "q3_ui/ui_mods",
    "q3_ui/ui_network",
    "q3_ui/ui_options",
    "q3_ui/ui_playermodel",
    "q3_ui/ui_players",
    "q3_ui/ui_playersettings",
    "q3_ui/ui_preferences",
    "q3_ui/ui_qmenu",
    "q3_ui/ui_removebots",
    "q3_ui/ui_saveconfig",
    "q3_ui/ui_serverinfo",
    "q3_ui/ui_servers2",
    "q3_ui/ui_setup",
    "q3_ui/ui_sound",
    "q3_ui/ui_sparena",
    "q3_ui/ui_specifyserver",
    "q3_ui/ui_splevel",
    "q3_ui/ui_sppostgame",
    "q3_ui/ui_spskill",
    "q3_ui/ui_startserver",
    "q3_ui/ui_team",
    "q3_ui/ui_teamorders",
    "q3_ui/ui_video",
    "qcommon/q_math",
    "qcommon/q_shared",
    "deepmind/dm_local",
    "ui/ui_syscalls",
]

cc_binary(
    name = "cgamex86_64.so",
    srcs = [CODE_DIR + "/" + asm + ".c" for asm in CGAME_QVM_ASMS] + [":qvm_headers"],
    copts = [
        "-fno-strict-aliasing",
        "-DQ3_DLL",
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    linkshared = 1,
    linkstatic = 1,
)

cc_binary(
    name = "qagamex86_64.so",
    srcs = [CODE_DIR + "/" + asm + ".c" for asm in QAGAME_QVM_ASMS] + [":qvm_headers"],
    copts = [
        "-fno-strict-aliasing",
        "-Wno-error=format-zero-length",
        "-DQ3_DLL",
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    linkshared = 1,
    linkstatic = 1,
)

cc_binary(
    name = "uix86_64.so",
    srcs = [CODE_DIR + "/" + asm + ".c" for asm in Q3UI_QVM_ASMS] + [":qvm_headers"],
    copts = [
        "-DQ3_DLL",
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    linkshared = 1,
    linkstatic = 1,
)

genrule(
    name = "cgame_qvm",
    srcs = [CODE_DIR + "/" + asm + ".asm" for asm in CGAME_QVM_ASMS],
    outs = [
        CODE_DIR + "/cgame/cgame.map",
        CODE_DIR + "/cgame/cgame.qvm",
    ],
    cmd = "$(location :q3asm) -m -o \"$(location engine/code/cgame/cgame.qvm)\" $(SRCS)",
    tools = [":q3asm"],
)

genrule(
    name = "qagame_qvm",
    srcs = [CODE_DIR + "/" + asm + ".asm" for asm in QAGAME_QVM_ASMS],
    outs = [
        CODE_DIR + "/game/qagame.map",
        CODE_DIR + "/game/qagame.qvm",
    ],
    cmd = "$(location :q3asm) -m -o \"$(location engine/code/game/qagame.qvm)\" $(SRCS)",
    tools = [":q3asm"],
)

genrule(
    name = "q3ui_qvm",
    srcs = [CODE_DIR + "/" + asm + ".asm" for asm in Q3UI_QVM_ASMS],
    outs = [
        CODE_DIR + "/q3_ui/ui.map",
        CODE_DIR + "/q3_ui/ui.qvm",
    ],
    cmd = "$(location :q3asm) -m -o \"$(location engine/code/q3_ui/ui.qvm)\" $(SRCS)",
    tools = [":q3asm"],
)

cc_binary(
    name = "bspc",
    srcs = glob([
        CODE_DIR + "/botlib/*.h",
        CODE_DIR + "/qcommon/*.h",
    ]) + [
        CODE_DIR + "/botlib/be_aas_bspq3.c",
        CODE_DIR + "/botlib/be_aas_cluster.c",
        CODE_DIR + "/botlib/be_aas_move.c",
        CODE_DIR + "/botlib/be_aas_optimize.c",
        CODE_DIR + "/botlib/be_aas_reach.c",
        CODE_DIR + "/botlib/be_aas_sample.c",
        CODE_DIR + "/botlib/l_libvar.c",
        CODE_DIR + "/botlib/l_precomp.c",
        CODE_DIR + "/botlib/l_script.c",
        CODE_DIR + "/botlib/l_struct.c",
        CODE_DIR + "/bspc/_files.c",
        CODE_DIR + "/bspc/aas_areamerging.c",
        CODE_DIR + "/bspc/aas_areamerging.h",
        CODE_DIR + "/bspc/aas_cfg.c",
        CODE_DIR + "/bspc/aas_cfg.h",
        CODE_DIR + "/bspc/aas_create.c",
        CODE_DIR + "/bspc/aas_create.h",
        CODE_DIR + "/bspc/aas_edgemelting.c",
        CODE_DIR + "/bspc/aas_edgemelting.h",
        CODE_DIR + "/bspc/aas_facemerging.c",
        CODE_DIR + "/bspc/aas_facemerging.h",
        CODE_DIR + "/bspc/aas_file.c",
        CODE_DIR + "/bspc/aas_file.h",
        CODE_DIR + "/bspc/aas_gsubdiv.c",
        CODE_DIR + "/bspc/aas_gsubdiv.h",
        CODE_DIR + "/bspc/aas_map.c",
        CODE_DIR + "/bspc/aas_map.h",
        CODE_DIR + "/bspc/aas_prunenodes.c",
        CODE_DIR + "/bspc/aas_prunenodes.h",
        CODE_DIR + "/bspc/aas_store.c",
        CODE_DIR + "/bspc/aas_store.h",
        CODE_DIR + "/bspc/be_aas_bspc.c",
        CODE_DIR + "/bspc/be_aas_bspc.h",
        CODE_DIR + "/bspc/brushbsp.c",
        CODE_DIR + "/bspc/bspc.c",
        CODE_DIR + "/bspc/csg.c",
        CODE_DIR + "/bspc/glfile.c",
        CODE_DIR + "/bspc/l_bsp_ent.c",
        CODE_DIR + "/bspc/l_bsp_ent.h",
        CODE_DIR + "/bspc/l_bsp_hl.c",
        CODE_DIR + "/bspc/l_bsp_hl.h",
        CODE_DIR + "/bspc/l_bsp_q1.c",
        CODE_DIR + "/bspc/l_bsp_q1.h",
        CODE_DIR + "/bspc/l_bsp_q2.c",
        CODE_DIR + "/bspc/l_bsp_q2.h",
        CODE_DIR + "/bspc/l_bsp_q3.c",
        CODE_DIR + "/bspc/l_bsp_q3.h",
        CODE_DIR + "/bspc/l_bsp_sin.c",
        CODE_DIR + "/bspc/l_bsp_sin.h",
        CODE_DIR + "/bspc/l_cmd.c",
        CODE_DIR + "/bspc/l_cmd.h",
        CODE_DIR + "/bspc/l_log.c",
        CODE_DIR + "/bspc/l_log.h",
        CODE_DIR + "/bspc/l_math.c",
        CODE_DIR + "/bspc/l_math.h",
        CODE_DIR + "/bspc/l_mem.c",
        CODE_DIR + "/bspc/l_mem.h",
        CODE_DIR + "/bspc/l_poly.c",
        CODE_DIR + "/bspc/l_poly.h",
        CODE_DIR + "/bspc/l_qfiles.c",
        CODE_DIR + "/bspc/l_qfiles.h",
        CODE_DIR + "/bspc/l_threads.c",
        CODE_DIR + "/bspc/l_threads.h",
        CODE_DIR + "/bspc/l_utils.c",
        CODE_DIR + "/bspc/l_utils.h",
        CODE_DIR + "/bspc/leakfile.c",
        CODE_DIR + "/bspc/map.c",
        CODE_DIR + "/bspc/map_hl.c",
        CODE_DIR + "/bspc/map_q1.c",
        CODE_DIR + "/bspc/map_q2.c",
        CODE_DIR + "/bspc/map_q3.c",
        CODE_DIR + "/bspc/map_sin.c",
        CODE_DIR + "/bspc/nodraw.c",
        CODE_DIR + "/bspc/portals.c",
        CODE_DIR + "/bspc/qbsp.h",
        CODE_DIR + "/bspc/q2files.h",
        CODE_DIR + "/bspc/q3files.h",
        CODE_DIR + "/bspc/sinfiles.h",
        CODE_DIR + "/bspc/textures.c",
        CODE_DIR + "/bspc/tree.c",
        CODE_DIR + "/qcommon/cm_load.c",
        CODE_DIR + "/qcommon/cm_patch.c",
        CODE_DIR + "/qcommon/cm_test.c",
        CODE_DIR + "/qcommon/cm_trace.c",
        CODE_DIR + "/qcommon/ioapi.c",
        CODE_DIR + "/qcommon/md4.c",
        CODE_DIR + "/qcommon/unzip.c",
    ],
    copts = [
        "-I" + CODE_DIR,
        "-w",
        "-fno-common",
        ARCH_VAR,
        STANDALONE_VAR,
        "-DBSPC",
        "-DLINUX",
        "-Dstricmp=strcasecmp",
        "-DCom_Memcpy=memcpy",
        "-DCom_Memset=memset",
        "-DMAC_STATIC=",
        "-DQDECL=",
        "-pthread",
    ],
    linkopts = [
        "-pthread",
        "-lm",
    ],
    visibility = ["//deepmind/level_generation:__subpackages__"],
    deps = ["@zlib_archive//:zlib"],
)

cc_library(
    name = "level_cache_types",
    hdrs = ["public/level_cache_types.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "file_reader_types",
    hdrs = ["public/file_reader_types.h"],
    visibility = ["//visibility:public"],
)

IOQ3_RENDERERGL1_SRCS = [
    CODE_DIR + "/renderergl2/tr_image_dds.c",
] + glob(
    include = [
        CODE_DIR + "/renderergl1/*.c",
        CODE_DIR + "/renderergl1/*.h",
    ],
    exclude = [
        CODE_DIR + "/renderergl1/tr_subs.c",
    ],
)

IOQ3_COMMON_SRCS = [
    CODE_DIR + "/asm/ftola.c",
    CODE_DIR + "/asm/qasm-inline.h",
    CODE_DIR + "/asm/snapvector.c",
    CODE_DIR + "/cgame/cg_public.h",
    CODE_DIR + "/client/libmumblelink.c",
    CODE_DIR + "/deepmind/context.h",
    CODE_DIR + "/deepmind/dm_public.h",
    CODE_DIR + "/deepmind/dmlab_load_model.c",
    CODE_DIR + "/deepmind/dmlab_load_model.h",
    CODE_DIR + "/deepmind/dmlab_recording.c",
    CODE_DIR + "/deepmind/dmlab_recording.h",
    CODE_DIR + "/deepmind/dmlab_save_model.c",
    CODE_DIR + "/deepmind/dmlab_save_model.h",
    CODE_DIR + "/game/bg_public.h",
    CODE_DIR + "/game/g_public.h",
    CODE_DIR + "/sys/con_log.c",
    CODE_DIR + "/sys/con_passive.c",
    CODE_DIR + "/sys/sys_main.c",
    CODE_DIR + "/sys/sys_unix.c",
    CODE_DIR + "/ui/ui_public.h",
] + glob(
    include = [
        CODE_DIR + "/botlib/*.c",
        CODE_DIR + "/botlib/*.h",
        CODE_DIR + "/client/*.h",
        CODE_DIR + "/client/cl_*.c",
        CODE_DIR + "/client/snd_*.c",
        CODE_DIR + "/qcommon/*.c",
        CODE_DIR + "/qcommon/*.h",
        CODE_DIR + "/renderercommon/*.c",
        CODE_DIR + "/renderercommon/*.h",
        CODE_DIR + "/server/*.c",
        CODE_DIR + "/server/*.h",
        CODE_DIR + "/sys/*.h",
    ],
    exclude = [
        CODE_DIR + "/client/fx_*.h",
        CODE_DIR + "/server/sv_rankings.c",
        CODE_DIR + "/qcommon/vm_armv7l.c",
        CODE_DIR + "/qcommon/vm_none.c",
        CODE_DIR + "/qcommon/vm_powerpc*.c",
        CODE_DIR + "/qcommon/vm_powerpc_asm.h",
        CODE_DIR + "/qcommon/vm_sparc.c",
        CODE_DIR + "/qcommon/vm_sparc.h",
    ],
) + IOQ3_RENDERERGL1_SRCS

IOQ3_COMMON_DEPS = [
    ":file_reader_types",
    ":level_cache_types",
    ":qcommon_hdrs",
    "//deepmind/engine:callbacks",
    "//deepmind/engine:context",
    "//deepmind/include:context_hdrs",
    "//third_party/rl_api:env_c_api",
    "@jpeg_archive//:jpeg",
    "@sdl_system//:sdl2",
    "@zlib_archive//:zlib",
]

IOQ3_COMMON_COPTS = [
    "-std=c99",
    "-fno-common",
    "-fno-strict-aliasing",
    ARCH_VAR,
    STANDALONE_VAR,
]

IOQ3_COMMON_DEFINES = [
    "BOTLIB",
    "_GNU_SOURCE",
]

MAPS = glob(["assets/maps/src/*.map"])

# Pre-built maps are now committed to source control.
#
# Building this target :map_assets will generate PK3 files that can be unzipped
# into the# maps/built folder for adding to, or updating, the pre-built maps.
genrule(
    name = "map_assets",
    srcs = MAPS,
    outs = ["baselab/" + f[16:-3] + "pk3" for f in MAPS],
    cmd = "cp -t $(@D)/baselab $(SRCS); " +
          "for s in $(SRCS); do " +
          "  BM=$$(basename $${s}); M=$${BM/.map/}; " +
          "  if $(location //deepmind/level_generation:compile_map_sh).runfiles/org_deepmind_lab/deepmind/level_generation/compile_map_sh" +
          "      -a $(@D)/baselab/$${M} > $(@D)/out.tmp 2> $(@D)/err.tmp; then " +
          "    echo \"Built map $${M}\"; " +
          "  else " +
          "    echo -e \"Error building map $${M}:\n$$(<$(@D)/out.tmp)\n$$(<$(@D)/err.tmp)\"; " +
          "  fi; " +
          "done",
    tags = ["manual"],
    tools = [
        "//:bspc",
        "//deepmind/level_generation:compile_map_sh",
        "//q3map2",
    ],
    visibility = ["//testing:__subpackages__"],
)

ASSETS = [
    "assets/default.cfg",
    "assets/q3config.cfg",
]

genrule(
    name = "non_pk3_assets",
    srcs = ASSETS,
    outs = ["baselab/" + f[7:] for f in ASSETS],
    cmd = "for s in $(SRCS); do " +
          "  A=$$(dirname $$s); " +
          "  B=$${A/assets/}; " +
          "  mkdir -p $(@D)/baselab$${B}; " +
          "  ln -s -L -t $(@D)/baselab$${B} $$($(location //deepmind/support:realpath) $${s}); " +
          "done",
    tools = ["//deepmind/support:realpath"],
    visibility = ["//visibility:public"],
)

GAME_SCRIPT_ASSETS = glob([
    "game_scripts/**/*.lua",
    "game_scripts/**/*.png",
])

genrule(
    name = "game_script_assets",
    srcs = GAME_SCRIPT_ASSETS,
    outs = ["baselab/" + f for f in GAME_SCRIPT_ASSETS],
    cmd = "for s in $(SRCS); do " +
          "  A=$$(dirname $$s); " +
          "  mkdir -p $(@D)/baselab/$${A}; " +
          "  ln -s -L -t $(@D)/baselab/$${A} $$($(location //deepmind/support:realpath) $${s}); " +
          "done",
    tools = ["//deepmind/support:realpath"],
    visibility = ["//visibility:public"],
)

BUILT_MAPS = glob([
    "assets/maps/built/*.aas",
    "assets/maps/built/*.bsp",
])

genrule(
    name = "built_maps",
    srcs = BUILT_MAPS,
    outs = ["baselab/maps" + f[len("assets/maps/built"):] for f in BUILT_MAPS],
    cmd = "for s in $(SRCS); do " +
          "  mkdir -p $(@D)/baselab/maps; " +
          "  ln -s -L -t $(@D)/baselab/maps $$($(location //deepmind/support:realpath) $${s}); " +
          "done",
    tools = ["//deepmind/support:realpath"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "assets_oa_pk3",
    srcs = ["assets_oa/scripts/shaderlist.txt"] + glob([
        "assets_oa/scripts/**/*.shader",
    ]),
    outs = ["baselab/assets_oa.pk3"],
    cmd = "A=$$(pwd); (cd assets_oa; zip --quiet -r $${A}/$(OUTS) -- .)",
    visibility = ["//visibility:public"],
)

genrule(
    name = "assets_pk3",
    srcs = ["assets/scripts/shaderlist.txt"] + glob([
        "assets/gfx/**/*.tga",
        "assets/icons/**/*.tga",
        "assets/menu/**/*.jpg",
        "assets/menu/**/*.tga",
        "assets/models/**/*",
        "assets/textures/**/*.tga",
        "assets/scripts/**/*.shader",
    ]),
    outs = ["baselab/assets.pk3"],
    cmd = "A=$$(pwd); (cd assets; zip --quiet -r $${A}/$(OUTS) -- .)",
    visibility = ["//visibility:public"],
)

genrule(
    name = "assets_bots_pk3",
    srcs = ["assets_oa/scripts/bots.txt"] + glob(["assets_oa/botfiles/**/*"]),
    outs = ["baselab/assets_bots.pk3"],
    cmd = "A=$$(pwd); (cd assets_oa; zip --quiet -r $${A}/$(OUTS) -- .)",
    visibility = ["//visibility:public"],
)

genrule(
    name = "vm_pk3",
    srcs = [
        CODE_DIR + "/cgame/cgame.map",
        CODE_DIR + "/cgame/cgame.qvm",
        CODE_DIR + "/game/qagame.map",
        CODE_DIR + "/game/qagame.qvm",
        CODE_DIR + "/q3_ui/ui.map",
        CODE_DIR + "/q3_ui/ui.qvm",
    ],
    outs = ["baselab/vm.pk3"],
    cmd = "A=$$(pwd); mkdir $(@D)/vm; ln -s -r -t $(@D)/vm -- $(SRCS); (cd $(@D); zip --quiet -r $${A}/$(OUTS) -- vm)",
    visibility = ["//testing:__subpackages__"],
)

GAME_ASSETS = [
    ":assets_bots_pk3",
    ":assets_oa_pk3",
    ":assets_pk3",
    ":game_script_assets",
    ":non_pk3_assets",
    ":built_maps",
    ":vm_pk3",
    "//deepmind/level_generation:compile_map_sh",
]

cc_library(
    name = "game_lib_sdl",
    srcs = IOQ3_COMMON_SRCS + [
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/sdl/sdl_icon.h",
        CODE_DIR + "/sdl/sdl_input.c",
        CODE_DIR + "/sdl/sdl_snd.c",

        ## OpenGL rendering
        CODE_DIR + "/sdl/sdl_gamma.c",
        CODE_DIR + "/sdl/sdl_glimp.c",
    ],
    hdrs = ["public/dmlab.h"],
    copts = IOQ3_COMMON_COPTS,
    defines = IOQ3_COMMON_DEFINES,
    linkopts = [
        "-lGL",
        "-lrt",
    ],
    deps = IOQ3_COMMON_DEPS,
    alwayslink = 1,
)

cc_library(
    name = "game_lib_headless_osmesa",
    srcs = IOQ3_COMMON_SRCS + [
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/null/null_input.c",
        CODE_DIR + "/null/null_snddma.c",

        ## OpenGL rendering
        CODE_DIR + "/deepmind/headless_osmesa_glimp.c",
        CODE_DIR + "/deepmind/glimp_common.h",
        CODE_DIR + "/deepmind/glimp_common.c",
    ],
    hdrs = ["public/dmlab.h"],
    copts = IOQ3_COMMON_COPTS,
    defines = IOQ3_COMMON_DEFINES,
    linkopts = [
        "-lOSMesa",
        "-lrt",
    ],
    deps = IOQ3_COMMON_DEPS,
    alwayslink = 1,
)

cc_library(
    name = "game_lib_headless_glx",
    srcs = IOQ3_COMMON_SRCS + [
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/null/null_input.c",
        CODE_DIR + "/null/null_snddma.c",

        ## OpenGL rendering
        CODE_DIR + "/deepmind/headless_native_glimp.c",
        CODE_DIR + "/deepmind/glimp_common.h",
        CODE_DIR + "/deepmind/glimp_common.c",
    ],
    hdrs = ["public/dmlab.h"],
    copts = IOQ3_COMMON_COPTS,
    defines = IOQ3_COMMON_DEFINES,
    linkopts = [
        "-lGL",
        "-lX11",
    ],
    deps = IOQ3_COMMON_DEPS,
    alwayslink = 1,
)

cc_library(
    name = "game_lib_headless_egl",
    srcs = IOQ3_COMMON_SRCS + [
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/null/null_input.c",
        CODE_DIR + "/null/null_snddma.c",

        ## OpenGL rendering
        CODE_DIR + "/deepmind/headless_egl_glimp.c",
        CODE_DIR + "/deepmind/glimp_common.h",
        CODE_DIR + "/deepmind/glimp_common.c",
    ],
    hdrs = ["public/dmlab.h"],
    copts = IOQ3_COMMON_COPTS,
    defines = IOQ3_COMMON_DEFINES,
    linkopts = [
        "-lEGL",
        "-lGL",
    ],
    deps = IOQ3_COMMON_DEPS + ["//third_party/GL/util:egl_util"],
    alwayslink = 1,
)

cc_test(
    name = "dmlab_connect_osmesa_test",
    srcs = ["public/dmlab_connect_test.cc"],
    data = GAME_ASSETS,
    deps = [
        ":game_lib_headless_osmesa",
        "//deepmind/support:test_srcdir",
        "//third_party/rl_api:env_c_api",
        "//third_party/rl_api:env_c_api_test_suite",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "game",
    srcs = ["examples/game_main.c"],
    copts = ["-std=c99"],
    data = GAME_ASSETS,
    deps = [":game_lib_sdl"],
)

# Run this Python example on a level in game_scripts/levels
# with a command such as:
#
#     bazel run -c opt //:python_game_py3 -- \
#         -l lt_chasm -s width=640 -s height=480
#
# Versions for Python 2 and Python 3 are provided.
#
[py_binary(
    name = "python_game_" + py.lower(),
    srcs = ["examples/game_main.py"],
    data = ["//:deepmind_lab.so"],
    main = "examples/game_main.py",
    python_version = py,
    srcs_version = py,
    tags = ["manual"],
    deps = ["@six_archive//:six"],
) for py in [
    "PY2",
    "PY3",
]]

config_setting(
    name = "dmlab_graphics_sdl",
    define_values = {"graphics": "sdl"},
)

config_setting(
    name = "dmlab_graphics_osmesa_or_glx",
    define_values = {"graphics": "osmesa_or_glx"},
)

config_setting(
    name = "dmlab_graphics_osmesa_or_egl",
    define_values = {"graphics": "osmesa_or_egl"},
)

cc_binary(
    name = "libdmlab_headless_hw.so",
    linkopts = ["-Wl,--version-script,$(location :dmlab.lds)"],
    linkshared = 1,
    linkstatic = 1,
    visibility = ["//testing:__subpackages__"],
    deps = [":dmlab.lds"] + select({
        "dmlab_graphics_osmesa_or_egl": [":game_lib_headless_egl"],
        "dmlab_graphics_osmesa_or_glx": [":game_lib_headless_glx"],
        "//conditions:default": [":game_lib_headless_egl"],
    }),
)

cc_binary(
    name = "libdmlab_headless_sw.so",
    linkopts = ["-Wl,--version-script,$(location :dmlab.lds)"],
    linkshared = 1,
    linkstatic = 1,
    visibility = ["//testing:__subpackages__"],
    deps = [
        ":dmlab.lds",
        ":game_lib_headless_osmesa",
    ],
)

cc_library(
    name = "dmlab_so_loader",
    srcs = ["public/dmlab_so_loader.cc"],
    hdrs = ["public/dmlab.h"],
    data = [
        ":libdmlab_headless_hw.so",
        ":libdmlab_headless_sw.so",
    ],
    linkopts = ["-ldl"],
    visibility = ["//testing:__subpackages__"],
    deps = [
        ":file_reader_types",
        ":level_cache_types",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
    ],
    alwayslink = 1,
)

cc_library(
    name = "dmlablib",
    hdrs = ["public/dmlab.h"],
    data = GAME_ASSETS,
    visibility = ["//testing:__subpackages__"],
    deps = [
        ":file_reader_types",
        ":level_cache_types",
        "//third_party/rl_api:env_c_api",
    ] + select({
        "dmlab_graphics_sdl": [":game_lib_sdl"],
        "//conditions:default": [":dmlab_so_loader"],
    }),
)

cc_binary(
    name = "deepmind_lab.so",
    srcs = [
        "public/dmlab.h",
        "python/dmlab_module.c",
    ],
    copts = [
        "-fno-strict-aliasing",  ## ick ick ick
        "-std=c99",
        "-DDEEPMIND_LAB_MODULE_RUNFILES_DIR",
    ],
    linkshared = 1,
    linkstatic = 1,
    visibility = [
        "//python/pip_package:__subpackages__",
        "//python/tests:__subpackages__",
    ],
    deps = [
        ":dmlablib",
        "@python_system//:python_headers",
    ],
)

py_library(
    name = "dmenv_module",
    srcs = ["python/dmenv_module.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    visibility = ["//python/tests:__subpackages__"],
    deps = [
        "@dm_env_archive//:dm_env",
        "@six_archive//:six",
    ],
)

py_binary(
    name = "python_random_agent",
    srcs = ["python/random_agent.py"],
    main = "python/random_agent.py",
    deps = [":python_random_agent_lib"],
)

py_library(
    name = "python_random_agent_lib",
    srcs = ["python/random_agent.py"],
    data = [":deepmind_lab.so"],
    visibility = ["//python/tests:__subpackages__"],
    deps = ["@six_archive//:six"],
)

LOAD_TEST_SCRIPTS = [
    level_script[len("game_scripts/levels/"):-len(".lua")]
    for level_script in glob(
        include = ["game_scripts/levels/**/*.lua"],
        exclude = [
            "game_scripts/levels/**/tests/**",
            "game_scripts/levels/**/factories/**",
        ],
    )
]

test_suite(
    name = "load_level_test",
    tests = ["load_level_test_" + level_name for level_name in LOAD_TEST_SCRIPTS],
)

[
    cc_test(
        name = "load_level_test_" + level_name,
        size = "large",
        args = [level_name],
        tags = ["noprecommit"],  # Takes too long for GitHub action
        deps = ["//testing:load_level_test_lib"],
    )
    for level_name in LOAD_TEST_SCRIPTS
]

SEED_TEST_SCRIPTS = [
    level_script[len("game_scripts/levels/"):-len(".lua")]
    for level_script in glob(
        include = ["game_scripts/levels/**/*.lua"],
        exclude = [
            "game_scripts/levels/**/demos/**",
            "game_scripts/levels/**/factories/**",
            "game_scripts/levels/**/tests/**",
        ],
    )
]

test_suite(
    name = "seed_level_test",
    tests = ["seed_level_test_" + level_name for level_name in SEED_TEST_SCRIPTS],
)

[
    cc_test(
        name = "seed_level_test_" + level_name,
        size = "large",
        args = [level_name],
        tags = ["noprecommit"],  # Takes too long for GitHub action
        deps = ["//testing:seed_level_test_lib"],
    )
    for level_name in SEED_TEST_SCRIPTS
]
