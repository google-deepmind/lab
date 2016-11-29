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

chat "kyonshi"
{
	#include "teamplay.h"

	type "game_enter"
	{
		"Ahh, there is so much chi here... I will feed well!";
	}

	type "game_exit"
	{
		"It's not easy being dead... so much to do so little time";
		"Toodles";
		"My hunger grows, I am off to find better chi.";
		GOODBYE4;
		GOODBYE2;
	}

	type "level_start"
	{
		"Ooh ", 4, "!  I like this place!";
	}

	type "level_end"
	{
		"Not bad... I like ", 4, " fast, clean, deadly, and filled with chi.";
	}

	type "level_end_victory"
	{
		"You cannot kill what is already dead.";
		"Thank you for your chi.";
	}

	type "level_end_lose"
	{
		"Aww... crap!";
	}

	type "hit_talking"
	{
		"Do you MIND!";
		DEATH_TALKING;
		DEATH_TALKING;
	}

	type "hit_nodeath"
	{
		TAUNT_FEM0;
	}

	type "hit_nokill"
	{
		HIT_NOKILL0;
		HIT_NOKILL1;
		HIT_NOKILL1;
	}

	type "death_telefrag"
	{
		DEATH_TELEFRAGGED0;
		
	}

	type "death_cratered"
	{
		DEATH_FALLING0;
		
	}

	type "death_lava"
	{
		"So the dead can die...";
	}

	type "death_slime"
	{
		"I need a bath now.";
	}

	type "death_drown"
	{
		"Much easier than my last death.";
	}

	type "death_suicide"
	{
		DEATH_SUICIDE0;
	}

	type "death_gauntlet"
	{
		"I did not see that coming.";
		"The shame...";
		DEATH_FEM_INSULT0;
		DEATH_GAUNTLET0;
		DEATH_GAUNTLET1;
		
	}

	type "death_rail"
	{
		DEATH_RAIL1;
		DEATH_RAIL0;
		DEATH_FEM_INSULT1;
		"Your death will not be quick.";
	}

	type "death_bfg"
	{
		DEATH_BFG1;
		DEATH_FEM_INSULT2;
		"I feel you lack the skill to truly face me.";
	}

	type "death_insult"
	{
		DEATH_FEM_INSULT0;
		DEATH_FEM_INSULT2;
		DEATH_FEM_INSULT1;
		DEATH_FEM_INSULT3;
	}

	type "death_praise"
	{
		 "The NSA just told me to tell you, ", 0, ", that they liked that last move.";
		 0, ", ~you suck!";
		 "I almost didn't see it coming.";
	}

	type "kill_rail"
	{
		DEATH_RAIL1;
		DEATH_RAIL1;
		DEATH_RAIL1;
	}

	type "kill_gauntlet"
	{
		KILL_GAUNTLET0;
		KILL_GAUNTLET1;
		KILL_GAUNTLET2;
	}

	type "kill_telefrag"
	{
		TELEFRAGGED0;
		
	}

	type "kill_suicide"
	{
		TAUNT_FEM0;
		TAUNT0;
	}

	type "kill_insult"
	{
		"*sigh*";
		"Oh puh-lease!";
		"You might want to put that gun down before you hurt yourself.";
		"Pure luck.";
	}

	type "kill_praise"
	{
		"~I'm impressed.";
	}

	type "random_insult"
	{
		"An insignificant gnat will fall by the wayside.";
		"Your ~mother needs to stop calling me.";
		"Your ~mama is a necrophiliac.";
		"douchebag";
	}

	type "random_misc"
	{
		GRRLTALK0;
		GRRLTALK1;
		"Do you ever get that not so fresh feeling?  I do, but I am a corpse.";
		0, " die already so I can feast on your chi!";
		"~They're out there watching you as we speak.";
		"Shh... ~they're listening.";
		
		
	}
}
