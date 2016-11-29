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


chat "gargoyle"
{
	#include "teamplay.h"

	type "game_enter"
	{
		"I hope that you are all ready to die, because you will.";
		"Ladies and Gentlemen, the word is pain.  Time to spread the word.";
		"...clowns to the left of me, jokers to the right, here I am stuck in the middle with you...";
		
	}

	type "game_exit"
	{
		"~It's been real and ~it's been fun...";
		"Sorry ~that's your ~Mom on the phone, I have to take this.";
		"Look at the time... later peeps.";
		
	}

	type "level_start"
	{
		4, "was a great place before they screwed it up.";
		4, "... again?!?";
		"I can remember ", 4, " when it didn't suck.";
		"Dibs on the railgun!";
	}

	type "level_end"
	{
		"*Yawn*... thought it would never end.";
		"Well that was almost interesting.";
		"What?  No dancing girls?  Where's my agent?!?";
		
	}

	type "level_end_victory"
	{
		"As usual... You still suck.";
		"Things are as they should be.";
		"Why do you even try?";
		"Did you really think that you could beat me?";
	}

	type "level_end_lose"
	{
		"Ya Basta!";
		"Two out of three?";
		"I was robbed!";
		"My heart wasn't in it, it was on a small planet somewhere in the vicinity of Beatlegeuse.";
		
	}

	type "hit_talking"
	{
		"Look jerkwad I was trying to hold a conversation!";
		"Do that again and ~I'll sell your genitals on EBay.";
		"Jerk.";
		"The chatbox makes a great target, doesn't it?";
	}

	type "hit_nodeath"
	{
		"Son of a...";
		"Time to go back to the firing range.";
		"I ~can't believe you survived that.";
		"I need a bigger gun.";
		
	}

	type "hit_nokill"
	{
		"Try harder.";
		"Was the safety off?";
		"HA!";
		"Try aiming.";
	}

	type "death_telefrag"
	{
		"Stupid praetorians!";
		"I thought all of the praetorians were killed in the last invasion of their home world?";
		"That was a terrible praetorian impersonation.";
		"I'm telling your ", family_member, ".";
		"Didn't your ", counselor, " tell you not to do that anymore?";
	}

	type "death_cratered"
	{
		"Aww crap!";
		"I had to give ", 0, " a chance.";
		"It didn't look that high from up there.";
		"I should learn to use these wings.";
		"Hehe... oops.";
	}

	type "death_lava"
	{
		"Anyone got any marshmallows?";
		"Ahh... warmth at last... ooh hot! hot! hot!";
		"Well... ", 0, " needed the help.";
		"So much for swimming in lava.";
	}

	type "death_slime"
	{
		"Well it looked safer than that...";
		"Who put this toxic waste here?";
		"That should be labeled toxic";
		"Where is Rob Reiner when you need him?";
	}

	type "death_drown"
	{
		"Ahh... sweet oblivion.";
		"I thought Leileilol gave me gills!";
		"I guess I can't breathe water";
		"So much for that indestructibility complex.";
	}

	type "death_suicide"
	{
		"Just seeing what it was like.";
		"Karma.";
		"Oops...";
		"I give up.";
	}

	type "death_gauntlet"
	{
		"Zagged when I should have zigged.";
		"That is humiliating.";
		"*sigh*";
		"Don't touch me there!";
	}

	type "death_rail"
	{
		"I never heard it.";
		"I thought I would hear it.";
		"Who'd a thunk it?";
		"Punk!";
	}

	type "death_bfg"
	{
		"*sigh* try a real weapon next time.";
		"Awful big gun for such a puny wimp...";
		"Can't hack it with a real weapon?";
		"Sissy!";
	}

	type "death_insult"
	{
		"Oh please, my ", family_member, " could do better than that.";
		"took you long enough...";
		"Is that all you've got?";
		curse;
		"You should practice more.";
	}

	type "death_praise"
	{
		"You are better than I thought, ", 0, ".";
		"I let you frag me.";
		"Great shot.";
		"Nice ~one";
		"Looks like you are on your way to arena mastery.";
	}

	type "kill_rail"
	{
		"Set 'em up and snipe them down.";
		"Like shooting fish in a barrel.";
		"Too easy.";
		"How do you like me now?";
	}

	type "kill_gauntlet"
	{
		"HA!";
		"I told you so.";
		"Mmmm hmmm.";
		"So much for your bluster.";
		"Do I make myself clear?";
		KILL_GAUNTLET1;
		KILL_GAUNTLET0;
	}

	type "kill_telefrag"
	{
		"Heh... praetorians rule!";
		"You are paying for the dry cleaning bill.";
		"You will be cleaning that up, right?";
		"Hehe";
		TELEFRAGGED0;
		TELEFRAGGED1;
		
	}

	type "kill_suicide"
	{
		
		TAUNT1;
		TAUNT0;
	}

	type "kill_insult"
	{
		"Pathetic";
		"looser";
		"Eesh!  You need practice.";
		"Too easy.";
		KILL_EXTREME_INSULT;
		curse;
	}

	type "kill_praise"
	{
		"I had to work for that one.";
		"You are improving.";
		"Not bad, Major will have her work cut out for her.";
		D_PRAISE1;
	}

	type "random_insult"
	{
		"I've seen bacteria with a better chance of getting a frag than you, ", 0, ".";
		peeps, " is better.";
		number, " US dollars says you don't win.";
		peeps, " needs to stop calling my ~Mom.";
	}

	type "random_misc"
	{
		"What ever happened to polite discourse?";
		"You know, I really do want to be anarchy... how do I go about that?";
		"Ever do any fishing?";
		"So, how about them Cubs?";
		"This is the year that the Cubs win the World Series!";
	}
}
