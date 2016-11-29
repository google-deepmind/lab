/*
===========================================================================
Copyright (C) 2006 Dmn_clown (aka: Bob Isaac (rjisaac@gmail.com))

This file is part of Open Arena and is based upon Mr. Elusive's fuzzy logic
system found in Quake 3 Arena.

Open Arena is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Open Arena is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

chat "default"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "game_exit" //initiated when the bot exits the game
	{
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "level_start" //initiated when a new level starts
	{
		// 0 = bot name
	} //end type
	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		// 0 = bot name
		// 1 = random opponent
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 4 = level's title
	} //end type
	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nodeath" //bot is hit by an opponent's weapon attack but didn't die; either praise or insult
	{
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		// 0 = opponent
	} //end type
	type "enemy_suicide" //enemy of the bot commits suicide
	{
		// 0 = enemy
	} //end type
	//======================================================
	//======================================================
	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		// 0 = enemy name
	} //end type
	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		// 0 = random opponent
	} //end type
	type "death_lava" //initiated when the bot dies in lava
	{
		// 0 = random opponent
	} //end type
	type "death_slime" //initiated when the bot dies in slime
	{
		// 0 = random opponent
	} //end type
	type "death_drown" //initiated when the bot drowns
	{
		// 0 = random opponent
	} //end type
	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		// 0 = random opponent
	} //end type
	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to Gauntlet)
	} //end type
	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to Railgun)
	} //end type
	type "death_bfg" //initiated when the bot died by a BFG
	{
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to BFG10K)
	} //end type
	type "death_insult" //insult initiated when the bot died
	{
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type
	type "death_praise" //praise initiated when the bot died
	{
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type
	//======================================================
	//======================================================
	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		// 0 = enemy name
	} //end type
	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		// 0 = enemy name
	} //end type
	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		// 0 = enemy name
	} //end type
	type "kill_insult" //insult initiated when the bot killed someone
	{
		// 0 = enemy name
	} //end type
	type "kill_praise" //praise initiated when the bot killed someone
	{
		// 0 = enemy name
	} //end type
	//======================================================
	//======================================================
	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
	type "random_misc" //miscellanous chats initiated randomly
	{
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
} //end chat

