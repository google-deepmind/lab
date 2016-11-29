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

chat "Neko"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"mew :3";
		"Are you prepared for pain?";
		HELLO1;
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "game_exit" //initiated when the bot exits the game
	{
		"Look at the ~time";
		4, " kinda wears on me, I think I'll find a different server, later ~peeps";
		GOODBYE1;
		GOODBYE0;
		// 0 = bot name
		// 1 = random opponent
		// 4 = level's title
	} //end type
	type "level_start" //initiated when a new level starts
	{
		0, " it is time for some payback!";
		1, " youll death, i'm expects it in ", 4, " lol";
		HELLO2;
		
		// 0 = bot name
	} //end type
	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"HA! ", 3, " is at the bottom, as usual.";
		2, ", I let you win.";
		4, " is can see improved.";
		"Is it over yet?";
		"...";
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		3, " nice try you roose";
		"The cat out of bag";
		"<=^_^=>";
		// 0 = bot name
		// 1 = random opponent
		// 3 = opponent in last place
		// 4 = level's title
	} //end type
	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		2, " is real loser I not!";
		"The cat in bag";
		"...";
		"hS";
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 4 = level's title
	} //end type
	//======================================================
	//======================================================
	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		0, " that was rude ^._.^";
		"I do not like is the interruption";
		"What if i came to you house and peek in you shower would you like that?";
		"What if i came to you dinner and at the steal of you food?";
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nodeath" //bot is hit by an opponent's weapon attack but didn't die; either praise or insult
	{
		"No!";
		"Yes!";
		// 0 = shooter
		// 1 = weapon used by shooter
	} //end type
	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		0, " go down";
		"*HISS*";
		"...";
		// 0 = opponent
	} //end type
	type "enemy_suicide" //enemy of the bot commits suicide
	{
		"Cat got you tongue!";
		"...";
		"...";
		// 0 = enemy
	} //end type
	//======================================================
	//======================================================
	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"do not want";
		"...";
		"...";
		DEATH_TELEFRAGGED1;
		// 0 = enemy name
	} //end type
	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		DEATH_FALLING0;
		"Aw, and I randed on my feet too!";
		">_<";
		// 0 = random opponent
	} //end type
	type "death_lava" //initiated when the bot dies in lava
	{
		"I am too hot!!";
		"Fur bad on lava";
		DEATH_SUICIDE1;
		// 0 = random opponent
	} //end type
	type "death_slime" //initiated when the bot dies in slime
	{
		"I don't like to take a srime bath!";
		DEATH_SLIME1;
		// 0 = random opponent
	} //end type
	type "death_drown" //initiated when the bot drowns
	{
		DEATH_DROWN0;
		"*HISS* I HATE WATER";
		"WATER NO";
		// 0 = random opponent
	} //end type
	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		DEATH_SUICIDE2;
		"...";
		"Oops. Ah well one less live of many to go!";
		
		
		// 0 = random opponent
	} //end type
	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"*HISS*";
		"You tore my special leather laced top, now i'm naked :(";
		"That is a wrong way to pet a cat";
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
	
		
		// 0 = enemy name
	} //end type
	type "kill_praise" //praise initiated when the bot killed someone
	{
		"You are getting better, ", 0, ", keep it up and you might actually win something.";
	
		// 0 = enemy name
	} //end type
	//======================================================
	//======================================================
	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		TAUNT0;
		TAUNT_FEM0;
		TAUNT1;
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
		"this one time, at band camp...";
		GRRLTALK1;
		MISC4;
		"Has anyone tried out the new ", 5, "?  Is it worth the upgrade?";
		MISC1;
		"You would be surprised what you can do with a ", animal, " a ", food, " and ", peeps, ".";
		"Next time you see ", peeps, " tell 'em to call me, mmmkay?";
		neuterbot, " is cute.";
		// 0 = name of randomly chosen player
		// 1 = name of the last player killed by this bot
		// 4 = level's title
		// 5 = random weapon from weapon list
	} //end type
} //end chat

