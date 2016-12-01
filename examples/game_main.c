// Copyright (C) 2016 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "public/dmlab.h"

static void __attribute__((noreturn, format(printf, 1, 2))) sys_error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fputc('\n', stderr);
  exit(EXIT_FAILURE);
}

// Returns whether val was successfully read from str.
static bool parse_int(const char* str, int* val) {
  char* e;
  errno = 0;
  long int n = strtol(str, &e, 0);
  if (e != str && *e == '\0' && errno == 0 && INT_MIN <= n && n <= INT_MAX) {
    *val = n;
    return true;
  }
  return false;
}

static const char kUsage[] =
    "Interactive DeepMind Lab \"Game\"\n"
    "\n"
    "Usage: game --level_script <level>            \\\n"
    "            [--level_setting key=value [...]] \\\n"
    "            [--num_episodes <N>]              \\\n"
    "            [--random_seed <S>]\n"
    "\n"
    "  -l, --level_script:   Mandatory. The level that is to be played. Levels are\n"
    "                        Lua scripts, and a script called \"name\" means that a\n"
    "                        file \"assets/game_scripts/name.lua\" is loaded.\n"
    "  -s, --level_setting:  Applies an opaque key-value setting. The setting is\n"
    "                        available to the level script. This flag may be provided\n"
    "                        multiple times.\n"
    "  -e, --num_episodes:   The number of episodes to play. Defaults to 1.\n"
    "  -r, --random_seed:    A seed value used for randomly generated content; using\n"
    "                        the same seed should result in the same content. Defaults\n"
    "                        to a fixed value.\n"
    ;

static void process_commandline(int argc, char** argv, EnvCApi* env_c_api,
                                void* context, int* num_episodes, int* seed) {
  static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"level_script", required_argument, NULL, 'l'},
      {"level_setting", required_argument, NULL, 's'},
      {"num_episodes", required_argument, NULL, 'e'},
      {"random_seed", required_argument, NULL, 'r'},
      {NULL, 0, NULL, 0}};

  char *key, *value;

  for (int c; (c = getopt_long(argc, argv, "hl:s:e:r:", long_options, 0)) != -1;) {
    switch (c) {
      case 'h':
        fputs(kUsage, stdout);
        exit(EXIT_SUCCESS);
      case 'l':
        if (env_c_api->setting(context, "levelName", optarg) != 0) {
          sys_error("Invalid levelName flag '%s'.", optarg);
        }
        break;
      case 's':
        key = optarg;
        value = strchr(optarg, '=');
        if (value == NULL) {
          sys_error("Setting must be in the form 'key=value'.");
        }
        value[0] = '\0';
        ++value;
        if (env_c_api->setting(context, key, value) != 0) {
          sys_error("Failed to apply setting '%s = %s'.", key, value);
        }
        break;
      case 'e':
        if (!parse_int(optarg, num_episodes) || *num_episodes <= 0) {
          sys_error("Failed to set num_episodes to '%s'.", optarg);
        }
        break;
      case 'r':
        if (!parse_int(optarg, seed)) {
          sys_error("Failed to set random_seed to '%s'.", optarg);
        }
        break;
      case ':':
      case '?':
      default:
        sys_error("Bad command-line flag. Use --help for usage instructions.");
    }
  }
}

int main(int argc, char** argv) {
  static const char kRunfiles[] = ".runfiles/org_deepmind_lab";
  static EnvCApi env_c_api;
  static void* context;
  static char runfiles_path[4096];

  if (sizeof(runfiles_path) < strlen(argv[0]) + sizeof(kRunfiles)) {
    sys_error("Runfiles directory name too long!");
  }
  strcpy(runfiles_path, argv[0]);
  strcat(runfiles_path, kRunfiles);

  DeepMindLabLaunchParams params;
  params.runfiles_path = runfiles_path;
  if (dmlab_connect(&params, &env_c_api, &context) != 0) {
    sys_error("Failed to connect RL API");
  }

  if (env_c_api.setting(context, "width", "640") != 0) {
    sys_error("Failed to apply default 'width' setting.");
  }

  if (env_c_api.setting(context, "height", "480") != 0) {
    sys_error("Failed to apply default 'height' setting.");
  }

  if (env_c_api.setting(context, "controls", "internal") != 0) {
    sys_error("Failed to apply 'controls' setting.");
  }

  if (env_c_api.setting(context, "appendCommand", " +set com_maxfps \"250\"")
      != 0) {
    sys_error("Failed to apply 'appendCommand' setting.");
  }

  int num_episodes = 1;
  int seed = 1;
  process_commandline(argc, argv, &env_c_api, context, &num_episodes, &seed);

  if (env_c_api.init(context) != 0) {
    sys_error("Failed to init RL API");
  }

  for (int episode = 0; episode < num_episodes; ++episode, ++seed) {
    if (env_c_api.start(context, episode, seed) != 0) {
      sys_error("Failed to start environment.");
    }
    printf("Episode: %d\n", episode);
    double score = 0;
    double reward;
    while (env_c_api.advance(context, 1, &reward) ==
           EnvCApi_EnvironmentStatus_Running) {
      if (reward != 0.0) {
        score += reward;
        printf("Score: %f\n", score);
        fflush(stdout);
      }
    }
  }

  env_c_api.release_context(context);
}
