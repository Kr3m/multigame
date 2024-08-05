

#include "g_local.h"

void G_GiveItem(gentity_t *ent, powerup_t pw);

void UsePowerup( gentity_t *ent, powerup_t powerup ) {
	gitem_t *item;
	vec3_t	origin, angles;
	int		i, j;
	gentity_t *drop;

  int itemClass = floor(powerup / PW_MAX_POWERUPS);
  ent->client->inventory[powerup] = 0;
  ent->client->inventoryModified[itemClass] = qtrue;
  ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;

  switch(powerup) {

#if defined(USE_ADVANCED_CLASS) || defined(USE_RUNES)
  case PW_SPECIAL_ABILITY:
#ifdef USE_RUNES
    if(ent->client->inventory[RUNE_ELECTRIC]) {
      gentity_t	*other;
	    int			entityList[MAX_GENTITIES];
    	int			numListedEntities, e;
      vec3_t		mins, maxs, dir;
      vec3_t bounding = {50, 50, 50};
      ent->client->ps.ammo[WP_LIGHTNING] -= 5;
      VectorCopy(ent->s.origin, mins);
      VectorCopy(ent->s.origin, maxs);
      VectorSubtract(mins, bounding, mins);
      VectorAdd(maxs, bounding, maxs);
      numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
      for ( e = 0 ; e < numListedEntities ; e++ ) {
        other = &g_entities[entityList[ e ]];

        if (!ent->takedamage) {
          continue;
        }

        VectorSubtract (other->r.currentOrigin, ent->s.origin, dir);
        dir[2] += 24;
        G_Damage( other, NULL, ent, dir, origin, 25, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_LV_DISCHARGE );
      }
      // TODO: add event that makes a radius of random lighting bolts like the hammer weapon
    } else
    if(ent->client->inventory[RUNE_HEALTH]) {
      UsePowerup(ent, HI_MEDKIT);
      ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
      ent->client->ps.stats[STAT_ABILITY] = 0;
    } else
    if(ent->client->inventory[RUNE_SHIELD]) {
      UsePowerup(ent, HI_INVULNERABILITY);
      ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
      ent->client->ps.stats[STAT_ABILITY] = 0;
    } else
    if(ent->client->inventory[RUNE_RECALL]) {
      UsePowerup(ent, HI_PORTAL);
      ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
      ent->client->ps.stats[STAT_ABILITY] = 0;
    }  else
    if(ent->client->inventory[RUNE_DIVINE]) {
      UsePowerup(ent, HI_KAMIKAZE);
      ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
      ent->client->ps.stats[STAT_ABILITY] = 0;
    } else
#endif
#if defined(USE_ADVANCED_CLASS)
    switch(ent->client->pers.playerclass) {
    case PCLASS_VISOR:
      G_GiveItem(ent, PW_VISIBILITY);
      ent->client->ps.stats[STAT_ABILITY] = 0;
      break;
    case PCLASS_RANGER:
      UsePowerup(ent, HI_TELEPORTER);
      ent->client->ps.stats[STAT_HOLDABLE_ITEM] = 0;
      ent->client->ps.stats[STAT_ABILITY] = 0;
      break;
    default:
      break;
    }
#endif
    break;
#endif

  case HI_TELEPORTER:		// teleporter
    // drop flags in CTF
    item = NULL;
    j = 0;

    if ( ent->client->ps.powerups[ PW_REDFLAG ] ) {
      item = BG_FindItemForPowerup( PW_REDFLAG );
      j = PW_REDFLAG;
    } else if ( ent->client->ps.powerups[ PW_BLUEFLAG ] ) {
      item = BG_FindItemForPowerup( PW_BLUEFLAG );
      j = PW_BLUEFLAG;
#if defined(USE_ADVANCED_GAMES) || defined(USE_ADVANCED_TEAMS)
    } else if ( ent->client->ps.powerups[ PW_GOLDFLAG ] ) {
      item = BG_FindItemForPowerup( PW_GOLDFLAG );
      j = PW_GOLDFLAG;
    } else if ( ent->client->ps.powerups[ PW_GREENFLAG ] ) {
      item = BG_FindItemForPowerup( PW_GREENFLAG );
      j = PW_GREENFLAG;
#endif
    } else if ( ent->client->ps.powerups[ PW_NEUTRALFLAG ] ) {
      item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
      j = PW_NEUTRALFLAG;
    }

    if ( item ) {
      drop = Drop_Item( ent, item, 0 );
      // decide how many seconds it has left
      drop->count = ( ent->client->ps.powerups[ j ] - level.time ) / 1000;
      if ( drop->count < 1 ) {
        drop->count = 1;
      }
      // for pickup prediction
      drop->s.time2 = drop->count;

      ent->client->ps.powerups[ j ] = 0;
    }

#ifdef MISSIONPACK
    if ( g_gametype.integer == GT_HARVESTER ) {
      if ( ent->client->ps.generic1 > 0 ) {
        if ( ent->client->sess.sessionTeam == TEAM_RED ) {
          item = BG_FindItem( "Blue Cube" );
        } else {
          item = BG_FindItem( "Red Cube" );
        }
        if ( item ) {
          for ( j = 0; j < ent->client->ps.generic1; j++ ) {
            drop = Drop_Item( ent, item, 0 );
            if ( ent->client->sess.sessionTeam == TEAM_RED ) {
              drop->spawnflags = TEAM_BLUE;
            } else {
              drop->spawnflags = TEAM_RED;
            }
          }
        }
        ent->client->ps.generic1 = 0;
      }
    }
#endif
    SelectSpawnPoint( ent, ent->client->ps.origin, origin, angles );
    TeleportPlayer( ent, origin, angles );
    break;

  case HI_MEDKIT:		// medkit
#ifdef USE_RUNES
    if(ent->client->inventory[RUNE_HEALTH]) {
      ent->client->ps.stats[STAT_MAX_HEALTH] = rune_healthMax.integer;
      ent->health = rune_health.integer;
    } else
#endif
    ent->health = ent->client->ps.stats[STAT_MAX_HEALTH] + 25;

    break;

  case HI_KAMIKAZE:		// kamikaze
    // make sure the invulnerability is off
    ent->client->invulnerabilityTime = 0;
    // start the kamikze
    G_StartKamikaze( ent );
    break;

  case HI_INVULNERABILITY:		// invulnerability
#ifdef USE_RUNES
    if(ent->client->inventory[RUNE_SHIELD]) {
    ent->client->invulnerabilityTime = level.time + 4000;
    } else
#endif
    ent->client->invulnerabilityTime = level.time + 10000;
    break;

#ifdef USE_PORTALS
  case HI_PORTAL:		// portal
    if( ent->client->portalID ) {
      DropPortalSource( ent, qfalse );
    }
    else {
      DropPortalDestination( ent, qfalse );
    }
    break;
#endif
  default:
    break;
  }
}

