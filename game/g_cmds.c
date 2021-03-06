/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"

extern SP_monster_soldier_light_2(edict_t* self);
extern SP_monster_berserk_2(edict_t* self);
extern SP_monster_gunner_2(edict_t* self);
extern SP_monster_infantry_2(edict_t* self);
extern SP_monster_flyer_2(edict_t* self);
/*
==================
MOD2
P_ProjectSource

Added to help with firing projectiles as spells

==================
*/
static void P_ProjectSource(gclient_t* client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy(distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource(point, _distance, forward, right, result);
}

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}


/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

/*
==================
MOD2
Cmd_Spell_Fire_f

Casts fire ball

argv(0) fire
==================
*/
void Cmd_Spell_Fire_f(edict_t* ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage = 120;
	float	damage_radius = 120;
	int		radius_damage = 120;

	if (ent->client->pers.mp >= 30) 
	{
		gi.cprintf(ent, PRINT_HIGH, "Casting Fire\n");
		AngleVectors(ent->client->v_angle, forward, right, NULL);

		VectorScale(forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;

		VectorSet(offset, 8, 8, ent->viewheight - 8);
		P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
		fire_rocket(ent, start, forward, damage, 650, damage_radius, radius_damage);
		ent->client->pers.mp -= 30;
	}
	else 
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough MP\n");
	}
	
	
}

/*
==================
MOD2
Cmd_Spell_Void_f

Casts void

argv(0) void
==================
*/
void Cmd_Spell_Void_f(edict_t* ent)
{
	if (ent->client->pers.mp >= 60) 
	{
		vec3_t	offset, start;
		vec3_t	forward, right;
		int		damage = 200;
		float	damage_radius = 500;

		gi.cprintf(ent, PRINT_HIGH, "Casting Void\n");
		AngleVectors(ent->client->v_angle, forward, right, NULL);
		VectorScale(forward, -2, ent->client->kick_origin);
		VectorSet(offset, 8, 8, ent->viewheight - 8);
		P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
		fire_bfg(ent, start, forward, damage, 500, damage_radius);
		ent->client->pers.mp -= 60;
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough MP\n");
	}
	
}

/*
==================
MOD2
Cmd_Spell_Aero_f

Casts Air Cutter

argv(0) air
==================
*/
void Cmd_Spell_Aero_f(edict_t* ent)
{
	if (ent->client->pers.mp >= 15) 
	{
		vec3_t		start;
		vec3_t		forward, right;
		vec3_t		offset;
		int			kick = 1;

		gi.cprintf(ent, PRINT_HIGH, "Casting Air Cutter\n");
		AngleVectors(ent->client->v_angle, forward, right, NULL);

		VectorScale(forward, -3, ent->client->kick_origin);
		ent->client->kick_angles[0] = -3;

		VectorSet(offset, 0, 7, ent->viewheight - 8);
		P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
		fire_rail(ent, start, forward, 15, kick);
		ent->client->pers.mp -= 15;
	}
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough MP\n");
	}

}

/*
==================
MOD2
Cmd_Spell_Dark_Blaze_f

Casts Dark Blaze. Makes the enemy explode

argv(0) blaze
==================
*/
void Cmd_Spell_Dark_Blaze_f(edict_t* ent)
{
	if (ent->client->pers.mp >= 150) 
	{
		//
		//PLAYER
		//
		vec3_t		start, forward;
		vec3_t		end;
		trace_t		tr;

		//ENEMY
		vec3_t		m_start, m_vec;
		vec3_t		m_forward, m_right;
		vec3_t		dir;

		gi.cprintf(ent, PRINT_HIGH, "Dark Blaze Casted\n");
		VectorCopy(ent->s.origin, start); //First make a copy of vector
		start[2] += ent->viewheight; //Adds ent->viewheight to z value of start | meant to align to viewheight of player for hitscan
		AngleVectors(ent->client->v_angle, forward, NULL, NULL); //Assignning respective x value of vector into forward
		VectorMA(start, 8192, forward, end); //setting end to start + 8192 * forward
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT); //Tracing into entity from start to end
		if (tr.ent && ((tr.ent->svflags & SVF_MONSTER) || tr.ent->client))
		{
			AngleVectors(tr.ent->s.angles, m_forward, m_right, NULL);
			G_ProjectSource(tr.ent->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_1], m_forward, m_right, m_start);
			VectorCopy(tr.ent->s.origin, m_vec);
			m_vec[2] += ent->viewheight;
			VectorSubtract(m_vec, m_start, dir);
			VectorNormalize(dir);
			//m_start[0] += 7;

			//monster_fire_rocket(tr.ent, m_start, dir, 200, 10, MZ2_BOSS2_ROCKET_1);
			fire_rocket_dark(ent, m_start, dir, 200, 500, 200, 200);
			ent->client->pers.mp -= 150;

		}

	}
	else 
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough MP\n");
	}
	
}

/*
==================
MOD2
Cmd_Spell_TP_f

Casts Teleport. Teleport to enemy

argv(0) blaze
==================
*/
void Cmd_Spell_TP_f(edict_t* ent)
{
	//
	//SPELL PROPERTIES
	//
	char *spell_element = "light";
	int damage = 50;

	//
	//PLAYER
	//
	vec3_t		start, forward;
	vec3_t		end;
	trace_t		tr;

	//
	//ENEMY
	//
	vec3_t		m_start, m_vec;
	vec3_t		m_forward, m_right;
	vec3_t		dir;

	if (ent->client->pers.mp >= 50)
	{

		VectorCopy(ent->s.origin, start); //First make a copy of vector
		start[2] += ent->viewheight; //Adds ent->viewheight to z value of start | meant to align to viewheight of player for hitscan
		AngleVectors(ent->client->v_angle, forward, NULL, NULL); //Assignning respective x value of vector into forward
		VectorMA(start, 8192, forward, end); //setting end to start + 8192 * forward
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT); //Tracing into entity from start to end
		if (tr.ent && ((tr.ent->svflags & SVF_MONSTER) || tr.ent->client))
		{
			//Teleport the player
			AngleVectors(tr.ent->s.angles, m_forward, m_right, NULL);
			G_ProjectSource(tr.ent->s.origin, monster_flash_offset[MZ2_BOSS2_ROCKET_1], m_forward, m_right, m_start); //Trying to get the attacking vector of origin of monster
			VectorCopy(tr.ent->s.origin, m_vec);
			m_vec[2] += ent->viewheight;
			VectorSubtract(m_vec, m_start, dir);
			VectorNormalize(dir);
			VectorSet(ent->s.origin, m_start[0], m_start[1], m_start[2]); //Moves player to the identified monster
			vectoangles(dir, ent->client->v_angle); //Supposed to fix the player view to it looks at the monster when teleported
			
			//Calculate damage to be applied
			if (tr.ent->element != NULL) {
				C_ElementDamage_NoEnt_Mod(spell_element, tr.ent, &damage);
				gi.cprintf(ent, PRINT_HIGH, "Casting Blink | Dealing %d %s damage\n", damage, spell_element);
			}

			//Do damage
			T_Damage(tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage, 1, DAMAGE_ENERGY, MOD_UNKNOWN); //might crash

			if (tr.ent->element != NULL) {
				
			}
			ent->client->pers.mp -= 50;

		}
	}
	else 
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough MP\n");
	}


}

/*
==================
Cmd_myMonster_f

Spawns a light soldier in front of the player
==================
*/
void Cmd_myMonster_f(edict_t* ent)
{
	edict_t* mons_ent;
	vec3_t	start, forward, end;
	char* name;
	qboolean noMonster = false;

	//Getting parameter
	name = gi.args();

	//Calculating place in front of player view
	VectorCopy(ent->s.origin, start); //First make a copy of vector
	start[2] += ent->viewheight; //Adds ent->viewheight to z value of start | meant to align to viewheight of player for hitscan
	AngleVectors(ent->client->v_angle, forward, NULL, NULL); //Assignning respective x value of vector into forward
	VectorMA(start, 90, forward, end); //setting end to start + 8192 * forward

	//Deciding the monster
	mons_ent = G_Spawn();
	VectorCopy(end, mons_ent->s.origin);
	if (Q_stricmp(name, "soldier") == 0) 
	{
		SP_monster_soldier_light_2(mons_ent);
		gi.cprintf(ent, PRINT_HIGH, "Spawned Soldier with element ");
		gi.cprintf(ent, PRINT_HIGH, mons_ent->element);
		gi.cprintf(ent, PRINT_HIGH, " | HP: %d", mons_ent->health);
		gi.cprintf(ent, PRINT_HIGH, "\n");
	}
	else if (Q_stricmp(name, "berserk") == 0) 
	{
		SP_monster_berserk_2(mons_ent);
		gi.cprintf(ent, PRINT_HIGH, "Spawned Berserk with element ");
		gi.cprintf(ent, PRINT_HIGH, mons_ent->element);
		gi.cprintf(ent, PRINT_HIGH, " | HP: %d", mons_ent->health);
		gi.cprintf(ent, PRINT_HIGH, "\n");
	}
	else if (Q_stricmp(name, "gunner") == 0) 
	{
		SP_monster_gunner_2(mons_ent);
		gi.cprintf(ent, PRINT_HIGH, "Spawned Gunner with element ");
		gi.cprintf(ent, PRINT_HIGH, mons_ent->element);
		gi.cprintf(ent, PRINT_HIGH, " | HP: %d", mons_ent->health);
		gi.cprintf(ent, PRINT_HIGH, "\n");
	}
	else if (Q_stricmp(name, "infantry") == 0) 
	{
		SP_monster_infantry_2(mons_ent);
		gi.cprintf(ent, PRINT_HIGH, "Spawned Infantry with element ");
		gi.cprintf(ent, PRINT_HIGH, mons_ent->element);
		gi.cprintf(ent, PRINT_HIGH, " | HP: %d", mons_ent->health);
		gi.cprintf(ent, PRINT_HIGH, "\n");
	}
	else if (Q_stricmp(name, "flyer") == 0) 
	{
		SP_monster_flyer_2(mons_ent);
		gi.cprintf(ent, PRINT_HIGH, "Spawned Flyer with element ");
		gi.cprintf(ent, PRINT_HIGH, mons_ent->element);
		gi.cprintf(ent, PRINT_HIGH, " | HP: %d", mons_ent->health);
		gi.cprintf(ent, PRINT_HIGH, "\n");
	}
	else 
	{
		noMonster = true;
		gi.cprintf(ent, PRINT_HIGH, "Monster not in bank\n");
	}
	
	gi.linkentity(mons_ent);
	if (noMonster) 
	{
		G_FreeEdict(mons_ent);
	}
	
}


void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		sprintf(st, "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent); //MOD Common: show help menu
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent); //OG: Cmd_Inven_f
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp (cmd, "fire") == 0)	//MOD2: Spell Fire command
		Cmd_Spell_Fire_f (ent);
	else if (Q_stricmp(cmd, "void") == 0)	//MOD2: Spell Void command
		Cmd_Spell_Void_f (ent);
	else if (Q_stricmp(cmd, "aero") == 0)	//MOD2: Spell Aero command
		Cmd_Spell_Aero_f(ent);
	else if (Q_stricmp(cmd, "blaze") == 0)	//MOD2: Spell Blaze command
		Cmd_Spell_Dark_Blaze_f(ent);
	else if (Q_stricmp(cmd, "tp") == 0)	//MOD2: Spell Blaze command
		Cmd_Spell_TP_f(ent);
	else if (Q_stricmp(cmd, "spawn") == 0)	//MOD4: demonstration purposes
		Cmd_myMonster_f(ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
