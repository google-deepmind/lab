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

	//the bot doesn't know who someone is
type "whois"
{
"Ok, so who is ", 0, "?";
"Who in their right mind uses the name ", 0, "?";
"Who in the hell is ", 0, "?";
"Is ", 0, " a friend of yours?";
"Who the bloodyhell is ",0," .";
0, "!?! Who dat?";
"How can I kill ", 0, " when I haven't the foggiest idea who ", 0, " is?";

}

	//the bot doesn't know where someone is hanging out
type "whereis"
{
"So where is ", 0, "?";
"Ok, so where is", 0, ".";
"Would someone please tell me where ", 0, " is.";
" ", 0, " hanging out?";
"Where the hell is ", 0, "?";
"Since when am I ", 0, "'s keeper?";
} 

	//the bot asks where you are
type "whereareyou"
{
"Yo, where are you ", 0, "?";
"Hello!! ", 0, "Where are you hiding?";
"I can't find you ", 0, ". Where are you?";
"Where did you scuttle off to ", 0, "?";
"How am I supposed to find you, ", 0, "?";
}

//cannot find something
type "cannotfind"
{
"Where would that be ", 0, "?";
"Where the hell is a ", 0, "?";
"Where is a ", 0, " in this level?";
"Is there a, ", 0, " in this level? I sure can't find it, I must be blind.";
} 

	//bot tells where he/she is
type "location"
{
"By the ", 0," what are you blind?";
"I am at the ", 0;
} 

//bot tells where he/she is and near which base
type "teamlocation"
{
"I'm near the ", 0, " in the ", 1, "base.";
"By the ", 0, " in the ", 1, " base.";
} 

	//start helping
type "help_start"
{
"I'm almost there, ", 0, ", coming to help.";
"Ok, I'm on my way,", 0,".";
"Keep 'em busy, ", 0," I'm on my way.";
}

	//start accompanying
type "accompany_start"
{
"Consider me your shadow ", 0 ,".";
affirmative, "... what else have I got planned?";

}

	//stop accompanying
type "accompany_stop"
{
"It's been real, but I'm off on my own, ", 0,".";
"I'm on my own now.";
"Good luck, I'm off on my own.";
}

	//cannot find companion
type "accompany_cannotfind"
{
"Where the hell are you ", 0, "?";
"Where are you hiding ", 0, "?";
0, "... come out, come out wherever you are...";

} 

	//arrived at companion
type "accompany_arrive"
{
"At your disposal ", 0, ".";
"Finally, I've found ", 0, ".";
"Better late than never, eh?";

}
	//bot decides to accompany flag or skull carrier
type "accompany_flagcarrier"
{
"I've got your back ", 0, ".";
"They'll have to get through me to get to you, ", 0, ".";
"Following.";

}

	//start defending a key area
type "defend_start"
{
"I'll defend the ", 0, ".";
"I'm defending the ", 0, ".";

}

	//stop defending a key area
type "defend_stop"
{
"That's it, I'll stop defending the ", 0, ".";
"I am not defending the ", 0, ".";
"I am not going to defend the ", 0, " anymore.";

}

	//start getting an item
type "getitem_start"
{
"I'll get the ", 0, ".";
"I'm off to get the ", 0, ".";

}
	//item is not there
type "getitem_notthere"
{
"the ", 0, " ain't there";
"The ", 0, " seems to be missing.";
"Where is the ", 0, "?";
}
	//picked up the item
type "getitem_gotit"
{
"I got the ", 0, "!";
"The ", 0, " is mine!";

}

	//go kill someone
type "kill_start"
{
"I'm going to kill ", 0,", wish me luck.";
0, " will be toast.";
0, " is a goner.";
0, " will be given a pair of cement shoes.";
"Ok";
"What ever you say";
"Finally some fun!";
}
	//killed the person
type "kill_done"
{
"Well that was easy. ", 0, " is dead.";
0, " has been wacked.";
0, " was given cement shoes.";
0, " kicked the bucket.";
0, " was taken out.";
0, " just bought the farm.";
0, " is now just dust in the wind.";
}

	//start camping
type "camp_start"
{
"That is a good sniping position.";
"Off to pitch my tent.";
"Sounds like a plan.";
}

	//stop camping
type "camp_stop"
{
"Time to move on.";
"Sniping isn't so good here, moving on.";
}

	//in camp position
type "camp_arrive" //0 = one that ordered the bot to camp
{
"I'm there. ", 0, ", time to snipe.";
"Tent pitched, sniping at will.";

}

	//start patrolling
type "patrol_start" //0 = locations
{
"Patrolling ", 0, ".";

}

	//stop patrolling
type "patrol_stop"
{
"The patrol is over, nothing to report.";
}

	//start trying to capture the enemy flag
type "captureflag_start"
{
"Their flag will be ours.";
"Sounds like a plan.";
"I'm off to get the flag.";
"Enemy flag here I come.";

}

	//return the flag
type "returnflag_start"
{
"I'll get our flag back.";
"I'll get the flag back.";
"I will return the flag.";

}

	//attack enemy base
type "attackenemybase_start"
{
"Got it, their base is toast.";
"I'm off to destroy their base and frag a few fools.";
"Death to the infidels!";

}

	//harvest
type "harvest_start"
{
"Death to the infidels!  Their skulls will be mine!";
"Death to the infidels!";
"The infidels shall die!";
}

	//the checkpoint is invalid
type "checkpoint_invalid"
{
"That checkpoint does not exist.";
"Quit fooling around, that is not a checkpoint.";
}

	//confirm the checkpoint
type "checkpoint_confirm" //0 = name, 1 = gps
{
affirmative, " Yep ", 0, " at ", 1, " is there.";
"It looks like ", 0, " at ", 1, " is there.";
}

	//follow me
type "followme"
{
"What the hell are you waiting for ", 0, "? Get over here!";
"Hey!  ", 0, " follow me and be quick about it.";
}

	//stop leading
type "lead_stop"
{
"That's it find someone else who wants the responsibility.";
"I refuse to lead anymore.";
"I do not want to lead anymore, find someone else.";
}

	//the bot is helping someone
type "helping"
{
"I'm trying to help, ", 0, ".";
"Helping ", 0, ", care to join me?";
}

	//the bot is accompanying someone
type "accompanying"
{
"I'm shadowing ", 0, ".  Is that alright?";
"Following ", 0, ", trying not to get shot.";
}

	//the bot is defending something
type "defending"
{
"I'm defending ", 0, ".";

}

	//the bot is going for an item
type "gettingitem"
{
"I'm off to get the ", 0, ".";
"The ", 0, " is going to be mine.";
"Dibs on the ", 0, ".";
}

	//trying to kill someone
type "killing"
{
"I've been trying to kill ", 0, ".";
"Trying to wack ", 0, ".";
}

	//the bot is camping
type "camping"
{
"Toasting marshmallows and sniping scum.";
"Where I am supposed to be, camping.";
"Sniping, just like you told me.";
}

	//the bot is patrolling
type "patrolling"
{
"On patrol, can't talk now.";
}

	//the bot is capturing the flag
type "capturingflag"
{
"Gots to get the flag.";
}

	//the bot is rushing to the base
type "rushingbase"
{
"Rushing to the base.";
}

	//trying to return the flag
type "returningflag"
{
"Getting the flag back.";
}

type "attackingenemybase"
{
"I'm destroying their base!  Care to help?";
"Wreaking havok in their base.";
}

type "harvesting"
{
"Collecting skulls, what are you doing?";
}

	//the bot is just roaming a bit
type "roaming"
{
"Rambling around, fragging at whim.";
"Mindlessly roaming around, like I was told.";
"~Wacking fools piece-meal.";
}

type "wantoffence"
{
"Let me go on offense.";
"Can I be on offense?";

}

type "wantdefence"
{
"I think I can handle the big D.";
"Can I be on defense?";
}

	//the bot will keep the team preference in mind
type "keepinmind"
{
"A'ight, ", 0," I'll keep it in mind.";

}

	//==========================
	// teamplay chats
	//==========================
	//team mate killed the bot
type "death_teammate"
{
"Same team, dumbass!";
"Hey ", 0," I'm on your team... idiot!";
"Why did you kill me?";
}
	//killed by a team mate
type "kill_teammate"
{
"hehe... oops.";
"Sorry!";
"Oops, won't happen again.";
}

	//==========================
	// CTF useless chats
	//==========================

	//team mate got the enemy flag
type "ctf_gotflag"
{
"It's about time, ", 0, " now get that flag home!";
}
	//team mate gets the enemy flag to the base
type "ctf_captureflag"
{
"Sweet, gj, ", 0, ".";
}
	//team mate returns the base flag
type "ctf_returnflag"
{
"Nice assist, ", 0, ".";
} //end type
	//team mate defends the base
type "ctf_defendbase"
{
"Nice D work there, ", 0, ".";
}
	//team mate carrying the enemy flag dies
type "ctf_flagcarrierdeath"
{
"Go get our flag!";
}
	//team mate kills enemy with base flag
type "ctf_flagcarrierkill"
{
"Yo, ", 0," get our flag now!";
}

	//==========================
	// NOTE: make sure these work with match.c
	//==========================
	//ask who the team leader is
type "whoisteamleader"
{
"Who's the leader of this rag-tag bunch?";
}

	//I am the team leader
type "iamteamleader"
{
"I am the leader.";
"I shall lead.";
"I lead.";
}
	//defend the base command
type "cmd_defendbase"
{
0, " defend our base.";
0, " set up some D.";
0, " you should defend our base.";
}
	//get the enemy flag command
type "cmd_getflag"
{
"Yo, ", 0, " get the flag!";
0, " get their flag.";
0, " capture the other team's flag.";
}
	//accompany someone command
type "cmd_accompany"
{
"Hey, ", 0, " shadow ", 1;
0, " follow ", 1, ", ", 1, " needs the help.";

}
	//accompany me command
type "cmd_accompanyme"
{
0, " you should follow me.";
"Follow me please, ", 0, ".";
}
	//attack enemy base command
type "cmd_attackenemybase"
{
0, " go after their base";
}
	//return the flag command
type "cmd_returnflag"
{
0, " please return our flag!";
0, " get our flag back, ASAP!";
}
	//go harvesting
type "cmd_harvest"
{
0, ", you should collect some skulls.";
}

	//Double Domination stuff:
type "dd_start_pointa"
{
"I'll dominate point A";
}

type "dd_start_pointb"
{
"I'll dominate point B";
}

type "dd_pointa"
{
"Dominating point A";
}

type "dd_pointb"
{
"Dominating point B";
}

//DD orders:
type "cmd_takea"
{
0, " dominate point A";
}

type "cmd_takeb"
{
0, " dominate point B";
}