/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/mapitems.h>

#include "character.h"
#include "projectile.h"
#include "laser.h"

#include <stdio.h>
#include <string.h>

// Character, "physical" player's part

void CCharacter::SetWeapon(int W)
{
	if(W == m_Core.m_ActiveWeapon)
		return;

	m_LastWeapon = m_Core.m_ActiveWeapon;
	m_QueuedWeapon = -1;
	m_Core.m_ActiveWeapon = W;

	if(m_Core.m_ActiveWeapon < 0 || m_Core.m_ActiveWeapon >= NUM_WEAPONS)
		m_Core.m_ActiveWeapon = 0;
}

void CCharacter::SetSolo(bool Solo)
{
	TeamsCore()->SetSolo(GetCID(), Solo);
}

bool CCharacter::IsGrounded()
{
	if(Collision()->CheckPoint(m_Pos.x+m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;
	if(Collision()->CheckPoint(m_Pos.x-m_ProximityRadius/2, m_Pos.y+m_ProximityRadius/2+5))
		return true;

	int index = Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y+m_ProximityRadius/2+4));
	int tile = Collision()->GetTileIndex(index);
	int flags = Collision()->GetTileFlags(index);
	if(tile == TILE_STOPA || (tile == TILE_STOP && flags == ROTATION_0) || (tile ==TILE_STOPS && (flags == ROTATION_0 || flags == ROTATION_180)))
		return true;
	tile = Collision()->GetFTileIndex(index);
	flags = Collision()->GetFTileFlags(index);
	if(tile == TILE_STOPA || (tile == TILE_STOP && flags == ROTATION_0) || (tile ==TILE_STOPS && (flags == ROTATION_0 || flags == ROTATION_180)))
		return true;

	return false;
}

void CCharacter::HandleJetpack()
{
	if(m_NumInputs < 2)
		return;

	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = false;
	if(m_Core.m_ActiveWeapon == WEAPON_GRENADE || m_Core.m_ActiveWeapon == WEAPON_SHOTGUN || m_Core.m_ActiveWeapon == WEAPON_RIFLE)
		FullAuto = true;
	if (m_Jetpack && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;

	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if(!WillFire)
		return;

	// check for ammo
	if(!m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
	{
		return;
	}

	switch(m_Core.m_ActiveWeapon)
	{
		case WEAPON_GUN:
		{
			if (m_Jetpack)
			{
				float Strength = m_LastJetpackStrength;
				TakeDamage(Direction * -1.0f * (Strength / 100.0f / 6.11f), g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage, GetCID(), m_Core.m_ActiveWeapon);
			}
		}
	}
}

void CCharacter::RemoveNinja()
{
	m_Ninja.m_CurrentMoveTime = 0;
	m_aWeapons[WEAPON_NINJA].m_Got = false;
	m_Core.m_ActiveWeapon = m_LastWeapon;

	SetWeapon(m_Core.m_ActiveWeapon);
}

void CCharacter::HandleNinja()
{
	if(m_Core.m_ActiveWeapon != WEAPON_NINJA)
		return;

	if ((GameWorld()->GameTick() - m_Ninja.m_ActivationTick) > (g_pData->m_Weapons.m_Ninja.m_Duration * GameWorld()->GameTickSpeed() / 1000))
	{
		// time's up, return
		RemoveNinja();
		return;
	}

	// force ninja Weapon
	SetWeapon(WEAPON_NINJA);

	m_Ninja.m_CurrentMoveTime--;

	if (m_Ninja.m_CurrentMoveTime == 0)
	{
		// reset velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir*m_Ninja.m_OldVelAmount;
	}

	if (m_Ninja.m_CurrentMoveTime > 0)
	{
		// Set velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir * g_pData->m_Weapons.m_Ninja.m_Velocity;
		vec2 OldPos = m_Pos;
		Collision()->MoveBox(&m_Core.m_Pos, &m_Core.m_Vel, vec2(m_ProximityRadius, m_ProximityRadius), 0.f);

		// reset velocity so the client doesn't predict stuff
		m_Core.m_Vel = vec2(0.f, 0.f);

		// check if we Hit anything along the way
		{
			CCharacter *aEnts[MAX_CLIENTS];
			vec2 Dir = m_Pos - OldPos;
			float Radius = m_ProximityRadius * 2.0f;
			vec2 Center = OldPos + Dir * 0.5f;
			int Num = GameWorld()->FindEntities(Center, Radius, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			// check that we're not in solo part
			if (TeamsCore()->GetSolo(GetCID()))
				return;

			for (int i = 0; i < Num; ++i)
			{
				if (aEnts[i] == this)
					continue;

				// Don't hit players in other teams
				if (Team() != aEnts[i]->Team())
					continue;

				// Don't hit players in solo parts
				if (TeamsCore()->GetSolo(aEnts[i]->GetCID()))
					return;

				// make sure we haven't Hit this object before
				bool bAlreadyHit = false;
				for (int j = 0; j < m_NumObjectsHit; j++)
				{
					if (m_aHitObjects[j] == aEnts[i]->GetCID())
						bAlreadyHit = true;
				}
				if (bAlreadyHit)
					continue;

				// check so we are sufficiently close
				if (distance(aEnts[i]->m_Pos, m_Pos) > (m_ProximityRadius * 2.0f))
					continue;

				// Hit a player, give him damage and stuffs...
				// set his velocity to fast upward (for now)
				if(m_NumObjectsHit < 10)
					m_aHitObjects[m_NumObjectsHit++] = aEnts[i]->GetCID();

				CCharacter *pChar = GameWorld()->GetCharacterByID(aEnts[i]->GetCID());
				if(pChar)
					pChar->TakeDamage(vec2(0, -10.0f), g_pData->m_Weapons.m_Ninja.m_pBase->m_Damage, GetCID(), WEAPON_NINJA);
			}
		}

		return;
	}

	return;
}

void CCharacter::DoWeaponSwitch()
{
	// make sure we can switch
	if(m_ReloadTimer != 0 || m_QueuedWeapon == -1 || m_aWeapons[WEAPON_NINJA].m_Got)
		return;

	// switch Weapon
	SetWeapon(m_QueuedWeapon);
}

void CCharacter::HandleWeaponSwitch()
{
	if(m_NumInputs < 2)
		return;

	int WantedWeapon = m_Core.m_ActiveWeapon;
	if(m_QueuedWeapon != -1)
		WantedWeapon = m_QueuedWeapon;

	bool Anything = false;
	 for(int i = 0; i < NUM_WEAPONS - 1; ++i)
		 if(m_aWeapons[i].m_Got)
			 Anything = true;
	 if(!Anything)
		 return;
	// select Weapon
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	if(Next < 128) // make sure we only try sane stuff
	{
		while(Next) // Next Weapon selection
		{
			WantedWeapon = (WantedWeapon+1)%NUM_WEAPONS;
			if(m_aWeapons[WantedWeapon].m_Got)
				Next--;
		}
	}

	if(Prev < 128) // make sure we only try sane stuff
	{
		while(Prev) // Prev Weapon selection
		{
			WantedWeapon = (WantedWeapon-1)<0?NUM_WEAPONS-1:WantedWeapon-1;
			if(m_aWeapons[WantedWeapon].m_Got)
				Prev--;
		}
	}

	// Direct Weapon selection
	if(m_LatestInput.m_WantedWeapon)
		WantedWeapon = m_Input.m_WantedWeapon-1;

	// check for insane values
	if(WantedWeapon >= 0 && WantedWeapon < NUM_WEAPONS && WantedWeapon != m_Core.m_ActiveWeapon && m_aWeapons[WantedWeapon].m_Got)
		m_QueuedWeapon = WantedWeapon;

	DoWeaponSwitch();
}

void CCharacter::FireWeapon()
{
	if(m_NumInputs < 2)
		return;

	if(!GameWorld()->m_WorldConfig.m_PredictWeapons)
		return;

	if(m_ReloadTimer != 0)
		return;

	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = false;
	if(m_Core.m_ActiveWeapon == WEAPON_GRENADE || m_Core.m_ActiveWeapon == WEAPON_SHOTGUN || m_Core.m_ActiveWeapon == WEAPON_RIFLE)
		FullAuto = true;
	if (m_Jetpack && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;

	// don't fire non auto weapons when player is deep and sv_deepfly is disabled
	if(!g_Config.m_SvDeepfly && !FullAuto && m_DeepFreeze)
		return;

	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if(FullAuto && (m_LatestInput.m_Fire&1) && m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if(!WillFire)
		return;

	// check for ammo
	if(!m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
	{
		return;
	}

	vec2 ProjStartPos = m_Pos+Direction*m_ProximityRadius*0.75f;

	switch(m_Core.m_ActiveWeapon)
	{
		case WEAPON_HAMMER:
		{
			// reset objects Hit
			m_NumObjectsHit = 0;

			if (m_Hit&DISABLE_HIT_HAMMER) break;

			CCharacter *apEnts[MAX_CLIENTS];
			int Hits = 0;
			int Num = GameWorld()->FindEntities(ProjStartPos, m_ProximityRadius*0.5f, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; ++i)
			{
				CCharacter *pTarget = apEnts[i];

				if((pTarget == this || (pTarget->IsAlive() && !CanCollide(pTarget->GetCID()))))
					continue;

				// set his velocity to fast upward (for now)

				vec2 Dir;
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);
				else
					Dir = vec2(0.f, -1.f);

				float Strength = Tuning()->m_HammerStrength;

				vec2 Temp = pTarget->m_Core.m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
				pTarget->Core()->LimitForce(&Temp);
				Temp -= pTarget->m_Core.m_Vel;

				vec2 Force = vec2(0.f, -1.0f) + Temp;

				if(GameWorld()->m_WorldConfig.m_IsFNG)
				{
					if(m_GameTeam == pTarget->m_GameTeam && pTarget->m_LastSnapWeapon == WEAPON_NINJA) // melt hammer
					{
						Force.x *= 50 * 0.01f;
						Force.y *= 50 * 0.01f;
					}
					else
					{
						Force.x *= 320 * 0.01f;
						Force.y *= 120 * 0.01f;
					}
				}
				else
					Force *= Strength;

				pTarget->TakeDamage(Force, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
					GetCID(), m_Core.m_ActiveWeapon);
				pTarget->UnFreeze();

				Hits++;
			}

			// if we Hit anything, we have to wait for the reload
			if(Hits)
				m_ReloadTimer = GameWorld()->GameTickSpeed()/3;

		} break;

		case WEAPON_GUN:
		{
			if (!m_Jetpack)
			{
				int Lifetime;
				Lifetime = (int)(GameWorld()->GameTickSpeed()*Tuning()->m_GunLifetime);
				new CProjectile
						(
						GameWorld(),
						WEAPON_GUN,//Type
						GetCID(),//Owner
						ProjStartPos,//Pos
						Direction,//Dir
						Lifetime,//Span
						0,//Freeze
						0,//Explosive
						0,//Force
						-1,//SoundImpact
						WEAPON_GUN//Weapon
						);
			}
		} break;

		case WEAPON_SHOTGUN:
		{
			if(GameWorld()->m_WorldConfig.m_IsVanilla)
			{

				int ShotSpread = 2;
				for(int i = -ShotSpread; i <= ShotSpread; ++i)
				{
					float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
					float a = GetAngle(Direction);
					a += Spreading[i+2];
					float v = 1-(absolute(i)/(float)ShotSpread);
					float Speed = mix((float)GameWorld()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
					new CProjectile
						(
						GameWorld(),
						WEAPON_SHOTGUN,//Type
						GetCID(),//Owner
						ProjStartPos,//Pos
						vec2(cosf(a), sinf(a))*Speed,//Dir
						(int)(GameWorld()->GameTickSpeed()*GameWorld()->Tuning()->m_ShotgunLifetime),//Span
						0,//Freeze
						0,//Explosive
						0,//Force
						-1,//SoundImpact
						WEAPON_SHOTGUN//Weapon
						);
				}
			}
			else if(GameWorld()->m_WorldConfig.m_IsDDRace)
			{
				float LaserReach = Tuning()->m_LaserReach;
				new CLaser(GameWorld(), m_Pos, Direction, LaserReach, GetCID(), WEAPON_SHOTGUN);
			}
		} break;

		case WEAPON_GRENADE:
		{
			int Lifetime = (int)(GameWorld()->GameTickSpeed()*Tuning()->m_GrenadeLifetime);
			new CProjectile
					(
					GameWorld(),
					WEAPON_GRENADE,//Type
					GetCID(),//Owner
					ProjStartPos,//Pos
					Direction,//Dir
					Lifetime,//Span
					0,//Freeze
					true,//Explosive
					0,//Force
					SOUND_GRENADE_EXPLODE,//SoundImpact
					WEAPON_GRENADE//Weapon
					);//SoundImpact
		} break;

		case WEAPON_RIFLE:
		{
			float LaserReach = Tuning()->m_LaserReach;
			new CLaser(GameWorld(), m_Pos, Direction, LaserReach, GetCID(), WEAPON_RIFLE);
		} break;

		case WEAPON_NINJA:
		{
			// reset Hit objects
			m_NumObjectsHit = 0;

			m_Ninja.m_ActivationDir = Direction;
			m_Ninja.m_CurrentMoveTime = g_pData->m_Weapons.m_Ninja.m_Movetime * GameWorld()->GameTickSpeed() / 1000;
			m_Ninja.m_OldVelAmount = length(m_Core.m_Vel);
		} break;

	}

	m_AttackTick = GameWorld()->GameTick();

	if(!m_ReloadTimer)
	{
		float FireDelay;
		Tuning()->Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
		m_ReloadTimer = FireDelay * GameWorld()->GameTickSpeed() / 1000;
	}
}

void CCharacter::HandleWeapons()
{
	//ninja
	HandleNinja();
	HandleJetpack();

	// check reload timer
	if(m_ReloadTimer)
	{
		m_ReloadTimer--;
		return;
	}

	// fire Weapon, if wanted
	FireWeapon();

	return;
}

bool CCharacter::GiveWeapon(int Weapon, int Ammo)
{
	if(m_aWeapons[Weapon].m_Ammo < g_pData->m_Weapons.m_aId[Weapon].m_Maxammo || !m_aWeapons[Weapon].m_Got)
	{
		m_aWeapons[Weapon].m_Got = true;
		if(!m_FreezeTime)
			m_aWeapons[Weapon].m_Ammo = minimum(g_pData->m_Weapons.m_aId[Weapon].m_Maxammo, Ammo);
		return true;
	}
	return false;
}

void CCharacter::GiveNinja()
{
	m_Ninja.m_ActivationTick = GameWorld()->GameTick();
	m_aWeapons[WEAPON_NINJA].m_Got = true;
	if (!m_FreezeTime)
		m_aWeapons[WEAPON_NINJA].m_Ammo = -1;
	if (m_Core.m_ActiveWeapon != WEAPON_NINJA)
		m_LastWeapon = m_Core.m_ActiveWeapon;
	m_Core.m_ActiveWeapon = WEAPON_NINJA;
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
	// copy new input
	mem_copy(&m_SavedInput, pNewInput, sizeof(m_SavedInput));
	mem_copy(&m_Input, pNewInput, sizeof(m_Input));
	//m_NumInputs++;

	// it is not allowed to aim in the center
	if(m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
	m_NumInputs++;
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
	mem_copy(&m_LatestInput, pNewInput, sizeof(m_LatestInput));

	// it is not allowed to aim in the center
	if(m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;

	if(m_NumInputs > 2 && Team() != TEAM_SPECTATORS)
	{
		HandleWeaponSwitch();
		FireWeapon();
	}

	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
}

void CCharacter::Tick()
{
	DDRaceTick();

	m_Core.m_Input = m_Input;
	m_Core.Tick(true);

	// handle Weapons
	HandleWeapons();

	DDRacePostCoreTick();

	// Previnput
	m_PrevInput = m_Input;

	m_PrevPos = m_Core.m_Pos;
	return;
}

void CCharacter::TickDefered()
{
	m_Core.Move();
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;
}

bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	m_Core.ApplyForce(Force);
	return true;
}

// DDRace

bool CCharacter::CanCollide(int ClientID)
{
	return TeamsCore()->CanCollide(GetCID(), ClientID);
}

bool CCharacter::SameTeam(int ClientID)
{
	return TeamsCore()->SameTeam(GetCID(), ClientID);
}

int CCharacter::Team()
{
	return TeamsCore()->Team(GetCID());
}

void CCharacter::HandleSkippableTiles(int Index)
{
	if(Index < 0)
		return;

	// handle speedup tiles
	if(Collision()->IsSpeedup(Index))
	{
		vec2 Direction, MaxVel, TempVel = m_Core.m_Vel;
		int Force, MaxSpeed = 0;
		float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
		Collision()->GetSpeedup(Index, &Direction, &Force, &MaxSpeed);
		if(Force == 255 && MaxSpeed)
		{
			m_Core.m_Vel = Direction * (MaxSpeed/5);
		}
		else
		{
			if(MaxSpeed > 0 && MaxSpeed < 5) MaxSpeed = 5;
			if(MaxSpeed > 0)
			{
				if(Direction.x > 0.0000001f)
					SpeederAngle = -atan(Direction.y / Direction.x);
				else if(Direction.x < 0.0000001f)
					SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
				else if(Direction.y > 0.0000001f)
					SpeederAngle = asin(1.0f);
				else
					SpeederAngle = asin(-1.0f);

				if(SpeederAngle < 0)
					SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

				if(TempVel.x > 0.0000001f)
					TeeAngle = -atan(TempVel.y / TempVel.x);
				else if(TempVel.x < 0.0000001f)
					TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
				else if(TempVel.y > 0.0000001f)
					TeeAngle = asin(1.0f);
				else
					TeeAngle = asin(-1.0f);

				if(TeeAngle < 0)
					TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

				TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

				DiffAngle = SpeederAngle - TeeAngle;
				SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;
				if(abs((int)SpeedLeft) > Force && SpeedLeft > 0.0000001f)
					TempVel += Direction * Force;
				else if(abs((int)SpeedLeft) > Force)
					TempVel += Direction * -Force;
				else
					TempVel += Direction * SpeedLeft;
			}
			else
				TempVel += Direction * Force;
			m_Core.LimitForce(&TempVel);
			m_Core.m_Vel = TempVel;
		}
	}
}

void CCharacter::HandleTiles(int Index)
{
	int MapIndex = Index;
	float Offset = 4.0f;
	int MapIndexL = Collision()->GetPureMapIndex(vec2(m_Pos.x + (m_ProximityRadius / 2) + Offset, m_Pos.y));
	int MapIndexR = Collision()->GetPureMapIndex(vec2(m_Pos.x - (m_ProximityRadius / 2) - Offset, m_Pos.y));
	int MapIndexT = Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y + (m_ProximityRadius / 2) + Offset));
	int MapIndexB = Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y - (m_ProximityRadius / 2) - Offset));
	m_TileIndex = Collision()->GetTileIndex(MapIndex);
	m_TileFlags = Collision()->GetTileFlags(MapIndex);
	m_TileIndexL = Collision()->GetTileIndex(MapIndexL);
	m_TileFlagsL = Collision()->GetTileFlags(MapIndexL);
	m_TileIndexR = Collision()->GetTileIndex(MapIndexR);
	m_TileFlagsR = Collision()->GetTileFlags(MapIndexR);
	m_TileIndexB = Collision()->GetTileIndex(MapIndexB);
	m_TileFlagsB = Collision()->GetTileFlags(MapIndexB);
	m_TileIndexT = Collision()->GetTileIndex(MapIndexT);
	m_TileFlagsT = Collision()->GetTileFlags(MapIndexT);
	m_TileFIndex = Collision()->GetFTileIndex(MapIndex);
	m_TileFFlags = Collision()->GetFTileFlags(MapIndex);
	m_TileFIndexL = Collision()->GetFTileIndex(MapIndexL);
	m_TileFFlagsL = Collision()->GetFTileFlags(MapIndexL);
	m_TileFIndexR = Collision()->GetFTileIndex(MapIndexR);
	m_TileFFlagsR = Collision()->GetFTileFlags(MapIndexR);
	m_TileFIndexB = Collision()->GetFTileIndex(MapIndexB);
	m_TileFFlagsB = Collision()->GetFTileFlags(MapIndexB);
	m_TileFIndexT = Collision()->GetFTileIndex(MapIndexT);
	m_TileFFlagsT = Collision()->GetFTileFlags(MapIndexT);//
	m_TileSIndex = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndex)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileIndex(MapIndex) : 0 : 0;
	m_TileSFlags = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndex)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileFlags(MapIndex) : 0 : 0;
	m_TileSIndexL = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexL)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileIndex(MapIndexL) : 0 : 0;
	m_TileSFlagsL = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexL)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileFlags(MapIndexL) : 0 : 0;
	m_TileSIndexR = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexR)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileIndex(MapIndexR) : 0 : 0;
	m_TileSFlagsR = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexR)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileFlags(MapIndexR) : 0 : 0;
	m_TileSIndexB = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexB)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileIndex(MapIndexB) : 0 : 0;
	m_TileSFlagsB = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexB)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileFlags(MapIndexB) : 0 : 0;
	m_TileSIndexT = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexT)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileIndex(MapIndexT) : 0 : 0;
	m_TileSFlagsT = (Collision()->m_pSwitchers && Collision()->m_pSwitchers[Collision()->GetDTileNumber(MapIndexT)].m_Status[Team()])?(Team() != TEAM_SUPER)? Collision()->GetDTileFlags(MapIndexT) : 0 : 0;

	// stopper
	if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL ==ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)) && m_Core.m_Vel.x > 0)
	{
		if((int)Collision()->GetPos(MapIndexL).x)
			if((int)Collision()->GetPos(MapIndexL).x < (int)m_Core.m_Pos.x)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.x = 0;
	}
	if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)) && m_Core.m_Vel.x < 0)
	{
		if((int)Collision()->GetPos(MapIndexR).x)
			if((int)Collision()->GetPos(MapIndexR).x > (int)m_Core.m_Pos.x)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.x = 0;
	}
	if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)) && m_Core.m_Vel.y < 0)
	{
		if((int)Collision()->GetPos(MapIndexB).y)
			if((int)Collision()->GetPos(MapIndexB).y > (int)m_Core.m_Pos.y)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.y = 0;
	}
	if(((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)) && m_Core.m_Vel.y > 0)
	{
		if((int)Collision()->GetPos(MapIndexT).y)
			if((int)Collision()->GetPos(MapIndexT).y < (int)m_Core.m_Pos.y)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.y = 0;
		m_Core.m_Jumped = 0;
		m_Core.m_JumpedTotal = 0;
	}

	if(!GameWorld()->m_WorldConfig.m_PredictTiles)
		return;

	if(Index < 0)
	{
		m_LastRefillJumps = false;
		return;
	}

	// freeze
	if(((m_TileIndex == TILE_FREEZE) || (m_TileFIndex == TILE_FREEZE)) && !m_Super && !m_DeepFreeze)
	{
		Freeze();
	}
	else if(((m_TileIndex == TILE_UNFREEZE) || (m_TileFIndex == TILE_UNFREEZE)) && !m_DeepFreeze)
	{
		UnFreeze();
	}

	// deep freeze
	if(((m_TileIndex == TILE_DFREEZE) || (m_TileFIndex == TILE_DFREEZE)) && !m_Super && !m_DeepFreeze)
	{
		m_DeepFreeze = true;
	}
	else if(((m_TileIndex == TILE_DUNFREEZE) || (m_TileFIndex == TILE_DUNFREEZE)) && !m_Super && m_DeepFreeze)
	{
		m_DeepFreeze = false;
	}

	// endless hook
	if(((m_TileIndex == TILE_EHOOK_START) || (m_TileFIndex == TILE_EHOOK_START)) && !m_EndlessHook)
	{
		m_EndlessHook = true;
	}
	else if(((m_TileIndex == TILE_EHOOK_END) || (m_TileFIndex == TILE_EHOOK_END)) && m_EndlessHook)
	{
		m_EndlessHook = false;
	}

	// collide with others
	if(((m_TileIndex == TILE_NPC_END) || (m_TileFIndex == TILE_NPC_END)) && m_Core.m_Collision)
	{
		m_Core.m_Collision = false;
	}
	else if(((m_TileIndex == TILE_NPC_START) || (m_TileFIndex == TILE_NPC_START)) && !m_Core.m_Collision)
	{
		m_Core.m_Collision = true;
	}

	// hook others
	if(((m_TileIndex == TILE_NPH_END) || (m_TileFIndex == TILE_NPH_END)) && m_Core.m_Hook)
	{
		m_Core.m_Hook = false;
	}
	else if(((m_TileIndex == TILE_NPH_START) || (m_TileFIndex == TILE_NPH_START)) && !m_Core.m_Hook)
	{
		m_Core.m_Hook = true;
	}

	// unlimited air jumps
	if(((m_TileIndex == TILE_SUPER_START) || (m_TileFIndex == TILE_SUPER_START)) && !m_SuperJump)
	{
		m_SuperJump = true;
	}
	else if(((m_TileIndex == TILE_SUPER_END) || (m_TileFIndex == TILE_SUPER_END)) && m_SuperJump)
	{
		m_SuperJump = false;
	}

	// walljump
	if((m_TileIndex == TILE_WALLJUMP) || (m_TileFIndex == TILE_WALLJUMP))
	{
		if(m_Core.m_Vel.y > 0 && m_Core.m_Colliding && m_Core.m_LeftWall)
		{
			m_Core.m_LeftWall = false;
			m_Core.m_JumpedTotal = m_Core.m_Jumps - 1;
			m_Core.m_Jumped = 1;
		}
	}

	// jetpack gun
	if(((m_TileIndex == TILE_JETPACK_START) || (m_TileFIndex == TILE_JETPACK_START)) && !m_Jetpack)
	{
		m_Jetpack = true;
	}
	else if(((m_TileIndex == TILE_JETPACK_END) || (m_TileFIndex == TILE_JETPACK_END)) && m_Jetpack)
	{
		m_Jetpack = false;
	}

	// solo part
	if(((m_TileIndex == TILE_SOLO_START) || (m_TileFIndex == TILE_SOLO_START)) && !TeamsCore()->GetSolo(GetCID()))
	{
		SetSolo(true);
	}
	else if(((m_TileIndex == TILE_SOLO_END) || (m_TileFIndex == TILE_SOLO_END)) && TeamsCore()->GetSolo(GetCID()))
	{
		SetSolo(false);
	}

	// refill jumps
	if(((m_TileIndex == TILE_REFILL_JUMPS) || (m_TileFIndex == TILE_REFILL_JUMPS)) && !m_LastRefillJumps)
	{
		m_Core.m_JumpedTotal = 0;
		m_Core.m_Jumped = 0;
		m_LastRefillJumps = true;
	}
	if((m_TileIndex != TILE_REFILL_JUMPS) && (m_TileFIndex != TILE_REFILL_JUMPS))
	{
		m_LastRefillJumps = false;
	}

	// handle switch tiles
	if(Collision()->IsSwitch(MapIndex) == TILE_SWITCHOPEN && Team() != TEAM_SUPER)
	{
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = true;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = 0;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHOPEN;
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_SWITCHTIMEDOPEN && Team() != TEAM_SUPER)
	{
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = true;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = GameWorld()->GameTick() + 1 + Collision()->GetSwitchDelay(MapIndex)*GameWorld()->GameTickSpeed() ;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHTIMEDOPEN;
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_SWITCHTIMEDCLOSE && Team() != TEAM_SUPER)
	{
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = false;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = GameWorld()->GameTick() + 1 + Collision()->GetSwitchDelay(MapIndex)*GameWorld()->GameTickSpeed();
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHTIMEDCLOSE;
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_SWITCHCLOSE && Team() != TEAM_SUPER)
	{
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = false;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = 0;
		Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHCLOSE;
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_FREEZE && Team() != TEAM_SUPER)
	{
		if(Collision()->GetSwitchNumber(MapIndex) == 0 || Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()])
			Freeze(Collision()->GetSwitchDelay(MapIndex));
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_DFREEZE && Team() != TEAM_SUPER && Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()])
	{
		m_DeepFreeze = true;
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_DUNFREEZE && Team() != TEAM_SUPER && Collision()->m_pSwitchers[Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()])
	{
		m_DeepFreeze = false;
	}
	else if(Collision()->IsSwitch(MapIndex) == TILE_JUMP)
	{
		int newJumps = Collision()->GetSwitchDelay(MapIndex);

		if (newJumps != m_Core.m_Jumps)
		{
			m_Core.m_Jumps = newJumps;
		}
	}
}

void CCharacter::DDRaceTick()
{
	mem_copy(&m_Input, &m_SavedInput, sizeof(m_Input));
	if(m_FreezeTime > 0 || m_FreezeTime == -1)
	{
		if(m_FreezeTime > 0)
			m_FreezeTime--;
		else
			m_Ninja.m_ActivationTick = GameWorld()->GameTick();
		if(!m_CanMoveInFreeze)
		{
			m_Input.m_Direction = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Hook = 0;
		}
		if (m_FreezeTime == 1)
			UnFreeze();
	}
}

void CCharacter::DDRacePostCoreTick()
{
	if(!GameWorld()->m_WorldConfig.m_PredictDDRace)
		return;

	if (m_EndlessHook)
		m_Core.m_HookTick = 0;

	if (m_DeepFreeze && !m_Super)
		Freeze();

	if (m_Core.m_Jumps == 0 && !m_Super)
		m_Core.m_Jumped = 3;
	else if (m_Core.m_Jumps == 1 && m_Core.m_Jumped > 0)
		m_Core.m_Jumped = 3;
	else if (m_Core.m_JumpedTotal < m_Core.m_Jumps - 1 && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;

	if ((m_Super || m_SuperJump) && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;

	int CurrentIndex = Collision()->GetMapIndex(m_Pos);
	HandleSkippableTiles(CurrentIndex);

	// handle Anti-Skip tiles
	std::list < int > Indices = Collision()->GetMapIndices(m_PrevPos, m_Pos);
	if(!Indices.empty())
		for(std::list < int >::iterator i = Indices.begin(); i != Indices.end(); i++)
			HandleTiles(*i);
	else
	{
		HandleTiles(CurrentIndex);
	}
}

bool CCharacter::Freeze(int Seconds)
{
	if(!GameWorld()->m_WorldConfig.m_PredictFreeze)
		return false;
	if ((Seconds <= 0 || m_Super || m_FreezeTime == -1 || m_FreezeTime > Seconds * GameWorld()->GameTickSpeed()) && Seconds != -1)
		 return false;
	if (m_FreezeTick < GameWorld()->GameTick() - GameWorld()->GameTickSpeed() || Seconds == -1)
	{
		for(int i = 0; i < NUM_WEAPONS; i++)
			if(m_aWeapons[i].m_Got)
			 {
				 m_aWeapons[i].m_Ammo = 0;
			 }
		m_FreezeTime = Seconds == -1 ? Seconds : Seconds * GameWorld()->GameTickSpeed();
		m_FreezeTick = GameWorld()->GameTick();
		return true;
	}
	return false;
}

bool CCharacter::Freeze()
{
	return Freeze(g_Config.m_SvFreezeDelay);
}

bool CCharacter::UnFreeze()
{
	if (m_FreezeTime > 0)
	{
		for(int i=0;i<NUM_WEAPONS;i++)
			m_aWeapons[i].m_Ammo = -1;
		if(!m_aWeapons[m_Core.m_ActiveWeapon].m_Got)
			m_Core.m_ActiveWeapon = WEAPON_GUN;
		m_FreezeTime = 0;
		m_FreezeTick = 0;
		if (m_Core.m_ActiveWeapon==WEAPON_HAMMER) m_ReloadTimer = 0;
		return true;
	}
	return false;
}

void CCharacter::GiveAllWeapons()
{
	for(int i=WEAPON_GUN;i<NUM_WEAPONS-1;i++)
	{
		m_aWeapons[i].m_Got = true;
		if(!m_FreezeTime) m_aWeapons[i].m_Ammo = -1;
	}
	return;
}

CTeamsCore* CCharacter::TeamsCore()
{
	return m_Core.m_pTeams;
}

CCharacter::CCharacter(CGameWorld *pGameWorld, int ID, CNetObj_Character *pChar, CNetObj_DDNetCharacter *pExtended)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_CHARACTER)
{
	m_ID = ID;

	m_LastWeapon = WEAPON_HAMMER;
	m_QueuedWeapon = -1;
	m_LastRefillJumps = false;
	m_Pos = vec2(pChar->m_X, pChar->m_Y);
	m_PrevPos = m_Pos;
	m_Core.Reset();
	m_Core.Init(&GameWorld()->m_Core, GameWorld()->Collision(), GameWorld()->Teams());
	m_Core.m_Id = ID;
	mem_zero(&m_Ninja, sizeof(m_Ninja));
	mem_zero(&m_SavedInput, sizeof(m_SavedInput));
	m_LatestInput = m_LatestPrevInput = m_PrevInput = m_Input = m_SavedInput;
	m_ProximityRadius = ms_PhysSize;
	m_Core.m_LeftWall = 1;
	m_ReloadTimer = 0;
	m_NumObjectsHit = 0;
	m_LastRefillJumps = false;
	m_LastJetpackStrength = 400.0;
	m_Super = false;
	m_CanMoveInFreeze = false;
	m_Alive = true;

	ResetPrediction();
	Read(pChar, pExtended, false);

	GameWorld()->InsertEntity(this);
}

void CCharacter::ResetPrediction()
{
	SetSolo(false);
	m_EndlessHook = false;
	m_Hit = HIT_ALL;
	m_SuperJump = false;
	m_Jetpack = false;
	m_Core.m_Jumps = 2;
	m_NumInputs = 0;
	m_FreezeTime = 0;
	m_FreezeTick = 0;
	m_DeepFreeze = 0;
	m_Super = false;
	for(int w = 0; w < NUM_WEAPONS; w++)
		SetWeaponGot(w, false);
}

void CCharacter::Read(CNetObj_Character *pChar, CNetObj_DDNetCharacter *pExtended, bool IsLocal)
{
	vec2 PosBefore = m_Pos;

	m_Core.Read((CNetObj_CharacterCore*) pChar);
	m_Pos = m_Core.m_Pos;
	m_AttackTick = pChar->m_AttackTick;

	if(distance(PosBefore, m_Pos) > 2.f) // misprediction, don't use prevpos
		m_PrevPos = m_Pos;

	if(distance(m_PrevPos, m_Pos) > 10.f * 32.f) // reset prevpos if the distance is high
		m_PrevPos = m_Pos;

	// remove weapons that are unavailable. if the current weapon is ninja just set ammo to zero in case the player is frozen
	if(pChar->m_Weapon != m_Core.m_ActiveWeapon)
	{
		if(pChar->m_Weapon == WEAPON_NINJA)
			m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo = 0;
		else
		{
			if(m_Core.m_ActiveWeapon == WEAPON_NINJA)
			{
				SetNinjaActivationDir(vec2(0,0));
				SetNinjaActivationTick(-500);
				SetNinjaCurrentMoveTime(0);
			}
			if(pChar->m_Weapon == m_LastSnapWeapon)
				m_aWeapons[m_Core.m_ActiveWeapon].m_Got = false;
		}
	}
	m_LastSnapWeapon = pChar->m_Weapon;

	// add weapons
	if(pChar->m_Weapon != WEAPON_NINJA)
	{
		m_aWeapons[pChar->m_Weapon].m_Got = true;
		m_aWeapons[pChar->m_Weapon].m_Ammo = (GameWorld()->m_WorldConfig.m_InfiniteAmmo || GameWorld()->m_WorldConfig.m_IsDDRace || pChar->m_Weapon == WEAPON_HAMMER) ? -1 : pChar->m_AmmoCount;
		SetActiveWeapon(pChar->m_Weapon);
	}

	if(pChar->m_Emote != EMOTE_PAIN && pChar->m_Emote != EMOTE_NORMAL)
		m_DeepFreeze = false;
	if(pChar->m_Weapon != WEAPON_NINJA || pChar->m_AttackTick > m_FreezeTick || absolute(pChar->m_VelX) == 256*10)
	{
		m_DeepFreeze = false;
		UnFreeze();
	}
	if(!GameWorld()->m_WorldConfig.m_PredictFreeze)
	{
		m_DeepFreeze = false;
		UnFreeze();
	}

	if(GameWorld()->m_WorldConfig.m_PredictWeapons && Tuning()->m_JetpackStrength > 0)
	{
		m_LastJetpackStrength = Tuning()->m_JetpackStrength;
		m_Jetpack = true;
	}
	else if(pChar->m_Weapon != WEAPON_NINJA)
		m_Jetpack = false;

	if(GameWorld()->m_WorldConfig.m_PredictTiles)
	{
		if(pChar->m_Jumped&2)
		{
			m_SuperJump = false;
			if(m_Core.m_Jumps > m_Core.m_JumpedTotal && m_Core.m_JumpedTotal > 0 && m_Core.m_Jumps > 2)
				m_Core.m_Jumps = m_Core.m_JumpedTotal + 1;
			else
				m_Core.m_JumpedTotal = m_Core.m_Jumps;
		}
		else
		{
			if(m_Core.m_Jumps < 2)
				m_Core.m_Jumps = m_Core.m_JumpedTotal + 2;
		}
		if(Tuning()->m_AirJumpImpulse == 0)
		{
			m_Core.m_Jumps = 0;
			m_Core.m_Jumped = 3;
		}
	}

	if(m_Core.m_HookTick != 0)
		m_EndlessHook = false;

	// reset player collision
	if(!pExtended)
		SetSolo(!Tuning()->m_PlayerCollision && !Tuning()->m_PlayerHooking);
	m_Core.m_Collision = Tuning()->m_PlayerCollision;
	m_Core.m_Hook = Tuning()->m_PlayerHooking;

	// reset all input except direction and hook for non-local players (as in vanilla prediction)
	if(!IsLocal)
	{
		mem_zero(&m_Input, sizeof(m_Input));
		mem_zero(&m_SavedInput, sizeof(m_SavedInput));
		m_Input.m_Direction = m_SavedInput.m_Direction = m_Core.m_Direction;
		m_Input.m_Hook = m_SavedInput.m_Hook = (m_Core.m_HookState == HOOK_GRABBED);
		m_Input.m_TargetX = cosf(pChar->m_Angle/256.0f);
		m_Input.m_TargetY = sinf(pChar->m_Angle/256.0f);
	}

	m_Alive = true;

	if(pExtended)
	{
		m_Super = pExtended->m_Flags & CHARACTERFLAG_SUPER;

		SetSolo(pExtended->m_Flags & CHARACTERFLAG_SOLO);
		m_Super = pExtended->m_Flags & CHARACTERFLAG_SUPER;
		if(m_Super)
			TeamsCore()->Team(GetCID(), TeamsCore()->m_IsDDRace16 ? VANILLA_TEAM_SUPER : TEAM_SUPER);
		m_EndlessHook = pExtended->m_Flags & CHARACTERFLAG_ENDLESS_HOOK;
		m_Core.m_Collision = !(pExtended->m_Flags & CHARACTERFLAG_NO_COLLISION);
		m_Core.m_Hook = !(pExtended->m_Flags & CHARACTERFLAG_NO_HOOK);
		m_SuperJump = pExtended->m_Flags & CHARACTERFLAG_ENDLESS_JUMP;
		m_Jetpack = pExtended->m_Flags & CHARACTERFLAG_JETPACK;

		m_Hit = HIT_ALL;
		if(pExtended->m_Flags & CHARACTERFLAG_NO_GRENADE_HIT)
			m_Hit |= DISABLE_HIT_GRENADE;
		if(pExtended->m_Flags & CHARACTERFLAG_NO_HAMMER_HIT)
			m_Hit |= DISABLE_HIT_HAMMER;
		if(pExtended->m_Flags & CHARACTERFLAG_NO_RIFLE_HIT)
			m_Hit |= DISABLE_HIT_RIFLE;
		if(pExtended->m_Flags & CHARACTERFLAG_NO_SHOTGUN_HIT)
			m_Hit |= DISABLE_HIT_SHOTGUN;
	}
}

void CCharacter::SetCoreWorld(CGameWorld *pGameWorld)
{
	m_Core.m_pWorld = &pGameWorld->m_Core;
	m_Core.m_pCollision = pGameWorld->Collision();
	m_Core.m_pTeams = pGameWorld->Teams();
}

bool CCharacter::Match(CCharacter *pChar)
{
	if(distance(pChar->m_Core.m_Pos, m_Core.m_Pos) > 32.f)
		return false;
	return true;
}
