// Copyright (C) 2016-2018 Google Inc.
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
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "public/dmlab.h"

enum {
  MAX_OBSERVATIONS = 1024,
  MAX_RUNFILES_PATH = 4096
};

static void __attribute__((noreturn, format(printf, 1, 2)))
sys_error(const char* fmt, ...) {
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
    "            [--observation <S>]               \\\n"
    "            [--num_episodes <N>]              \\\n"
    "            [--start_index <N>]               \\\n"
    "            [--print_events]                  \\\n"
    "            [--random_seed <S>]\n"
    "\n"
    "  -l, --level_script:   Mandatory. The level that is to be played. Levels are\n"
    "                        Lua scripts, and a script called \"name\" means that a\n"
    "                        file \"assets/game_scripts/name.lua\" is loaded.\n"
    "  -s, --level_setting:  Applies an opaque key-value setting. The setting is\n"
    "                        available to the level script. This flag may be provided\n"
    "                        multiple times.\n"
    "  -o, --observation:    Print specified observation each frame. This flag\n"
    "                        may be provided multiple times.\n"
    "  -e, --num_episodes:   The number of episodes to play. Defaults to 1.\n"
    "  -i, --start_index:    Starting episode index. Defaults to 0.\n"
    "  -p, --print_events:   Print events emitted.\n"
    "  -r, --random_seed:    A seed value used for randomly generated content; using\n"
    "                        the same seed should result in the same content. Defaults\n"
    "                        to a fixed value.\n"
    "  -m, --mixer_seed:     A XOR mask applied to the most significant bits of the seed.\n"
    "  -f, --fps:            Set the frames per second the game runs at. Note\n"
    "                        com_maxFPS is set to this value too (default 250).\n"
    ;

static void process_commandline(int argc, char** argv, EnvCApi* env_c_api,
                                void* context, int* start_index,
                                int* num_episodes, int* seed, int* mixer_seed,
                                bool* log_events, const char** fps,
                                const char* observation_names[],
                                int* observation_count) {
  static struct option long_options[] = {
      {"help", no_argument, NULL, 'h'},
      {"level_script", required_argument, NULL, 'l'},
      {"level_setting", required_argument, NULL, 's'},
      {"start_index", required_argument, NULL, 'i'},
      {"num_episodes", required_argument, NULL, 'e'},
      {"random_seed", required_argument, NULL, 'r'},
      {"mixer_seed", required_argument, NULL, 'm'},
      {"print_events", no_argument, NULL, 'p'},
      {"fps", no_argument, NULL, 'f'},
      {"observation", required_argument, NULL, 'o'},
      {NULL, 0, NULL, 0}};

  char *key, *value;
  *observation_count = 0;
  for (int c; (c = getopt_long(argc, argv, "hl:s:i:e:r:m:pf:o:", long_options,
                               0)) != -1;) {
    switch (c) {
      case 'h':
        fputs(kUsage, stdout);
        exit(EXIT_SUCCESS);
      case 'l':
        if (env_c_api->setting(context, "levelName", optarg) != 0) {
          sys_error("Invalid level_script '%s'. Internal error: %s", optarg,
                    env_c_api->error_message(context));
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
          sys_error("Invalid level_setting '%s=%s'. Internal error: %s", key,
                    value, env_c_api->error_message(context));
        }
        break;
      case 'i':
        if (!parse_int(optarg, start_index) || *start_index < 0) {
          sys_error("Failed to set start_index to '%s'.", optarg);
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
      case 'f': {
        int fps_int = 0;
        if (!parse_int(optarg, &fps_int)) {
          sys_error("Failed to set fps to '%s'.", optarg);
        }
        *fps = optarg;
      } break;
      case 'm':
        if (!parse_int(optarg, mixer_seed)) {
          sys_error("Failed to set mixer_seed to '%s'.", optarg);
        }
        if (*mixer_seed < 0 || *mixer_seed > UINT32_MAX) {
          sys_error("Invalid 'mixerSeed' setting. Must be a positive integer "
                    "not greater than %" PRIu32 ".", UINT32_MAX);
        }
        break;
      case 'p':
        *log_events = true;
        break;
      case 'o':
        if (*observation_count >= MAX_OBSERVATIONS) {
          sys_error("Too many observations specified. Maximum number is %d",
                    MAX_OBSERVATIONS);
        }
        observation_names[(*observation_count)++] = optarg;
        break;
      case ':':
      case '?':
      default:
        sys_error("Bad command-line flag. Use --help for usage instructions.");
    }
  }
}

static void print_observation(const EnvCApi_Observation* obs) {
  switch (obs->spec.type) {
    case EnvCApi_ObservationString:
      printf("\"%.*s\"", obs->spec.shape[0], obs->payload.string);
      break;
    case EnvCApi_ObservationDoubles:
      if (obs->spec.dims == 1) {
        if (obs->spec.shape[0] == 1) {
          printf("%f", obs->payload.doubles[0]);
          break;
        } else if (obs->spec.shape[0] < 6) {
          fputs("{", stdout);
          for (int i = 0; i < obs->spec.shape[0]; ++i) {
            if (i != 0) fputs(", ", stdout);
            printf("%f", obs->payload.doubles[i]);
          }
          fputs("}", stdout);
          break;
        }
      }
      fputs("<DoubleTensor ", stdout);
      for (int i = 0; i < obs->spec.dims; ++i) {
        if (i != 0) fputs("x", stdout);
        printf("%d", obs->spec.shape[i]);
      }
      fputs(">", stdout);
      break;
    case EnvCApi_ObservationBytes:
      fputs("<ByteTensor ", stdout);
      for (int i = 0; i < obs->spec.dims; ++i) {
        if (i != 0) fputs("x", stdout);
        printf("%d", obs->spec.shape[i]);
      }
      fputs(">", stdout);
      break;
    default:
      sys_error("Observation type: %d not supported", obs->spec.type);
  }
}

// Prints events to stdout.
static void print_observation_ids(EnvCApi* env_c_api, void* context,
                                  const char* observation_names[],
                                  const int ob_ids[], int ob_count) {
  for (int ob = 0; ob < ob_count; ++ob) {
    if (ob_ids[ob] < 0) {
      continue;
    }
    EnvCApi_Observation observation;
    env_c_api->observation(context, ob_ids[ob], &observation);
    printf("observation \"%s\" - ", observation_names[ob]);
    print_observation(&observation);
    fputs("\n", stdout);
  }
}

// Prints events to stdout. Returns number printed.
static int print_events(EnvCApi* env_c_api, void* context) {
  int event_count = env_c_api->event_count(context);
  for (int e = 0; e < event_count; ++e) {
    EnvCApi_Event event;
    env_c_api->event(context, e, &event);
    printf("Event %d: \"%s\" - ", e,
           env_c_api->event_type_name(context, event.id));
    for (int obs_id = 0; obs_id < event.observation_count; ++obs_id) {
      if (obs_id != 0) {
        fputs(", ", stdout);
      }
      print_observation(&event.observations[obs_id]);
    }
    fputs("\n", stdout);
  }
  return event_count;
}

int main(int argc, char** argv) {
  static const char kRunfiles[] = ".runfiles/org_deepmind_lab";
  static EnvCApi env_c_api;
  static void* context;
  static char runfiles_path[MAX_RUNFILES_PATH];
  static const char* observation_names[MAX_OBSERVATIONS];
  static int observation_ids[MAX_OBSERVATIONS];
  static int observation_count = 0;

  bool log_events = false;

  if (sizeof(runfiles_path) < strlen(argv[0]) + sizeof(kRunfiles)) {
    sys_error("Runfiles directory name too long!");
  }
  strcpy(runfiles_path, argv[0]);
  strcat(runfiles_path, kRunfiles);

  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path;

  if (dmlab_connect(&params, &env_c_api, &context) != 0) {
    sys_error("Failed to connect RL API");
  }

  if (env_c_api.setting(context, "width", "640") != 0) {
    sys_error("Failed to apply default 'width' setting. Internal error: %s",
              env_c_api.error_message(context));
  }

  if (env_c_api.setting(context, "height", "480") != 0) {
    sys_error("Failed to apply default 'height' setting. Internal error: %s",
              env_c_api.error_message(context));
  }

  if (env_c_api.setting(context, "nativeApp", "true") != 0) {
    sys_error("Failed to apply 'nativeApp' setting. Internal error: %s",
              env_c_api.error_message(context));
  }

  int start_index = 0;
  int num_episodes = 1;
  int seed = 1;
  int mixer_seed = 0;
  const char* fps = NULL;
  process_commandline(argc, argv, &env_c_api, context, &start_index,
                      &num_episodes, &seed, &mixer_seed, &log_events, &fps,
                      observation_names, &observation_count);
  if (env_c_api.setting(context, "appendCommand", " +set com_maxfps ") != 0) {
    sys_error("Failed to apply 'appendCommand' setting. Internal error: %s",
              env_c_api.error_message(context));
  }

  env_c_api.setting(context, "appendCommand", " +set com_maxfps ");
  if (fps != NULL) {
    if (env_c_api.setting(context, "appendCommand", fps) != 0) {
      sys_error("Failed to apply 'appendCommand' setting. Internal error: %s",
                env_c_api.error_message(context));
    }
    if (env_c_api.setting(context, "fps", fps) != 0) {
      sys_error("Failed to apply 'fps' setting. Internal error: %s",
                env_c_api.error_message(context));
    }
  } else {
    env_c_api.setting(context, "appendCommand", "250");
  }

  static char mixer_seed_str[16];
  snprintf(mixer_seed_str, sizeof(mixer_seed_str), "%d", mixer_seed);
  if (env_c_api.setting(context, "mixerSeed", mixer_seed_str) != 0) {
    sys_error("Failed to apply 'mixerSeed' setting. Internal error: %s",
              env_c_api.error_message(context));
  }

  if (env_c_api.init(context) != 0) {
    sys_error("Failed to init RL API: %s", env_c_api.error_message(context));
  }

  int env_observation_count = env_c_api.observation_count(context);

  for (int ob = 0; ob < observation_count; ++ob) {
    observation_ids[ob] = -1;
    for (int env_ob = 0; env_ob < env_observation_count; ++env_ob) {
      if (strcmp(observation_names[ob],
                 env_c_api.observation_name(context, env_ob)) == 0) {
        observation_ids[ob] = env_ob;
        break;
      }
    }
    if (observation_ids[ob] == -1) {
      sys_error("Requested observation '%s' not found in environment",
                observation_names[ob]);
    }
  }

  EnvCApi_EnvironmentStatus status = EnvCApi_EnvironmentStatus_Running;
  for (int i = 0; i < num_episodes && status != EnvCApi_EnvironmentStatus_Error;
       ++i, ++seed) {
    int episode = start_index + i;
    if (env_c_api.start(context, episode, seed) != 0) {
      sys_error("Failed to start environment. Internal error: %s",
                env_c_api.error_message(context));
    }
    printf("Episode: %d\n", episode);
    if (log_events && print_events(&env_c_api, context) != 0) {
      fflush(stdout);
    }
    double score = 0;
    double reward;

    while (EnvCApi_EnvironmentStatus_Running ==
           (status = env_c_api.advance(context, 1, &reward))) {
      int event_count = log_events ? print_events(&env_c_api, context) : 0;
      if (reward != 0.0) {
        score += reward;
        printf("Score: %f\n", score);
      }
      if (event_count != 0 || reward != 0.0) {
        fflush(stdout);
      }

      if (observation_count > 0) {
        print_observation_ids(&env_c_api, context, observation_names,
                              observation_ids, observation_count);
      }
    }
    if ((log_events && print_events(&env_c_api, context) != 0) ||
        observation_count > 0) {
      fflush(stdout);
    }
  }
  if (status == EnvCApi_EnvironmentStatus_Error) {
    sys_error("%s", env_c_api.error_message(context));
  }

  env_c_api.release_context(context);
}
