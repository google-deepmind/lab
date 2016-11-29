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

chat "Jenna"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"Hello.";
		HELLO1;
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "game_exit" //initiated when the bot exits the game
	{
		"Look at the ~time";
		GOODBYE1;
		GOODBYE0;
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "level_start" //initiated when a new level starts
	{
		0, " it is time for some payback!";
		1, " you are going to have some pain in ", 4, ".";
		HELLO2;
		
		// 0 = bot name
	} //end type
	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		2, ", I let you win.";
		4, " has changed a lot, it is much faster now.";
		"Is it over yet?";
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		3, " you need to practice more... and forget about winning against me.";
		LEVEL_END_VICTORY1;
		// 0 = bot name
		// 1 = random opponent
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		2, " cheated.";
		"I had a hang nail, so of course I lost.";
		"So much for my bluster...";
		"I felt bad for you so I let you win.";
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 4 = level's title
	} //end type
	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		0, ", didn't your mother ever tell you it was rude to interrupt a conversation?";
		"I am going to shove that ", 1, " down your throat if you do that again!";
		DEATH_TALKING;
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nodeath" //bot is hit by an opponent's weapon attack but didn't die; either praise or insult
	{
		1, " is not a good weapon, a ", weapon, " should be more your speed.";
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		0, " must be cheating.";
		"Well that sucked...";
		"Note to self... turn off safety next ~time";
		// 0 = opponent
	} //end type
	type "enemy_suicide" //enemy of the bot commits suicide
	{
		"Stop smoking ", substance, " and maybe you won't kill yourself so often.";
		"Tired of life, ", 0, "?  Give it up.";
		"I guess that beats dying of ", disease, "...";
		// 0 = enemy
	} //end type
	//======================================================
	//======================================================
	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"I hate praetorian impressions.";
		"My ", counselor, " warned about days like this.";
		"D'oh!";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type
	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		DEATH_FALLING0;
		"It didn't look that high from up there...";
		"*sigh*";
		// 0 = random opponent
	} //end type
	type "death_lava" //initiated when the bot dies in lava
	{
		"Well... it beats a case of hypothermia.";
		"At least it is warm here";
		DEATH_SUICIDE1;
		// 0 = random opponent
	} //end type
	type "death_slime" //initiated when the bot dies in slime
	{
		"Green has never been my color...";
		DEATH_SLIME1;
		// 0 = random opponent
	} //end type
	type "death_drown" //initiated when the bot drowns
	{
		DEATH_DROWN0;
		"It isn't that difficult to just breath water...";
		"So why can't I breath water?";
		// 0 = random opponent
	} //end type
	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		DEATH_SUICIDE2;
		"So much for life and all of its cruel irony.";
		"It is just easier for ", 0, " this way.";
		
		
		// 0 = random opponent
	} //end type
	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"Unsanitary.";
		"What part of '~Don't touch me there' do you not understand?";
		DEATH_GAUNTLET1;
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to Gauntlet)
	} //end type
	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		DEATH_INSULT0;
		DEATH_FEM_INSULT1;
		DEATH_INSULT5;
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to Railgun)
	} //end type
	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Can't find a skill weapon?  Or is it just that you suck?";
		"Over-compensating for something?";
		DEATH_BFG0;
		DEATH_BFG2;
		// 0 = enemy name
		// 1 = weapon used by enemy (NOTE: always set to BFG10K)
	} //end type
	type "death_insult" //insult initiated when the bot died
	{
		curse;
		"This is far more painful than it looks.";
		"One day, ", 0, " you me and a ", weapon, " are going to have a discussion.";
		"Is that the best you can do?";
		DEATH_INSULT2;
		DEATH_INSULT0;
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type
	type "death_praise" //praise initiated when the bot died
	{
		"Not bad, ", 0, ", have you been practicing?";
		"I never saw it coming.";
		D_PRAISE0;
		D_PRAISE1;
		// 0 = enemy name
		// 1 = weapon used by enemy
	} //end type
	//======================================================
	//======================================================
	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		0, ", you should stay down...";
		"*sigh* if only this was challenging.";
		"I love this gun!";
		"Way too easy";
		// 0 = enemy name
	} //end type
	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"LOL!";
		"Too easy";
		// 0 = enemy name
	} //end type
	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"LOL!";
		"Aww man... now I have to take a bath to get the brains out.";
		"Does anyone have a quick way of getting brains out of your hair?";
		// 0 = enemy name
	} //end type
	type "kill_insult" //insult initiated when the bot killed someone
	{
		KILL_INSULT4;
		"Why do you even bother?";
		"This is why the upper classes are called the 'ruling elite.'";
		"Why do the lower classes think they can match someone of my caliber?";
		
		// 0 = enemy name
	} //end type
	type "kill_praise" //praise initiated when the bot killed someone
	{
		"You are getting better, ", 0, ", keep it up and you might actually win something.";
		"That was a challenge.";
		// 0 = enemy name
	} //end type
	//======================================================
	//======================================================
	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		TAUNT0;
		TAUNT_FEM0;
		TAUNT1;
		curse;
		"You should give up your ", substance, " addiction by next ", month, ".";
		"I saw those pictures of ", 0, " and a ", animal, " posted on the web last night... Like, ewww!";
		immaturity01;
		"Your ", family_member, " needs to stop calling me.";
		
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
	type "random_misc" //miscellanous chats initiated randomly
	{
		GRRLTALK0;
		MISC4;
		"Has anyone tried out the new ", 5, "?  Is it worth the upgrade?";
		MISC1;
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
} //end chat

