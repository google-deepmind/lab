// Copyright (C) 2016-2017 Google Inc.
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

#include "deepmind_hooks.h"
#include "../deepmind/context.h"
#include "../deepmind/dm_public.h"
#include "qcommon.h"

int dmlab_callback(
    int dm_callnum, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4,
    intptr_t a5, intptr_t a6, intptr_t a7, intptr_t a8, intptr_t a9,
    intptr_t a10, intptr_t a11, intptr_t a12) {
  DeepmindContext* ctx = dmlab_context();
  switch (dm_callnum) {
    case DEEPMIND_UPDATE_SPAWN_VARS:
      return ctx->hooks.update_spawn_vars(ctx->userdata,
                                          /*spawn_var_chars=*/VM_ArgPtr(a1),
                                          /*num_spawn_var_chars=*/VM_ArgPtr(a2),
                                          /*spawn_vars_offsets=*/VM_ArgPtr(a3),
                                          /*num_spawn_vars=*/VM_ArgPtr(a4));

    case DEEPMIND_FIND_ITEM:
      return ctx->hooks.find_item(ctx->userdata, /*class_name=*/VM_ArgPtr(a1),
                                  /*index=*/VM_ArgPtr(a2));
    case DEEPMIND_ITEM_COUNT:
      return ctx->hooks.item_count(ctx->userdata);
    case DEEPMIND_ITEM:
      return ctx->hooks.item(
          ctx->userdata, /*index=*/a1, /*item_name=*/VM_ArgPtr(a2),
          /*max_item_name=*/a3,
          /*class_name=*/VM_ArgPtr(a4), /*max_class_name=*/a5,
          /*model_name=*/VM_ArgPtr(a6), /*max_model_name=*/a7,
          /*quantity=*/VM_ArgPtr(a8), /*type=*/VM_ArgPtr(a9),
          /*tag=*/VM_ArgPtr(a10));
    case DEEPMIND_CLEAR_ITEMS:
      ctx->hooks.clear_items(ctx->userdata);
      return 1;
    case DEEPMIND_FINISH_MAP:
      ctx->hooks.set_map_finished(ctx->userdata, /*map_finished=*/a1);
      return 1;
    case DEEPMIND_CAN_PICKUP:
      return ctx->hooks.can_pickup(ctx->userdata, /*entity_id=*/a1);
    case DEEPMIND_OVERRIDE_PICKUP:
      return ctx->hooks.override_pickup(ctx->userdata, /*entity_id=*/a1,
                                        /*respawn=*/VM_ArgPtr(a2));
    case DEEPMIND_REWARD_OVERRIDE:
      return ctx->hooks.reward_override(ctx->userdata,
                                        /*reason=*/VM_ArgPtr(a1),
                                        /*player_id=*/a2,
                                        /*team=*/a3,
                                        /*other_player_id_opt=*/VM_ArgPtr(a4),
                                        /*origin=*/VM_ArgPtr(a5),
                                        /*score=*/a6);
    case DEEPMIND_SET_PLAYER_STATE: {
      const playerState_t* ps = VM_ArgPtr(a1);
      int timestamp_msec = ctx->calls.total_engine_time_msec();
      ctx->hooks.player_state(ctx->userdata, ps->origin, ps->velocity,
                              ps->viewangles, ps->viewheight,
                              /*team_score=*/a2,
                              /*other_team_score=*/a3, ps->clientNum,
                              timestamp_msec);
      break;
    }
    case DEEPMIND_MAKE_SCREEN_MESSAGES:
      return ctx->hooks.make_screen_messages(ctx->userdata, /*width=*/a1,
                                             /*height=*/a2, /*line_height=*/a3,
                                             /*string_buffer_size=*/a4);
    case DEEPMIND_GET_SCREEN_MESSAGE:
      ctx->hooks.get_screen_message(
          ctx->userdata, /*message_id=*/a1, /*buffer=*/VM_ArgPtr(a2),
          /*x=*/VM_ArgPtr(a3),
          /*y=*/VM_ArgPtr(a4), /*shadow=*/VM_ArgPtr(a5),
          /*align_l0_r1_c2=*/VM_ArgPtr(a6),
          /*rgba=*/VM_ArgPtr(a7));
      break;
      break;
    case DEEPMIND_PLAYER_SCORE:
      return ctx->calls.player_score(ctx->context);
    case DEEPMIND_TEAM_SELECT:
      return ctx->hooks.team_select(ctx->userdata,
                                    /*player_id=*/a1,
                                    /*player_name=*/VM_ArgPtr(a2));
      break;
    default:
      Com_Error(ERR_DROP, "DeepMind system call %d not implemented\n",
                dm_callnum);
  }

  return 0;
}
