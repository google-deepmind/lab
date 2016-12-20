# Description:
#   DeepMind Lab, a machine-learning research environment
#   forked from ioquake/ioq3.

licenses(["restricted"])  # GPLv2

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

# To link with the SDL frontend, depend on :game_lib_sdl
# and add "-lGL" to the linker flags.
cc_library(
    name = "game_lib_sdl",
    srcs = [
        CODE_DIR + "/asm/ftola.c",
        CODE_DIR + "/asm/qasm-inline.h",
        CODE_DIR + "/asm/snapvector.c",
        CODE_DIR + "/cgame/cg_public.h",
        CODE_DIR + "/client/libmumblelink.c",
        CODE_DIR + "/deepmind/dm_public.h",
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/game/bg_public.h",
        CODE_DIR + "/game/g_public.h",
        CODE_DIR + "/sdl/sdl_icon.h",
        CODE_DIR + "/sdl/sdl_input.c",
        CODE_DIR + "/sdl/sdl_snd.c",
        CODE_DIR + "/sys/con_log.c",
        CODE_DIR + "/sys/con_passive.c",
        CODE_DIR + "/sys/sys_main.c",
        CODE_DIR + "/sys/sys_unix.c",
        CODE_DIR + "/ui/ui_public.h",

        ## OpenGL rendering
        CODE_DIR + "/sdl/sdl_gamma.c",
        CODE_DIR + "/sdl/sdl_glimp.c",
    ] + glob(
        [
            CODE_DIR + "/botlib/*.c",
            CODE_DIR + "/client/cl_*.c",
            CODE_DIR + "/client/snd_*.c",
            CODE_DIR + "/qcommon/*.c",
            CODE_DIR + "/renderercommon/*.c",
            CODE_DIR + "/renderergl1/*.c",
            CODE_DIR + "/server/*.c",
        ],
        exclude = [
            CODE_DIR + "/renderergl1/tr_subs.c",
            CODE_DIR + "/server/sv_rankings.c",
            CODE_DIR + "/qcommon/vm_none.c",
            CODE_DIR + "/qcommon/vm_powerpc*.c",
            CODE_DIR + "/qcommon/vm_sparc.c",
        ],
    ),
    hdrs = [
        "public/dmlab.h",
        CODE_DIR + "/deepmind/context.h",
    ] + glob(
        [
            CODE_DIR + "/botlib/*.h",
            CODE_DIR + "/client/*.h",
            CODE_DIR + "/qcommon/*.h",
            CODE_DIR + "/sys/*.h",
            CODE_DIR + "/server/*.h",
            CODE_DIR + "/renderercommon/*.h",
            CODE_DIR + "/renderergl1/*.h",
        ],
        exclude = [
            CODE_DIR + "/client/fx_*.h",
            CODE_DIR + "/qcommon/vm_powerpc_asm.h",
            CODE_DIR + "/qcommon/vm_sparc.h",
            CODE_DIR + "/renderercommon/tr_types.h",
            CODE_DIR + "/renderercommon/tr_public.h",
            CODE_DIR + "/renderergl1/tr_local.h",
        ],
    ),
    copts = [
        "-std=c99",
        "-fno-strict-aliasing",
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    defines = [
        "BOTLIB",
        "_GNU_SOURCE",
    ],
    textual_hdrs = [
        CODE_DIR + "/renderercommon/tr_types.h",
        CODE_DIR + "/renderercommon/tr_public.h",
        CODE_DIR + "/renderergl1/tr_local.h",
    ],
    deps = [
        ":qcommon_hdrs",
        "//deepmind/include:context_headers",
        "//third_party/rl_api:env_c_api",
        "@jpeg_archive//:jpeg",
        "@sdl_system//:sdl2",
    ],
)

# To link with the headless OSMesa frontend, depend on :game_lib_headless_osmesa
# and add "-lGL -lOSMesa" to the linker flags.
cc_library(
    name = "game_lib_headless_osmesa",
    srcs = [
        CODE_DIR + "/asm/ftola.c",
        CODE_DIR + "/asm/qasm-inline.h",
        CODE_DIR + "/asm/snapvector.c",
        CODE_DIR + "/cgame/cg_public.h",
        CODE_DIR + "/client/libmumblelink.c",
        CODE_DIR + "/deepmind/dm_public.h",
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/game/bg_public.h",
        CODE_DIR + "/game/g_public.h",
        CODE_DIR + "/null/null_input.c",
        CODE_DIR + "/null/null_snddma.c",
        CODE_DIR + "/sys/con_log.c",
        CODE_DIR + "/sys/con_passive.c",
        CODE_DIR + "/sys/sys_main.c",
        CODE_DIR + "/sys/sys_unix.c",
        CODE_DIR + "/ui/ui_public.h",

        ## OpenGL rendering
        CODE_DIR + "/deepmind/headless_osmesa_glimp.c",
        CODE_DIR + "/deepmind/glimp_common.h",
        CODE_DIR + "/deepmind/glimp_common.c",
    ] + glob(
        [
            CODE_DIR + "/botlib/*.c",
            CODE_DIR + "/client/cl_*.c",
            CODE_DIR + "/client/snd_*.c",
            CODE_DIR + "/qcommon/*.c",
            CODE_DIR + "/renderercommon/*.c",
            CODE_DIR + "/renderergl1/*.c",
            CODE_DIR + "/server/*.c",
        ],
        exclude = [
            CODE_DIR + "/renderergl1/tr_subs.c",
            CODE_DIR + "/server/sv_rankings.c",
            CODE_DIR + "/qcommon/vm_none.c",
            CODE_DIR + "/qcommon/vm_powerpc*.c",
            CODE_DIR + "/qcommon/vm_sparc.c",
        ],
    ),
    hdrs = [
        "public/dmlab.h",
        CODE_DIR + "/deepmind/context.h",
    ] + glob(
        [
            CODE_DIR + "/botlib/*.h",
            CODE_DIR + "/client/*.h",
            CODE_DIR + "/qcommon/*.h",
            CODE_DIR + "/sys/*.h",
            CODE_DIR + "/server/*.h",
            CODE_DIR + "/renderercommon/*.h",
            CODE_DIR + "/renderergl1/*.h",
        ],
        exclude = [
            CODE_DIR + "/client/fx_*.h",
            CODE_DIR + "/qcommon/vm_powerpc_asm.h",
            CODE_DIR + "/qcommon/vm_sparc.h",
            CODE_DIR + "/renderercommon/tr_types.h",
            CODE_DIR + "/renderercommon/tr_public.h",
            CODE_DIR + "/renderergl1/tr_local.h",
        ],
    ),
    copts = [
        "-std=c99",
        "-fno-strict-aliasing",
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    defines = [
        "BOTLIB",
        "_GNU_SOURCE",
    ],
    textual_hdrs = [
        CODE_DIR + "/renderercommon/tr_types.h",
        CODE_DIR + "/renderercommon/tr_public.h",
        CODE_DIR + "/renderergl1/tr_local.h",
    ],
    deps = [
        ":qcommon_hdrs",
        "//deepmind/include:context_headers",
        "//third_party/rl_api:env_c_api",
        "@jpeg_archive//:jpeg",
        "@sdl_system//:sdl2",
    ],
)

# To link with the headless GLX frontend, depend on :game_lib_headless_glx
# and add "-lGL -lX11" to the linker flags.
cc_library(
    name = "game_lib_headless_glx",
    srcs = [
        CODE_DIR + "/asm/ftola.c",
        CODE_DIR + "/asm/qasm-inline.h",
        CODE_DIR + "/asm/snapvector.c",
        CODE_DIR + "/cgame/cg_public.h",
        CODE_DIR + "/client/libmumblelink.c",
        CODE_DIR + "/deepmind/dm_public.h",
        CODE_DIR + "/deepmind/dmlab_connect.c",
        CODE_DIR + "/game/bg_public.h",
        CODE_DIR + "/game/g_public.h",
        CODE_DIR + "/null/null_input.c",
        CODE_DIR + "/null/null_snddma.c",
        CODE_DIR + "/sys/con_log.c",
        CODE_DIR + "/sys/con_passive.c",
        CODE_DIR + "/sys/sys_main.c",
        CODE_DIR + "/sys/sys_unix.c",
        CODE_DIR + "/ui/ui_public.h",

        ## OpenGL rendering
        CODE_DIR + "/deepmind/headless_native_glimp.c",
        CODE_DIR + "/deepmind/glimp_common.h",
        CODE_DIR + "/deepmind/glimp_common.c",
    ] + glob(
        [
            CODE_DIR + "/botlib/*.c",
            CODE_DIR + "/client/cl_*.c",
            CODE_DIR + "/client/snd_*.c",
            CODE_DIR + "/qcommon/*.c",
            CODE_DIR + "/renderercommon/*.c",
            CODE_DIR + "/renderergl1/*.c",
            CODE_DIR + "/server/*.c",
        ],
        exclude = [
            CODE_DIR + "/renderergl1/tr_subs.c",
            CODE_DIR + "/server/sv_rankings.c",
            CODE_DIR + "/qcommon/vm_none.c",
            CODE_DIR + "/qcommon/vm_powerpc*.c",
            CODE_DIR + "/qcommon/vm_sparc.c",
        ],
    ),
    hdrs = [
        "public/dmlab.h",
        CODE_DIR + "/deepmind/context.h",
    ] + glob(
        [
            CODE_DIR + "/botlib/*.h",
            CODE_DIR + "/client/*.h",
            CODE_DIR + "/qcommon/*.h",
            CODE_DIR + "/sys/*.h",
            CODE_DIR + "/server/*.h",
            CODE_DIR + "/renderercommon/*.h",
            CODE_DIR + "/renderergl1/*.h",
        ],
        exclude = [
            CODE_DIR + "/client/fx_*.h",
            CODE_DIR + "/qcommon/vm_powerpc_asm.h",
            CODE_DIR + "/qcommon/vm_sparc.h",
            CODE_DIR + "/renderercommon/tr_types.h",
            CODE_DIR + "/renderercommon/tr_public.h",
            CODE_DIR + "/renderergl1/tr_local.h",
        ],
    ),
    copts = [
        "-std=c99",
        "-fno-strict-aliasing",
        ARCH_VAR,
        STANDALONE_VAR,
    ],
    defines = [
        "BOTLIB",
        "_GNU_SOURCE",
    ],
    textual_hdrs = [
        CODE_DIR + "/renderercommon/tr_types.h",
        CODE_DIR + "/renderercommon/tr_public.h",
        CODE_DIR + "/renderergl1/tr_local.h",
    ],
    deps = [
        ":qcommon_hdrs",
        "//deepmind/include:context_headers",
        "//third_party/rl_api:env_c_api",
        "@jpeg_archive//:jpeg",
        "@sdl_system//:sdl2",
    ],
)

ASSETS = [
    "assets/default.cfg",
    "assets/q3config.cfg",
] + glob(["assets/game_scripts/**/*.lua"])

MAPS = glob(["assets/maps/*.map"])

genrule(
    name = "map_assets",
    srcs = MAPS,
    outs = ["baselab/" + f[12:-3] + "pk3" for f in MAPS],
    cmd = "cp -t $(@D)/baselab $(SRCS); " +
          "for s in $(SRCS); do " +
          "  BM=$$(basename $${s}); M=$${BM/.map/}; " +
          "  $(location //deepmind/level_generation:compile_map_sh).runfiles/org_deepmind_lab/deepmind/level_generation/compile_map_sh $(@D)/baselab/$${M}; " +
          "done",
    tools = [
        "//:bspc",
        "//deepmind/level_generation:compile_map_sh",
        "//q3map2",
    ],
)

genrule(
    name = "non_pk3_assets",
    srcs = ASSETS,
    outs = ["baselab/" + f[7:] for f in ASSETS],
    cmd = "for s in $(SRCS); do " +
          "  A=$$(dirname $$s); " +
          "  B=$${A/assets/}; " +
          "  mkdir -p $(@D)/baselab$${B}; " +
          "  ln -s -L -t $(@D)/baselab$${B} $$(realpath $${s}); " +
          "done",
    visibility = ["//visibility:public"],
)

genrule(
    name = "assets_oa_pk3",
    srcs = ["assets_oa/scripts/shaderlist.txt"] + glob([
        "assets_oa/scripts/**/*.shader",
    ]),
    outs = ["baselab/assets_oa.pk3"],
    cmd = "A=$$(pwd); (cd assets_oa; zip -r $${A}/$(OUTS) -- .)",
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
    cmd = "A=$$(pwd); (cd assets; zip -r $${A}/$(OUTS) -- .)",
    visibility = ["//visibility:public"],
)

genrule(
    name = "assets_bots_pk3",
    srcs = ["assets_oa/scripts/bots.txt"] + glob(["assets_oa/botfiles/**/*"]),
    outs = ["baselab/assets_bots.pk3"],
    cmd = "A=$$(pwd); (cd assets_oa; zip -r $${A}/$(OUTS) -- .)",
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
    cmd = "A=$$(pwd); mkdir $(@D)/vm; ln -s -r -t $(@D)/vm -- $(SRCS); (cd $(@D); zip -r $${A}/$(OUTS) -- vm)",
)

cc_binary(
    name = "game",
    srcs = ["examples/game_main.c"],
    copts = ["-std=c99"],
    data = [
        ":assets_bots_pk3",
        ":assets_oa_pk3",
        ":assets_pk3",
        ":map_assets",
        ":non_pk3_assets",
        ":vm_pk3",
    ],
    linkopts = [
        "-lGL",
        "-lm",
    ],
    deps = [
        ":game_lib_sdl",
        "//deepmind/engine:callbacks",
        "//deepmind/engine:context",
        "@zlib_archive//:zlib",
    ],
)

config_setting(
    name = "dmlab_lib_sdl",
    values = {"define": "headless=false"},
)

config_setting(
    name = "dmlab_headless_hw",
    values = {"define": "headless=glx"},
)

config_setting(
    name = "dmlab_headless_sw",
    values = {"define": "headless=osmesa"},
)

cc_binary(
    name = "libdmlab.so",
    linkopts = select({
        "//conditions:default": ["-lOSMesa"],
        ":dmlab_headless_hw": [
            "-lGL",
            "-lX11",
        ],
        ":dmlab_lib_sdl": [
            "-lGL",
            "-lX11",
        ],
    }) + [
        "-Wl,--version-script",
        ":dmlab.lds",
    ],
    linkshared = 1,
    linkstatic = 1,
    deps = select({
        "//conditions:default": [":game_lib_headless_osmesa"],
        ":dmlab_lib_sdl": [":game_lib_sdl"],
        ":dmlab_headless_hw": [":game_lib_headless_glx"],
        ":dmlab_headless_sw": [":game_lib_headless_osmesa"],
    }) + [
        ":dmlab.lds",
        "//deepmind/engine:callbacks",
        "//deepmind/engine:context",
        "@zlib_archive//:zlib",
    ],
)

cc_library(
    name = "dmlablib",
    srcs = ["public/dmlab_so_loader.cc"],
    hdrs = ["public/dmlab.h"],
    copts = ["-DDMLAB_SO_LOCATION=\\\"libdmlab.so\\\""],
    data = [
        ":assets",
        ":libdmlab.so",
        "//deepmind/level_generation:compile_map_sh",
    ],
    linkopts = ["-ldl"],
    deps = ["//third_party/rl_api:env_c_api"],
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
    data = [
        ":assets_bots_pk3",
        ":assets_oa_pk3",
        ":assets_pk3",
        ":map_assets",
        ":non_pk3_assets",
        ":vm_pk3",
    ],
    linkshared = 1,
    linkstatic = 1,
    deps = [
        ":dmlablib",
        "@python_system//:python",
    ],
)

py_test(
    name = "python_module_test",
    srcs = ["python/dmlab_module_test.py"],
    data = [":deepmind_lab.so"],
    imports = ["python"],
    main = "python/dmlab_module_test.py",
)

py_binary(
    name = "python_benchmark",
    srcs = ["python/benchmark.py"],
    data = [":deepmind_lab.so"],
    main = "python/benchmark.py",
)

py_binary(
    name = "random_agent",
    srcs = ["python/random_agent.py"],
    data = [":deepmind_lab.so"],
    main = "python/random_agent.py",
)

py_test(
    name = "random_agent_test",
    srcs = ["python/random_agent_test.py"],
    main = "python/random_agent_test.py",
    deps = [":random_agent"],
)
