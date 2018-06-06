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

#include "deepmind_hooks.h"
#include "../deepmind/context.h"
#include "../deepmind/dm_public.h"
#include "../renderercommon/tr_types.h"
#include "../game/bg_public.h"
#include "qcommon.h"

int dmlab_callback(
    int dm_callnum, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4,
    intptr_t a5, intptr_t a6, intptr_t a7, intptr_t a8, intptr_t a9,
    intptr_t a10, intptr_t a11, intptr_t a12) {
  DeepmindContext* ctx = dmlab_context();
  switch (dm_callnum) {
    case DEEPMIND_UPDATE_SPAWN_VARS:
      return ctx->hooks.pickups.update_spawn_vars(ctx->userdata,
                                          /*spawn_var_chars=*/VM_ArgPtr(a1),
                                          /*num_spawn_var_chars=*/VM_ArgPtr(a2),
                                          /*spawn_vars_offsets=*/VM_ArgPtr(a3),
                                          /*num_spawn_vars=*/VM_ArgPtr(a4));

    case DEEPMIND_MAKE_EXTRA_ENTITIES:
      return ctx->hooks.pickups.make_extra_entities(ctx->userdata);
    case DEEPMIND_READ_EXTRA_ENTITY:
      ctx->hooks.pickups.read_extra_entity(ctx->userdata,
                                   /*entity_index=*/a1,
                                   /*spawn_var_chars=*/VM_ArgPtr(a2),
                                   /*num_spawn_var_chars=*/VM_ArgPtr(a3),
                                   /*spawn_vars_offsets=*/VM_ArgPtr(a4),
                                   /*num_spawn_vars=*/VM_ArgPtr(a5));
      return 0;
    case DEEPMIND_FIND_ITEM:
      return ctx->hooks.pickups.find_item(ctx->userdata, /*class_name=*/VM_ArgPtr(a1),
                                  /*index=*/VM_ArgPtr(a2));
    case DEEPMIND_ITEM_COUNT:
      return ctx->hooks.pickups.item_count(ctx->userdata);
    case DEEPMIND_ITEM:
      return ctx->hooks.pickups.item(
          ctx->userdata, /*index=*/a1, /*item_name=*/VM_ArgPtr(a2),
          /*max_item_name=*/a3,
          /*class_name=*/VM_ArgPtr(a4), /*max_class_name=*/a5,
          /*model_name=*/VM_ArgPtr(a6), /*max_model_name=*/a7,
          /*quantity=*/VM_ArgPtr(a8), /*type=*/VM_ArgPtr(a9),
          /*tag=*/VM_ArgPtr(a10),
          /*move_type=*/VM_ArgPtr(a11));
    case DEEPMIND_CLEAR_ITEMS:
      ctx->hooks.pickups.clear_items(ctx->userdata);
      return 1;

    case DEEPMIND_REGISTER_DYNAMIC_ITEMS:
      return ctx->hooks.pickups.register_dynamic_items(ctx->userdata);
    case DEEPMIND_READ_DYNAMIC_ITEM_NAME:
      ctx->hooks.pickups.read_dynamic_item_name(ctx->userdata,
                                                /*item_index=*/a1,
                                                /*item_name=*/VM_ArgPtr(a2));
      return 0;

    case DEEPMIND_DYNAMIC_SPAWN_ENTITY_COUNT:
      return ctx->hooks.pickups.dynamic_spawn_entity_count(ctx->userdata);
    case DEEPMIND_CLEAR_DYNAMIC_SPAWN_ENTITIES:
      ctx->hooks.pickups.clear_dynamic_spawn_entities(ctx->userdata);
      return 0;
    case DEEPMIND_READ_DYNAMIC_SPAWN_ENTITY:
      ctx->hooks.pickups.read_dynamic_spawn_entity(ctx->userdata,
                             /*entity_index=*/a1,
                             /*spawn_var_chars=*/VM_ArgPtr(a2),
                             /*num_spawn_var_chars=*/VM_ArgPtr(a3),
                             /*spawn_vars_offsets=*/VM_ArgPtr(a4),
                             /*num_spawn_vars=*/VM_ArgPtr(a5));
      return 0;
    case DEEPMIND_FINISH_MAP:
      ctx->hooks.set_map_finished(ctx->userdata, /*map_finished=*/a1);
      return 1;
    case DEEPMIND_CAN_PICKUP: {
      const playerState_t* ps = VM_ArgPtr(a2);
      return ctx->hooks.can_pickup(ctx->userdata, /*entity_id=*/a1,
                                   /*player_id=*/ps->clientNum);
    }
    case DEEPMIND_OVERRIDE_PICKUP: {
      const playerState_t* ps = VM_ArgPtr(a3);
      return ctx->hooks.override_pickup(ctx->userdata, /*entity_id=*/a1,
                                        /*respawn=*/VM_ArgPtr(a2),
                                        /*player_id=*/ps->clientNum);
    }
    case DEEPMIND_CAN_TRIGGER: {
      const playerState_t* ps = VM_ArgPtr(a3);
      return ctx->hooks.can_trigger(ctx->userdata, /*entity_id=*/a1,
                                    /*target_name=*/VM_ArgPtr(a2),
                                    /*player_id=*/ps->clientNum);
    }
    case DEEPMIND_OVERRIDE_TRIGGER: {
      const playerState_t* ps = VM_ArgPtr(a3);
      return ctx->hooks.override_trigger(ctx->userdata, /*entity_id=*/a1,
                                         /*target_name=*/VM_ArgPtr(a2),
                                         /*player_id=*/ps->clientNum);
    }
    case DEEPMIND_OVERRIDE_LOOKAT: {
      const playerState_t* ps = VM_ArgPtr(a4);
      ctx->hooks.trigger_lookat(ctx->userdata, /*entity_id=*/a1,
                                /*looked_at=*/a2,
                                /*position=*/VM_ArgPtr(a3),
                                /*player_id=*/ps->clientNum);
    } break;
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
      ctx->hooks.player_state(
          ctx->userdata, ps->origin, ps->velocity, ps->viewangles,
          ps->viewheight,
          /*eyePos=*/VM_ArgPtr(a2),
          /*team_score=*/a3,
          /*other_team_score=*/a4, ps->clientNum,
          /*teleporter_flip=*/(ps->eFlags & EF_TELEPORT_BIT),
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
    case DEEPMIND_MAKE_FILLED_RECTANGLES:
      return ctx->hooks.make_filled_rectangles(ctx->userdata,
                                               /*screen_width=*/a1,
                                               /*screen_height=*/a2);
    case DEEPMIND_GET_FILLED_RECTANGLE:
      ctx->hooks.get_filled_rectangle(
          ctx->userdata, /*rectangle_id=*/a1, /*x=*/VM_ArgPtr(a2),
          /*y=*/VM_ArgPtr(a3),
          /*width=*/VM_ArgPtr(a4), /*height=*/VM_ArgPtr(a5),
          /*rgba=*/VM_ArgPtr(a6));
      break;
    case DEEPMIND_PLAYER_SCORE:
      return ctx->calls.player_score(ctx->context);
    case DEEPMIND_LUA_MOVER:
      ctx->hooks.lua_mover(
          ctx->userdata, /*entity_id=*/a1, /*entity_pos=*/VM_ArgPtr(a2),
          /*player_pos=*/VM_ArgPtr(a3), /*player_vel=*/VM_ArgPtr(a4),
          /*player_pos_delta=*/VM_ArgPtr(a5),
          /*player_vel_delta=*/VM_ArgPtr(a6));
      break;
    case DEEPMIND_GAME_EVENT:
      ctx->hooks.game_event(ctx->userdata, /*event_name=*/VM_ArgPtr(a1),
                            /*count=*/a2,
                            /*data=*/VM_ArgPtr(a3));
      break;
    case DEEPMIND_SPAWN_INVENTORY:
    case DEEPMIND_UPDATE_INVENTORY: {
      playerState_t* ps = VM_ArgPtr(a1);
      ctx->hooks.update_inventory(
          ctx->userdata,
          /*is_spawning=*/dm_callnum == DEEPMIND_SPAWN_INVENTORY,
          /*is_bot=*/a2 != 0,
          /*player_id=*/ps->clientNum,
          /*gadget_count=*/MAX_WEAPONS,
          /*gadget_inventory=*/ps->ammo,
          /*persistent_count=*/MAX_PERSISTANT,
          /*persistent=*/ps->persistant,
          /*stat_count=*/MAX_STATS,
          /*stats_inventory=*/ps->stats,
          /*powerup_count=*/MAX_POWERUPS,
          /*powerup_time=*/ps->powerups,
          /*gadget_held=*/ps->weapon,
          /*height=*/ps->viewheight,
          /*position=*/ps->origin,
          /*velocity=*/ps->velocity,
          /*view_angles=*/ps->viewangles);
      break;
    }
    case DEEPMIND_TEAM_SELECT:
      return ctx->hooks.team_select(ctx->userdata,
                                    /*player_id=*/a1,
                                    /*player_name=*/VM_ArgPtr(a2));
      break;
    case DEEPMIND_UPDATE_PLAYER_INFO:
      return ctx->hooks.update_player_info(ctx->userdata,
                                           /*player_id=*/a1,
                                           /*info=*/VM_ArgPtr(a2),
                                           /*info_size=*/a3);
      break;
    case DEEPMIND_ENTITIES_CLEAR:
      ctx->hooks.entities.clear(ctx->userdata);
      break;
    case DEEPMIND_ENTITIES_ADD:
      ctx->hooks.entities.add(ctx->userdata, /*player_id=*/a1,
                              /*user_id=*/a2,
                              /*type=*/a3,
                              /*flags=*/a4,
                              /*position=*/VM_ArgPtr(a5),
                              /*classname=*/VM_ArgPtr(a6));
      break;
    case DEEPMIND_CUSTOM_VIEW: {
      refdef_t* camera = VM_ArgPtr(a1);
      vec3_t angles = {0, 0, 0};
      bool render_player;
      ctx->hooks.custom_view(ctx->userdata, &camera->width, &camera->height,
                             camera->vieworg, angles, &render_player);
      AnglesToAxis(angles, camera->viewaxis);
      int width, height, buff_width, buff_height;
      ctx->calls.screen_shape(&width, &height, &buff_width, &buff_height);
      camera->y = buff_height - camera->height;
      double aspect = (double)camera->height / camera->width;
      camera->fov_x = 90;
      double fov_x_rad = camera->fov_x * (M_PI / 180.0);
      double fov_y_rad = 2.0 * atan(aspect * tan(fov_x_rad * 0.5));
      camera->fov_y = (180.0 / M_PI) * fov_y_rad;
      return (render_player) ? qtrue : qfalse;
    }
    case DEEPMIND_NEW_CLIENT_INFO:
      ctx->hooks.new_client_info(ctx->userdata, a1, VM_ArgPtr(a2),
                                 VM_ArgPtr(a3));
      break;
    default:
      Com_Error(ERR_DROP, "DeepMind system call %d not implemented\n",
                dm_callnum);
  }

  return 0;
}
