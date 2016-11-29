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

#include "match.h"

#define PURE_CRAP	2
#define MORE_CRAP	2



// this is rare but people can always fuckup
// don't use EC"(", EC")", EC"[", EC"]" or EC":" inside player names
// don't use EC": " inside map locations

//entered the game message
MTCONTEXT_MISC
{
	
	NETNAME, " has joined the game" = (MSG_ENTERGAME, 0);
	NETNAME, " is the team leader" = (MSG_NEWLEADER, 0);
} 

//team command chat messages
MTCONTEXT_INITIALTEAMCHAT
{
	//help and/or meet someone
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": help "|" meet ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_HELP, ST_NEARITEM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": help "|" meet ", TEAMMATE = (MSG_HELP, ST_SOMEWHERE);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " help "|" meet ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_HELP, $evalint(ST_NEARITEM|ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " help "|" meet ", TEAMMATE = (MSG_HELP, $evalint(ST_SOMEWHERE|ST_ADDRESSED));

	//accompany someone (and meet at the rendezvous point) ("hunk follow me", "hunk go with babe", etc.)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "go with "|"follow "|"cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_NEARITEM|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "go with "|"follow "|"cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_ACCOMPANY, ST_NEARITEM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "go with "|"follow "|"cover "|" protect ", TEAMMATE, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_SOMEWHERE|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "go with "|"follow "|"cover "|" protect ", TEAMMATE = (MSG_ACCOMPANY, ST_SOMEWHERE);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go with "|" follow "|" cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_NEARITEM|ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go with "|" follow "|" cover "|" protect ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_ACCOMPANY, $evalint(ST_NEARITEM|ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go with "|" follow "|" cover "|" protect ", TEAMMATE, " for", TIME = (MSG_ACCOMPANY, $evalint(ST_SOMEWHERE|ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go with "|" follow "|" cover "|" protect ", TEAMMATE = (MSG_ACCOMPANY, $evalint(ST_SOMEWHERE|ST_ADDRESSED));

	//preference
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to ", "be "|"", "on "|"", "defense"|"defend" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to capture ", "the "|"their ", "Red Flag"|"Blue Flag"|"flag" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to ", "get "|"capture ", "the "|"their ", "Red Flag"|"Blue Flag"|"flag" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to ", "attack"|"assault" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to be ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will defend", ""|"the base"|"the flag"|"the obelisk" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will not harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_DEFENDER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I am ", "on "|"", "defense" = (MSG_TASKPREFERENCE, ST_DEFENDER);

	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will not defend", ""|"the flag"|"the base"|"the obelisk" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I do not want to be ", "on "|"", "defense" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to get ", "the "|"their", "Red Flag"|"Blue Flag"|"flag" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to ", "attack"|"assault", ""|" the "|"their ", "Red Flag"|"Blue Flag"|"Red Obelisk"|"Blue Obelisk"|"base"|"" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to be ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will ", "attack"|"assault", ""|" the "|" their ", "Red Obelisk"|"Blue Obelisk"|"base"|"" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will be ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I will harvest", " skulls"|" cubes"|"" = (MSG_TASKPREFERENCE, ST_ATTACKER);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I am ", "on "|"", "offense" = (MSG_TASKPREFERENCE, ST_ATTACKER);

	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I want to roam" = (MSG_TASKPREFERENCE, ST_ROAMER);
	

	//get the flag
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": capture ", "the blue "|"the enemy "|"the red "|"their "|"the "|"enemy ", "flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": get ", "the blue "|"the red "|"the enemy "|"their "|"the "|"enemy ", "flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " capture ", "the blue "|"the enemy"|"the red "|"their "|"the "|"enemy ", "flag" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " get ", "the blue "|"the enemy"|"the red "|"their "|"the "|"enemy ", "flag" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": get ", PURE_CRAP, " flag ", MORE_CRAP = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": kill the flag carrier" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": kill the flag" = (MSG_GETFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " kill the flag carrier" = (MSG_GETFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " kill the flag" = (MSG_GETFLAG, ST_ADDRESSED);

	//attack the enemy base
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " attack ", "enemy "|"the enemy "|"the red "|"the blue "|"their ", "base"|"flag"|"obelisk" = (MSG_ATTACKENEMYBASE, ST_ADDRESSED);

	//go harvesting
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " harvest" = (MSG_HARVEST, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go harvesting" = (MSG_HARVEST, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " collect ","skulls"|"cubes" = (MSG_HARVEST, ST_ADDRESSED);
	
	

	//kill someone (NOTE: make sure these are after the get flag match templates because of the "kill"
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": kill ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " kill ", ENEMY = (MSG_KILL, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wack ", ENEMY = (MSG_KILL, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " take out ", ENEMY = (MSG_KILL, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": hunt down ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": kill ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": wack ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": take out ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": death to ", ENEMY = (MSG_KILL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " hunt down ", ENEMY = (MSG_KILL, ST_ADDRESSED);

	//get item
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " grab the "|" go grab the ", ITEM = (MSG_GETITEM, ST_ADDRESSED);
	

	//defend/guard a key area
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "defend "|"guard ", "the checkpoint "|"the waypoint ", KEYAREA, " for", TIME = (MSG_DEFENDKEYAREA, ST_TIME);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "defend "|"guard ", "the checkpoint "|"the waypoint ", KEYAREA = (MSG_DEFENDKEYAREA, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " defend "|" guard ", "the ", "checkpoint "|"waypoint ", KEYAREA, " for", TIME = (MSG_DEFENDKEYAREA, $evalint(ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " defend "|" guard ", "the checkpoint "|"the waypoint ", KEYAREA = (MSG_DEFENDKEYAREA, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " defend the "|" guard the ", KEYAREA = (MSG_DEFENDKEYAREA, ST_ADDRESSED);

	//camp somewhere ("hunk camp here", "hunk camp there", "hunk camp near the rl", etc.)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "there "|"over there ", " for", TIME = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_TIME|ST_THERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "there"|"over there" = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_THERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "here"|"over here ", " for", TIME = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_TIME|ST_HERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "here"|"over here" = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_HERE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "near "|"at "|"", "the "|"checkpoint "|"waypoint ", KEYAREA, " for", TIME = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_NEARITEM|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " camp ", "near "|"at ", "the checkpoint "|"the waypoint ", KEYAREA = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_NEARITEM));
	//go to (same as camp)
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go to ", "the "|"checkpoint "|"waypoint ", KEYAREA = (MSG_CAMP, $evalint(ST_ADDRESSED|ST_NEARITEM));

	//Double Domination orders
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "take"|"dominate"|"hold", " point A"|" red base" = (MSG_TAKEA, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "take"|"dominate"|"hold", " point B"|" blue base" = (MSG_TAKEB, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " take"|" dominate"|" hold", " point A"|"red base" = (MSG_TAKEA, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " take"|" dominate"|" hold", " point B"|"blue base" = (MSG_TAKEB, ST_ADDRESSED);

	//rush to the base in CTF
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " rush ", "to "|"to the "|"the ", "base" = (MSG_RUSHBASE, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " go ", "to"|"to the", " base" = (MSG_RUSHBASE, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "rush base" = (MSG_RUSHBASE, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "return to base" = (MSG_RUSHBASE, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "go to base" = (MSG_RUSHBASE, 0);

	//return the flag
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " return ", PURE_CRAP, " flag" = (MSG_RETURNFLAG, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": return ", MORE_CRAP, " flag", PURE_CRAP = (MSG_RETURNFLAG, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": return the", MESSAGE, " flag", PURE_CRAP = (MSG_RETURNFLAG, 0);


	//who is the team leader
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": who is ", "the leader"|"the team leader"|"team leader"|"leader", "?"|"" = (MSG_WHOISTEAMLAEDER, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": is there a ", "leader"|"team leader", "?"|"" = (MSG_WHOISTEAMLAEDER, 0);

	//become the team leader
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " will be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " want to be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " wants to be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " is ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " you are ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " will be ", THE_TEAM, "leader" = (MSG_STARTTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I am ", "the leader"|"the team leader"|"team leader"|"leader" = (MSG_STARTTEAMLEADERSHIP, ST_I);
	

	//stop being the team leader
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", TEAMMATE, " is not ", THE_TEAM, "leader" = (MSG_STOPTEAMLEADERSHIP, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "I quit being leader"|"I no longer lead"|"i stop being the leader" = (MSG_STOPTEAMLEADERSHIP, ST_I);
	

	//wait for someone
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for me", " near "|" at ", "the "|"checkpoint "|"waypoint ", ITEM = (MSG_WAIT, $evalint(ST_NEARITEM|ST_ADDRESSED|ST_I));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for me" = (MSG_WAIT, $evalint(ST_SOMEWHERE|ST_ADDRESSED|ST_I));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for ", TEAMMATE, " near "|" at ", "the "|"checkpoint "|"waypoint "|"", ITEM = (MSG_WAIT, $evalint(ST_NEARITEM|ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " wait for ", TEAMMATE = (MSG_WAIT, $evalint(ST_SOMEWHERE|ST_ADDRESSED));

	//ask what someone/everyone is doing
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " report" = (MSG_WHATAREYOUDOING, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": report" = (MSG_WHATAREYOUDOING, 0);
	

	//ask the team leader what to do
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what is my command", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what should I do", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what am I supposed to do", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": what is my job", "?"|"" = (MSG_WHATISMYCOMMAND, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what is my command", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what should I do", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what am I supposed to do", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what is my job", "?"|"" = (MSG_WHATISMYCOMMAND, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": orders", "?"|"" = (MSG_WHATISMYCOMMAND, 0);

	//ask where someone or everyone is
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " where are you", "?"|"" = (MSG_WHEREAREYOU, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": where are you ", ADDRESSEE, "?"|"" = (MSG_WHEREAREYOU, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": where is ", ADDRESSEE, "?"|"" = (MSG_WHEREAREYOU, ST_ADDRESSED);

	//join a sub team
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " create squad ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " join squad ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " you"|" we", " are in", " squad ", TEAMNAME = (MSG_JOINSUBTEAM, ST_ADDRESSED);

	//leave a sub team
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " leave your squad" = (MSG_LEAVESUBTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " disband" = (MSG_LEAVESUBTEAM, ST_ADDRESSED);

	//what team are you in
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " which "|" what ", "squad", " are you ", "in"|"on", "?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " what is your ", "squad","?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " are you ", "in"|"on", " a squad","?"|"" = (MSG_WHICHTEAM, ST_ADDRESSED);

	//dismiss
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " stop action"|" roam"|" dismissed" = (MSG_DISMISS, ST_ADDRESSED);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "cancel order"|"roam" = (MSG_DISMISS, 0);

	//remember checkpoint
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", "checkpoint "|"waypoint ", NAME, " is at ", POSITION = (MSG_CHECKPOINT, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " checkpoint "|" waypoint ", NAME, " is at ", POSITION = (MSG_CHECKPOINT, ST_ADDRESSED);

	//patrol
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": patrol ", "from "|"", KEYAREA, " for", TIME = (MSG_PATROL, ST_TIME);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": patrol ", "from "|"", KEYAREA = (MSG_PATROL, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " patrol ", "from "|"", KEYAREA, " for", TIME = (MSG_PATROL, $evalint(ST_ADDRESSED|ST_TIME));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " patrol ", "from "|"", KEYAREA = (MSG_PATROL, ST_ADDRESSED);

	
	//lead the way
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": lead the way" = (MSG_LEADTHEWAY, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I'll follow" = (MSG_LEADTHEWAY, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": lead the way ", ADDRESSEE = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": I'll follow ", ADDRESSEE = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " lead the way" = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": lead ", TEAMMATE, " around ", ADDRESSEE = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED|ST_SOMEONE));
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " lead ", TEAMMATE, "" = (MSG_LEADTHEWAY, $evalint(ST_ADDRESSED|ST_SOMEONE));

	// suicide
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": suicide" = (MSG_SUICIDE, 0);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, EC": ", ADDRESSEE, " suicide" = (MSG_SUICIDE, $evalint(ST_ADDRESSED));
	
	//anti-bigot bot suicide code (I hope I am not acting as the thought police...)
	
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, " nigger "|" nigr "|" niger "|"wop"|"macaca"|"monkey", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "kyke"|"kykes"|"kike"|"kikes"|"jewish pig"|"judan"|"jews"|"jew"|"jew lover"|"ex-slaves"|"slaves"|"slave"|"ex-slave"|"xslave"|"xslaves"|"red sea pedestrians"|"red sea pedestrian", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "niggers"|"nigrs"|"nigers"|"wops"|"macacas", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "rag head"|"raghead"|"rag-head"|"camel jockey"|"cml jky"|"cmljky", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "rag heads"|"ragheads"|"rag-heads"|"camel jokeys"|"cml jkys"|"cmljkys", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "towel head"|"towel heads"|"towel-head"|"towel-heads"|"towelhead"|"towelheads", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "homo"|"gay"|"gaygrrl"|"gaygirl"|"faggit"|"gay-girl"|"gay-grrl", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "faggot"|"fagit"|"fagot"|"queer"|"gayboy"|"gayboi", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "dyke"|"dike"|"lez"|"les"|"lezbo"|"lesbo"|"fags"|"fagz", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "homos"|"dykes"|"dikes"|"lesbos"|"lezbos"|"fagots"|"faggots"|"faggits"|"fagits", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE,  "sand-nigger"|"sand-niger"|"sndngr"|"sand-niggers"|"sand-nigers"|"sndngrs", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "red skins"|"red-skins"|"braves"|"chiefs"|"red skin"|"red-skin"|"chief", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "bean-picker"|"bean picker"|"beanpicker"|"wet back"|"wetback"|"wet-back", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", "bean-pickers"|"beanpickers"|"bean pickers"|"wet backs"|"wetbacks"|"wet-backs", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "spook"|"gook"|"slant"|"mamasan", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	EC"("|EC"[", NETNAME, EC")"|EC"]", PLACE, "spooks"|"gooks"|"slants"|"mamasans"|"hitler", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	// not a good solution but it does work in certain situations...
	NETNAME, EC": ", MORE_CRAP,  "sand-nigger"|"sand-niger"|"sndngr"|"sand-niggers"|"sand-nigers"|" sndngrs ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, "red skins"|"red-skins"|" braves "|" chiefs "|"red skin"|" red-skin "|" chief ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " bean-picker "|" bean picker "|" beanpicker "|"wet back"|" wetback "|"wet-back", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " bean-pickers "|" beanpickers "|"bean pickers"|"wet backs"|" wetbacks "|"wet-backs", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " spook "|" gook "|" slant "|" mamasan ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " spooks "|" gooks "|" slants "|" mamasans ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " homo "|" gay "|" gaygrrl "|" gaygirl "|" gay-girl "|" gay-grrl "|" faggit ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " faggot "|" fagit "|" fagot "|" queer "|" gayboy "|" gayboi ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " dyke "|" dike "|" lez "|" les "|" lezbo "|" lesbo "|" fags "|" fagz ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " homos "|" dykes "|" dikes "|" lesbos "|" lezbos "|" faggots "|" fagots "|" faggits "|" fagits ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " rag head"|" raghead"|" rag-head"|" camel jockey"|" cml jky"|" cmljky", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " rag heads"|" ragheads "|" rag-heads"|" camel jokeys"|" cml jkys", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " towel head"|" towel heads"|" towel-head"|" towel-heads"|" towelhead"|" towelheads", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " nigrs "|" nigers "|"wops"|" macacas ", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " kyke "|" kykes "|" kike "|" kikes "|"jewish pig"|"judan"|" jew "|" jews "|"jew lover"|"ex-slaves"|"slaves"|"slave"|"ex-slave"|"xslave"|"xslaves"|"red sea pedestrians"|"red sea pedestrian", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);
	NETNAME, EC": ", MORE_CRAP, " nigger"|" nigr "|" niger "|"wop"|" macaca "|" monkey"|" hitler", PURE_CRAP = (MSG_SUICIDE, ST_TEAM);

} 

MTCONTEXT_CTF
{
	NETNAME, " got the ", FLAG, " flag", "!"|"" = (MSG_CTF, ST_GOTFLAG);
	NETNAME, " captured the ", FLAG, " flag", "!"|"" = (MSG_CTF, ST_CAPTUREDFLAG);
	NETNAME, " returned the ", FLAG, " flag", "!"|"" = (MSG_CTF, ST_RETURNEDFLAG);
	//for One Flag CTF
	NETNAME, " got the flag", "!"|"" = (MSG_CTF, ST_1FCTFGOTFLAG);
} 

MTCONTEXT_TIME
{
	TIME, " minute"|" min","s"|"" = (MSG_MINUTES, 0);
	TIME, " second"|" sec","s"|"" = (MSG_SECONDS, 0);
	"ever" = (MSG_FOREVER, 0);
	" a long time" = (MSG_FORALONGTIME, 0);
	" a while" = (MSG_FORAWHILE, 0);
} 

MTCONTEXT_PATROLKEYAREA
{
	"the "|"checkpoint "|"waypoint "|"", KEYAREA, " to "|" and ", MORE = (MSG_PATROLKEYAREA, ST_MORE);
	"the "|"checkpoint "|"waypoint "|"", KEYAREA, " and loop"|" and back", " to the start"|"" = (MSG_PATROLKEYAREA, ST_BACK);
	"the "|"checkpoint "|"waypoint "|"", KEYAREA, " and reverse" = (MSG_PATROLKEYAREA, ST_REVERSE);
	"the "|"checkpoint "|"waypoint "|"", KEYAREA = (MSG_PATROLKEYAREA, 0);
} 

MTCONTEXT_TEAMMATE
{
	"me"|"I" = (MSG_ME, 0);
} 

MTCONTEXT_ADDRESSEE
{
	"Everyone"|"Everybody" = (MSG_EVERYONE, 0);
	TEAMMATE, " and "|", "|","|" ,", MORE = (MSG_MULTIPLENAMES, 0);
	TEAMMATE = (MSG_NAME, 0);
} 

MTCONTEXT_REPLYCHAT
{
	EC"(", NETNAME, EC")", PLACE, EC": ", MESSAGE = (MSG_CHATTEAM, 0);
	EC"[", NETNAME, EC"]", PLACE, EC": ", MESSAGE = (MSG_CHATTELL, 0);
	// included for peace of mind
	NETNAME, EC": ", MESSAGE = (MSG_CHATALL, 0);
}

