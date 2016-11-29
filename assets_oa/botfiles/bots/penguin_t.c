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

chat "Penguin"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"Stop using winblows";
		"GNU RU1Z3!";
		"Linus told me to skin you alive, windoze users.";
		
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "game_exit" //initiated when the bot exits the game
	{
		"I need to recompile my qvms, l8r.";
		"I need to go burn a Win98 CD";
		"Pizza's here, l8r";
		GOODBYE1;
		GOODBYE0;
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "level_start" //initiated when a new level starts
	{
		HELLO2;
		
		// 0 = bot name
	} //end type
	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		3, " sucks!";
		2, ", I let you win.";
		
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		LEVEL_END_VICTORY0;
		LEVEL_END_VICTORY1;
		
		// 0 = bot name
		// 1 = random opponent
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		2, " must be an Arena Lord.";
		1, " kept getting in my way.";
		4, " has never been good to me.";
		2, " is a known cheater... the only way I could have lost.";
		2, " has a more optimized build than I.";
		2, " what use flags did you use in your compile?";
		4, " isn't optimized properly.";
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 4 = level's title
	} //end type
	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"Jerk";
		"Schmuck";
		"Putz";
		DEATH_TALKING;
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nodeath" //bot is hit by an opponent's weapon attack but didn't die; either praise or insult
	{
		"Try harder next time.";
		"Wuss";
		"Heh, you suck.";
		"Nice shooting, not.";
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		HIT_NOKILL0;
		HIT_NOKILL1;
		// 0 = opponent
	} //end type
	type "enemy_suicide" //enemy of the bot commits suicide
	{
		"LOL!";
		// 0 = enemy
	} //end type
	//======================================================
	//======================================================
	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		DEATH_TELEFRAGGED0;
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type
	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		DEATH_FALLING0;
		
		// 0 = random opponent
	} //end type
	type "death_lava" //initiated when the bot dies in lava
	{
		"It burns!";
		"That's hot.";
		DEATH_SUICIDE0;
		DEATH_SUICIDE1;
		// 0 = random opponent
	} //end type
	type "death_slime" //initiated when the bot dies in slime
	{
		DEATH_SLIME0;
		DEATH_SLIME1;
		// 0 = random opponent
	} //end type
	type "death_drown" //initiated when the bot drowns
	{
		"I can't swim!";
		
		
		// 0 = random opponent
	} //end type
	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Aww crap!";
		"Shutup!";
		DEATH_SUICIDE2;
		DEATH_SUICIDE1;
		
		
		// 0 = random opponent
	} //end type
	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"D'oh!";
		DEATH_GAUNTLET0;
		DEATH_GAUNTLET1;
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to Gauntlet)
	} //end type
	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		DEATH_INSULT0;
		DEATH_INSULT5;
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to Railgun)
	} //end type
	type "death_bfg" //initiated when the bot died by a BFG
	{
		DEATH_BFG0;
		DEATH_BFG2;
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to BFG10K)
	} //end type
	type "death_insult" //insult initiated when the bot died
	{
		DEATH_INSULT2;
		DEATH_INSULT0;
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type
	type "death_praise" //praise initiated when the bot died
	{
		"Linus, you are no longer my god, ", 0, " is.";
		"Was that the new ", 1, "?";
		D_PRAISE0;
		D_PRAISE1;
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type
	//======================================================
	//======================================================
	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		KILL_RAIL1;
		KILL_RAIL0;
		// 0 = enemy name
	} //end type
	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"LOL!";
		"Too easy";
		"Try recompiling with -pipe -O12 -fweb -funswitch-loops -funroll-all-loops -funit-at-a-time -fsched2-use-superblocks -frename-registers -fprefetch-loop-arrays -finline-limit=32768 -ffast-math -fbranch-target-load-optimize2"; //may be too long
		"tsk... sad, really.  I almost hated to do that.";

	
		// 0 = enemy name
	} //end type
	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"LOL!";
		TELEFRAGGED2;
		// 0 = enemy name
	} //end type
	type "kill_insult" //insult initiated when the bot killed someone
	{
		"p0wn3d";
		"you are not 1337";
		"Bi11 G473z owns you.";
		KILL_INSULT4;
		KILL_INSULT2;
		
		// 0 = enemy name
	} //end type
	type "kill_praise" //praise initiated when the bot killed someone
	{
		PRAISE0;
		PRAISE1;
		// 0 = enemy name
	} //end type
	//======================================================
	//======================================================
	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"Your breath stinks";
		"Get a hair cut";
		"Get a real job";
		"Get a life";
		"You need 1337 lessons";
		"I so nailed your mom last night.";
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
	type "random_misc" //miscellanous chats initiated randomly
	{
		"Has anyone tried kernel 2.8.56-rc3-mm45, yet?";
		"Binary packaging encourages sloppy design, contrast with the stochastic API unit testing which occurs naturally with all the building with different configurations.";
		"It could be the purpose of your life is only to serve as a warning to others";
		"Don't put that finger there, you don't know where's it been!";
		"I miss Richard Nixon...";
		"Viva la tux!";
		GUYTALK1;
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
} //end chat

