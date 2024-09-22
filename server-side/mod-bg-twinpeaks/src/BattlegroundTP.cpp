/*
 * Copyright (C) ArkCORE
 * Copyright (C) 2019+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: http://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
*/

#include "BattlegroundTP.h"
#include "Creature.h"
#include "GameObject.h"
#include "Language.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "BattlegroundMgr.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "Battleground.h"
#include "GameGraveyard.h"
#include <unordered_map>

#include "ScriptMgr.h"
#include "Config.h"

// adding Battleground to the core battlegrounds list
BattlegroundTypeId BATTLEGROUND_TP = BattlegroundTypeId(108); // value from BattlemasterList.dbc
BattlegroundQueueTypeId BATTLEGROUND_QUEUE_TP = BattlegroundQueueTypeId(12);

// these variables aren't used outside of this file, so declare them only here
enum BG_TP_Rewards
{
    BG_TP_WIN = 0,
    BG_TP_FLAG_CAP,
    BG_TP_MAP_COMPLETE,
    BG_TP_REWARD_NUM
};

uint32 BG_TP_Honor[BG_HONOR_MODE_NUM][BG_TP_REWARD_NUM] = {
    {20, 40, 40}, // normal honor
    {60, 40, 80}  // holiday
};

uint32 BG_TP_Reputation[BG_HONOR_MODE_NUM][BG_TP_REWARD_NUM] = {
    {0, 35, 0},   // normal honor
    {0, 45, 0}    // holiday
};

BattlegroundTP::BattlegroundTP()
{
    BgObjects.resize(BG_TP_OBJECT_MAX);
    BgCreatures.resize(BG_CREATURES_MAX_TP);

    StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_BG_TP_START_TWO_MINUTES;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_TP_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_TP_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_TP_HAS_BEGUN;

    _flagKeepers[TEAM_ALLIANCE] = 0;
    _flagKeepers[TEAM_HORDE] = 0;
    _droppedFlagGUID[TEAM_ALLIANCE] = 0;
    _droppedFlagGUID[TEAM_HORDE] = 0;
    _flagState[TEAM_ALLIANCE] = BG_TP_FLAG_STATE_ON_BASE;
    _flagState[TEAM_HORDE] = BG_TP_FLAG_STATE_ON_BASE;
    _lastFlagCaptureTeam = TEAM_NEUTRAL;
    _reputationCapture = 0;
    _honorWinKills = 0;
    _honorEndKills = 0;
}

BattlegroundTP::~BattlegroundTP() { }

void BattlegroundTP::PostUpdateImpl(uint32 diff)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        _bgEvents.Update(diff);
        switch (_bgEvents.ExecuteEvent())
        {
            case BG_TP_EVENT_UPDATE_GAME_TIME:
                UpdateWorldState(BG_TP_STATE_TIMER, GetMatchTime());
                _bgEvents.ScheduleEvent(BG_TP_EVENT_UPDATE_GAME_TIME, ((BG_TP_TOTAL_GAME_TIME - GetStartTime()) % (MINUTE*IN_MILLISECONDS)) + 1);
                break;
            case BG_TP_EVENT_NO_TIME_LEFT:
                if (GetTeamScore(TEAM_ALLIANCE) == GetTeamScore(TEAM_HORDE))
                    EndBattleground(_lastFlagCaptureTeam);
                else
                    EndBattleground(GetTeamScore(TEAM_HORDE) > GetTeamScore(TEAM_ALLIANCE) ? TEAM_HORDE : TEAM_ALLIANCE);
                break;
            case BG_TP_EVENT_RESPAWN_BOTH_FLAGS:
                SpawnBGObject(BG_TP_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
                SpawnBGObject(BG_TP_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
                SendMessageToAll(LANG_BG_TP_F_PLACED, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                PlaySoundToAll(BG_TP_SOUND_FLAGS_RESPAWNED);
                break;
            case BG_TP_EVENT_ALLIANCE_DROP_FLAG:
                RespawnFlagAfterDrop(TEAM_ALLIANCE);
                break;
            case BG_TP_EVENT_HORDE_DROP_FLAG:
                RespawnFlagAfterDrop(TEAM_HORDE);
                break;
            case BG_TP_EVENT_BOTH_FLAGS_KEPT10:
                if (Player* player = ObjectAccessor::GetObjectInMap(GetFlagPickerGUID(TEAM_ALLIANCE), this->FindBgMap(), (Player*)NULL))
                    player->CastSpell(player, BG_TP_SPELL_FOCUSED_ASSAULT, true);
                if (Player* player = ObjectAccessor::GetObjectInMap(GetFlagPickerGUID(TEAM_HORDE), this->FindBgMap(), (Player*)NULL))
                    player->CastSpell(player, BG_TP_SPELL_FOCUSED_ASSAULT, true);
                break;
            case BG_TP_EVENT_BOTH_FLAGS_KEPT15:
                if (Player* player = ObjectAccessor::GetObjectInMap(GetFlagPickerGUID(TEAM_ALLIANCE), this->FindBgMap(), (Player*)NULL))
                {
                    player->RemoveAurasDueToSpell(BG_TP_SPELL_FOCUSED_ASSAULT);
                    player->CastSpell(player, BG_TP_SPELL_BRUTAL_ASSAULT, true);
                }
                if (Player* player = ObjectAccessor::GetObjectInMap(GetFlagPickerGUID(TEAM_HORDE), this->FindBgMap(), (Player*)NULL))
                {
                    player->RemoveAurasDueToSpell(BG_TP_SPELL_FOCUSED_ASSAULT);
                    player->CastSpell(player, BG_TP_SPELL_BRUTAL_ASSAULT, true);
                }
                break;
        }
    }
}

void BattlegroundTP::StartingEventCloseDoors()
{
    for (uint32 i = BG_TP_OBJECT_DOOR_A_1; i <= BG_TP_OBJECT_DOOR_H_4; ++i)
    {
        DoorClose(i);
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
    }

    for (uint32 i = BG_TP_OBJECT_A_FLAG; i <= BG_TP_OBJECT_BERSERKBUFF_2; ++i)
        SpawnBGObject(i, RESPAWN_ONE_DAY);
}

void BattlegroundTP::StartingEventOpenDoors()
{
    for (uint32 i = BG_TP_OBJECT_DOOR_A_1; i <= BG_TP_OBJECT_DOOR_H_4; ++i)
    {
        DoorOpen(i);
    }

    for (uint32 i = BG_TP_OBJECT_A_FLAG; i <= BG_TP_OBJECT_BERSERKBUFF_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);

    // players joining later are not egible
    //StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, TP_EVENT_START_BATTLE);
    UpdateWorldState(BG_TP_STATE_TIMER_ACTIVE, 1);
    _bgEvents.ScheduleEvent(BG_TP_EVENT_UPDATE_GAME_TIME, 0);
    _bgEvents.ScheduleEvent(BG_TP_EVENT_NO_TIME_LEFT, BG_TP_TOTAL_GAME_TIME - 2*MINUTE*IN_MILLISECONDS); // 27 - 2 = 25 minutes
}

void BattlegroundTP::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    PlayerScores[player->GetGUID()] = new BattlegroundTPScore(player);
}

void BattlegroundTP::RespawnFlagAfterDrop(TeamId teamId)
{
    if (GetStatus() != STATUS_IN_PROGRESS || GetFlagState(teamId) != BG_TP_FLAG_STATE_ON_GROUND)
        return;

    UpdateFlagState(teamId, BG_TP_FLAG_STATE_ON_BASE);
    SpawnBGObject(teamId == TEAM_ALLIANCE ? BG_TP_OBJECT_A_FLAG : BG_TP_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
    SendMessageToAll(teamId == TEAM_ALLIANCE ? LANG_BG_TP_ALLIANCE_FLAG_RESPAWNED : LANG_BG_TP_HORDE_FLAG_RESPAWNED, CHAT_MSG_BG_SYSTEM_NEUTRAL);
    PlaySoundToAll(BG_TP_SOUND_FLAGS_RESPAWNED);

    if (GameObject* flag = GetBgMap()->GetGameObject(GetDroppedFlagGUID(teamId)))
        flag->Delete();

    SetDroppedFlagGUID(0, teamId);
    _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT10);
    _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT15);
    RemoveAssaultAuras();
}

void BattlegroundTP::EventPlayerCapturedFlag(Player* player)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    RemoveAssaultAuras();

    AddPoints(player->GetTeamId(), 1);
    SetFlagPicker(0, GetOtherTeamId(player->GetTeamId()));
    UpdateFlagState(GetOtherTeamId(player->GetTeamId()), BG_TP_FLAG_STATE_ON_BASE);
    if (player->GetTeamId() == TEAM_ALLIANCE)
    {
        player->RemoveAurasDueToSpell(BG_TP_SPELL_HORDE_FLAG);
        PlaySoundToAll(BG_TP_SOUND_FLAG_CAPTURED_ALLIANCE);
        SendMessageToAll(LANG_BG_TP_CAPTURED_HF, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
    }
    else
    {
        player->RemoveAurasDueToSpell(BG_TP_SPELL_ALLIANCE_FLAG);
        PlaySoundToAll(BG_TP_SOUND_FLAG_CAPTURED_HORDE);
        SendMessageToAll(LANG_BG_TP_CAPTURED_AF, CHAT_MSG_BG_SYSTEM_HORDE, player);
    }

    SpawnBGObject(BG_TP_OBJECT_H_FLAG, BG_TP_FLAG_RESPAWN_TIME);
    SpawnBGObject(BG_TP_OBJECT_A_FLAG, BG_TP_FLAG_RESPAWN_TIME);

    UpdateWorldState(player->GetTeamId() == TEAM_ALLIANCE ? BG_TP_FLAG_CAPTURES_ALLIANCE : BG_TP_FLAG_CAPTURES_HORDE, GetTeamScore(player->GetTeamId()));
    UpdatePlayerScore(player, SCORE_FLAG_CAPTURES, 1);      // +1 flag captures
    _lastFlagCaptureTeam = player->GetTeamId();

    RewardHonorToTeam(GetBonusHonorFromKill(2), player->GetTeamId());

    if (GetTeamScore(TEAM_ALLIANCE) == BG_TP_MAX_TEAM_SCORE || GetTeamScore(TEAM_HORDE) == BG_TP_MAX_TEAM_SCORE)
    {
        UpdateWorldState(BG_TP_STATE_TIMER_ACTIVE, 0);
        EndBattleground(GetTeamScore(TEAM_HORDE) == BG_TP_MAX_TEAM_SCORE ? TEAM_HORDE : TEAM_ALLIANCE);
    }
    else
        _bgEvents.ScheduleEvent(BG_TP_EVENT_RESPAWN_BOTH_FLAGS, BG_TP_FLAG_RESPAWN_TIME);

    _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT10);
    _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT15);
}

void BattlegroundTP::EventPlayerDroppedFlag(Player* player)
{
    if (GetFlagPickerGUID(TEAM_HORDE) != player->GetGUID() && GetFlagPickerGUID(TEAM_ALLIANCE) != player->GetGUID())
        return;

    SetFlagPicker(0, GetOtherTeamId(player->GetTeamId()));
    player->RemoveAurasDueToSpell(BG_TP_SPELL_HORDE_FLAG);
    player->RemoveAurasDueToSpell(BG_TP_SPELL_FOCUSED_ASSAULT);
    player->RemoveAurasDueToSpell(BG_TP_SPELL_BRUTAL_ASSAULT);

    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    player->CastSpell(player, SPELL_RECENTLY_DROPPED_FLAG, true);
    if (player->GetTeamId() == TEAM_ALLIANCE)
    {
        UpdateFlagState(TEAM_HORDE, BG_TP_FLAG_STATE_ON_GROUND);
        player->CastSpell(player, BG_TP_SPELL_HORDE_FLAG_DROPPED, true);
        SendMessageToAll(LANG_BG_TP_DROPPED_HF, CHAT_MSG_BG_SYSTEM_HORDE, player);
        _bgEvents.RescheduleEvent(BG_TP_EVENT_HORDE_DROP_FLAG, BG_TP_FLAG_DROP_TIME);
    }
    else
    {
        UpdateFlagState(TEAM_ALLIANCE, BG_TP_FLAG_STATE_ON_GROUND);
        player->CastSpell(player, BG_TP_SPELL_ALLIANCE_FLAG_DROPPED, true);
        SendMessageToAll(LANG_BG_TP_DROPPED_AF, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
        _bgEvents.RescheduleEvent(BG_TP_EVENT_ALLIANCE_DROP_FLAG, BG_TP_FLAG_DROP_TIME);
    }
}

void BattlegroundTP::EventPlayerClickedOnFlag(Player* player, GameObject* gameObject)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    player->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    // Alliance Flag picked up from base
    if (player->GetTeamId() == TEAM_HORDE && GetFlagState(TEAM_ALLIANCE) == BG_TP_FLAG_STATE_ON_BASE && BgObjects[BG_TP_OBJECT_A_FLAG] == gameObject->GetGUID())
    {
        SpawnBGObject(BG_TP_OBJECT_A_FLAG, RESPAWN_ONE_DAY);
        SetFlagPicker(player->GetGUID(), TEAM_ALLIANCE);
        UpdateFlagState(TEAM_ALLIANCE, BG_TP_FLAG_STATE_ON_PLAYER);
        Aura::TryRefreshStackOrCreate(sSpellMgr->GetSpellInfo(BG_TP_SPELL_ALLIANCE_FLAG), MAX_EFFECT_MASK, player, player);
        player->StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_SPELL_TARGET, BG_TP_SPELL_ALLIANCE_FLAG_PICKED);

        PlaySoundToAll(BG_TP_SOUND_ALLIANCE_FLAG_PICKED_UP);
        SendMessageToAll(LANG_BG_TP_PICKEDUP_AF, CHAT_MSG_BG_SYSTEM_HORDE, player);

        if (GetFlagState(TEAM_HORDE) != BG_TP_FLAG_STATE_ON_BASE)
        {
            _bgEvents.RescheduleEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT10, BG_TP_SPELL_FORCE_TIME);
            _bgEvents.RescheduleEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT15, BG_TP_SPELL_BRUTAL_TIME);
        }
        return;
    }

    // Horde Flag picked up from base
    if (player->GetTeamId() == TEAM_ALLIANCE && GetFlagState(TEAM_HORDE) == BG_TP_FLAG_STATE_ON_BASE && BgObjects[BG_TP_OBJECT_H_FLAG] == gameObject->GetGUID())
    {
        SpawnBGObject(BG_TP_OBJECT_H_FLAG, RESPAWN_ONE_DAY);
        SetFlagPicker(player->GetGUID(), TEAM_HORDE);
        UpdateFlagState(TEAM_HORDE, BG_TP_FLAG_STATE_ON_PLAYER);
        Aura::TryRefreshStackOrCreate(sSpellMgr->GetSpellInfo(BG_TP_SPELL_HORDE_FLAG), MAX_EFFECT_MASK, player, player);
        player->StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_SPELL_TARGET, BG_TP_SPELL_HORDE_FLAG_PICKED);

        PlaySoundToAll(BG_TP_SOUND_HORDE_FLAG_PICKED_UP);
        SendMessageToAll(LANG_BG_TP_PICKEDUP_HF, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);

        if (GetFlagState(TEAM_ALLIANCE) != BG_TP_FLAG_STATE_ON_BASE)
        {
            _bgEvents.RescheduleEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT10, BG_TP_SPELL_FORCE_TIME);
            _bgEvents.RescheduleEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT15, BG_TP_SPELL_BRUTAL_TIME);
        }
        return;
    }
    if (player->IsMounted())
    {
        player->Dismount();
        player->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }
    // Alliance Flag on ground
    if (GetFlagState(TEAM_ALLIANCE) == BG_TP_FLAG_STATE_ON_GROUND && player->IsWithinDistInMap(gameObject, 10.0f) && gameObject->GetEntry() == BG_OBJECT_A_FLAG_GROUND_TP_ENTRY)
    {
        SetDroppedFlagGUID(0, TEAM_ALLIANCE);
        if (player->GetTeamId() == TEAM_ALLIANCE)
        {
            UpdateFlagState(TEAM_ALLIANCE, BG_TP_FLAG_STATE_ON_BASE);
            SpawnBGObject(BG_TP_OBJECT_A_FLAG, RESPAWN_IMMEDIATELY);
            UpdatePlayerScore(player, SCORE_FLAG_RETURNS, 1);

            PlaySoundToAll(BG_TP_SOUND_FLAG_RETURNED);
            SendMessageToAll(LANG_BG_TP_RETURNED_AF, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
            _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT10);
            _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT15);
            RemoveAssaultAuras();
            return;
        }
        else
        {
            SetFlagPicker(player->GetGUID(), TEAM_ALLIANCE);
            UpdateFlagState(TEAM_ALLIANCE, BG_TP_FLAG_STATE_ON_PLAYER);
            Aura::TryRefreshStackOrCreate(sSpellMgr->GetSpellInfo(BG_TP_SPELL_ALLIANCE_FLAG), MAX_EFFECT_MASK, player, player);
            if (uint32 assaultSpellId = GetAssaultSpellId())
              player->CastSpell(player, assaultSpellId, true);

            PlaySoundToAll(BG_TP_SOUND_ALLIANCE_FLAG_PICKED_UP);
            SendMessageToAll(LANG_BG_TP_PICKEDUP_AF, CHAT_MSG_BG_SYSTEM_HORDE, player);
            return;
        }
    }

    // Horde Flag on ground
    if (GetFlagState(TEAM_HORDE) == BG_TP_FLAG_STATE_ON_GROUND && player->IsWithinDistInMap(gameObject, 10.0f) && gameObject->GetEntry() == BG_OBJECT_H_FLAG_GROUND_TP_ENTRY)
    {
        SetDroppedFlagGUID(0, TEAM_HORDE);
        if (player->GetTeamId() == TEAM_HORDE)
        {
            UpdateFlagState(TEAM_HORDE, BG_TP_FLAG_STATE_ON_BASE);
            SpawnBGObject(BG_TP_OBJECT_H_FLAG, RESPAWN_IMMEDIATELY);
            UpdatePlayerScore(player, SCORE_FLAG_RETURNS, 1);

            PlaySoundToAll(BG_TP_SOUND_FLAG_RETURNED);
            SendMessageToAll(LANG_BG_TP_RETURNED_HF, CHAT_MSG_BG_SYSTEM_HORDE, player);
            _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT10);
            _bgEvents.CancelEvent(BG_TP_EVENT_BOTH_FLAGS_KEPT15);
            RemoveAssaultAuras();
            return;
        }
        else
        {
            SetFlagPicker(player->GetGUID(), TEAM_HORDE);
            UpdateFlagState(TEAM_HORDE, BG_TP_FLAG_STATE_ON_PLAYER);
            Aura::TryRefreshStackOrCreate(sSpellMgr->GetSpellInfo(BG_TP_SPELL_HORDE_FLAG), MAX_EFFECT_MASK, player, player);
            if (uint32 assaultSpellId = GetAssaultSpellId())
              player->CastSpell(player, assaultSpellId, true);

            PlaySoundToAll(BG_TP_SOUND_HORDE_FLAG_PICKED_UP);
            SendMessageToAll(LANG_BG_TP_PICKEDUP_HF, CHAT_MSG_BG_SYSTEM_ALLIANCE, player);
            return;
        }
    }
}

void BattlegroundTP::RemovePlayer(Player* player)
{
    if (GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID() || GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
        EventPlayerDroppedFlag(player);
}

void BattlegroundTP::UpdateFlagState(TeamId teamId, uint32 value)
{
    _flagState[teamId] = value;
    UpdateWorldState(teamId == TEAM_ALLIANCE ? BG_TP_FLAG_STATE_HORDE : BG_TP_FLAG_STATE_ALLIANCE, value);
}

void BattlegroundTP::HandleAreaTrigger(Player* player, uint32 trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch(trigger)
    {
        case 5904:                                          // Alliance Flag spawn
            if (GetFlagState(TEAM_ALLIANCE) == BG_TP_FLAG_STATE_ON_BASE && GetFlagPickerGUID(TEAM_HORDE) == player->GetGUID())
                    EventPlayerCapturedFlag(player);
            break;
        case 5905:                                          // Horde Flag spawn
            if (GetFlagState(TEAM_HORDE) == BG_TP_FLAG_STATE_ON_BASE && GetFlagPickerGUID(TEAM_ALLIANCE) == player->GetGUID())
                EventPlayerCapturedFlag(player);
            break;
        case 5908:                                          // Horde Tower
        case 5909:                                          // Twin Peak House big
        case 5910:                                          // Horde House
        case 5911:                                          // Twin Peak House small
        case 5914:                                          // Alliance Start right
        case 5916:                                          // Alliance Start
        case 5917:                                          // Alliance Start left
        case 5918:                                          // Horde Start
        case 5920:                                          // Horde Start Front entrance
        case 5921:                                          // Horde Start left Water channel
            break;
        // default:
        //     Battleground::HandleAreaTrigger(player, trigger);
        //     break;
    }
}

bool BattlegroundTP::SetupBattleground()
{
        // flags
    AddObject(BG_TP_OBJECT_A_FLAG, BG_OBJECT_A_FLAG_TP_ENTRY, 2118.210f, 191.621f, 44.052f, 5.741259f, 0, 0, 0.9996573f, 0.02617699f, BG_TP_FLAG_RESPAWN_TIME/1000);
    AddObject(BG_TP_OBJECT_H_FLAG, BG_OBJECT_H_FLAG_TP_ENTRY, 1578.380f, 344.037f, 2.419f, 3.055978f, 0, 0, 0.008726535f, 0.9999619f, BG_TP_FLAG_RESPAWN_TIME/1000);
        // buffs
    AddObject(BG_TP_OBJECT_SPEEDBUFF_1, BG_OBJECTID_SPEEDBUFF_ENTRY, 1545.402f, 304.028f, 0.5923f, -1.64061f, 0, 0, 0.7313537f, -0.6819983f, BUFF_RESPAWN_TIME);
    AddObject(BG_TP_OBJECT_SPEEDBUFF_2, BG_OBJECTID_SPEEDBUFF_ENTRY, 2171.279f, 222.334f, 43.8001f, 2.663309f, 0, 0, 0.7313537f, 0.6819984f, BUFF_RESPAWN_TIME);
    AddObject(BG_TP_OBJECT_REGENBUFF_1, BG_OBJECTID_REGENBUFF_ENTRY, 1753.957f, 242.092f, -14.1170f, 1.105848f, 0, 0, 0.1305263f, -0.9914448f, BUFF_RESPAWN_TIME);
    AddObject(BG_TP_OBJECT_REGENBUFF_2, BG_OBJECTID_REGENBUFF_ENTRY, 1952.121f, 383.857f, -10.2870f, 4.192612f, 0, 0, 0.333807f, -0.9426414f, BUFF_RESPAWN_TIME);
    AddObject(BG_TP_OBJECT_BERSERKBUFF_1, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1934.369f, 226.064f, -17.0441f, 2.499154f, 0, 0, 0.5591929f, 0.8290376f, BUFF_RESPAWN_TIME);
    AddObject(BG_TP_OBJECT_BERSERKBUFF_2, BG_OBJECTID_BERSERKERBUFF_ENTRY, 1725.240f, 446.431f, -7.8327f, 5.709677f, 0, 0, 0.9396926f, -0.3420201f, BUFF_RESPAWN_TIME);
        // alliance gates
    AddObject(BG_TP_OBJECT_DOOR_A_1, BG_OBJECT_DOOR_A_1_TP_ENTRY, 2115.399f, 150.175f, 43.526f, 2.544690f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
    AddObject(BG_TP_OBJECT_DOOR_A_2, BG_OBJECT_DOOR_A_2_TP_ENTRY, 2156.803f, 220.331f, 43.482f, 2.544690f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
    AddObject(BG_TP_OBJECT_DOOR_A_3, BG_OBJECT_DOOR_A_3_TP_ENTRY, 2127.512f, 223.711f, 43.640f, 2.544690f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
    AddObject(BG_TP_OBJECT_DOOR_A_4, BG_OBJECT_DOOR_A_4_TP_ENTRY, 2096.102f, 166.920f, 54.230f, 2.544690f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
        // horde gates
    AddObject(BG_TP_OBJECT_DOOR_H_1, BG_OBJECT_DOOR_H_1_TP_ENTRY, 1556.595f, 314.502f, 1.2230f, 6.179126f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
    AddObject(BG_TP_OBJECT_DOOR_H_2, BG_OBJECT_DOOR_H_2_TP_ENTRY, 1587.093f, 319.853f, 1.5233f, 6.179126f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
    AddObject(BG_TP_OBJECT_DOOR_H_3, BG_OBJECT_DOOR_H_3_TP_ENTRY, 1591.463f, 365.732f, 13.494f, 6.179126f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);
    AddObject(BG_TP_OBJECT_DOOR_H_4, BG_OBJECT_DOOR_H_4_TP_ENTRY, 1558.315f, 372.709f, 1.4840f, 6.179126f, 0, 0, 0, 0, RESPAWN_IMMEDIATELY);

    GraveyardStruct const* sg = sGraveyard->GetGraveyard(TP_GRAVEYARD_MIDDLE_ALLIANCE);   
    AddSpiritGuide(TP_SPIRIT_ALLIANCE, sg->x, sg->y, sg->z, 3.641396f, TEAM_ALLIANCE);
    
    sg = sGraveyard->GetGraveyard(TP_GRAVEYARD_START_ALLIANCE);
    AddSpiritGuide(TP_SPIRIT_ALLIANCE, sg->x, sg->y, sg->z, 3.641396f, TEAM_ALLIANCE);
    
    sg = sGraveyard->GetGraveyard(TP_GRAVEYARD_MIDDLE_HORDE);
    AddSpiritGuide(TP_SPIRIT_HORDE, sg->x, sg->y, sg->z, 3.641396f, TEAM_HORDE);
    
    sg = sGraveyard->GetGraveyard(TP_GRAVEYARD_START_HORDE);
    AddSpiritGuide(TP_SPIRIT_ALLIANCE, sg->x, sg->y, sg->z, 3.641396f, TEAM_HORDE);

    for (uint32 i = BG_TP_OBJECT_DOOR_A_1; i < BG_TP_OBJECT_MAX; ++i)
        if (BgObjects[i] == 0)
        {
            sLog->outErrorDb("BatteGroundTP: Failed to spawn some object Battleground not created!");
            return false;
        }

    for (uint32 i = TP_SPIRIT_ALLIANCE; i < BG_CREATURES_MAX_TP; ++i)
        if (BgCreatures[i] == 0)
        {
            sLog->outErrorDb("BatteGroundTP: Failed to spawn spirit guides Battleground not created!");
            return false;
        }

    return true;
}

void BattlegroundTP::Init()
{
    //call parent's class reset
    Battleground::Init();

    _bgEvents.Reset();
    _flagKeepers[TEAM_ALLIANCE]     = 0;
    _flagKeepers[TEAM_HORDE]        = 0;
    _droppedFlagGUID[TEAM_ALLIANCE] = 0;
    _droppedFlagGUID[TEAM_HORDE]    = 0;
    _flagState[TEAM_ALLIANCE]       = BG_TP_FLAG_STATE_ON_BASE;
    _flagState[TEAM_HORDE]          = BG_TP_FLAG_STATE_ON_BASE;
    _lastFlagCaptureTeam            = TEAM_NEUTRAL;

    if (sBattlegroundMgr->IsBGWeekend(GetBgTypeID()))
    {
        _reputationCapture = 45;
        _honorWinKills = 3;
        _honorEndKills = 4;
    }
    else
    {
        _reputationCapture = 35;
        _honorWinKills = 1;
        _honorEndKills = 2;
    }
}

void BattlegroundTP::EndBattleground(TeamId winnerTeamId)
{
    // Win reward
    RewardHonorToTeam(GetBonusHonorFromKill(_honorWinKills), winnerTeamId);

    // Complete map_end rewards (even if no team wins)
    RewardHonorToTeam(GetBonusHonorFromKill(_honorEndKills), TEAM_ALLIANCE);
    RewardHonorToTeam(GetBonusHonorFromKill(_honorEndKills), TEAM_HORDE);

    Battleground::EndBattleground(winnerTeamId);
}

void BattlegroundTP::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    EventPlayerDroppedFlag(player);
    Battleground::HandleKillPlayer(player, killer);
}

void BattlegroundTP::UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor)
{
    BattlegroundScoreMap::iterator itr = PlayerScores.find(player->GetGUID());
    if (itr == PlayerScores.end())                         // player not found
        return;

    switch(type)
    {
        case SCORE_FLAG_CAPTURES:                           // flags captured
            ((BattlegroundTPScore*)itr->second)->FlagCaptures += value;
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, TP_OBJECTIVE_CAPTURE_FLAG);
            break;
        case SCORE_FLAG_RETURNS:                            // flags returned
            ((BattlegroundTPScore*)itr->second)->FlagReturns += value;
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE, TP_OBJECTIVE_RETURN_FLAG);
            break;
        default:
            Battleground::UpdatePlayerScore(player, type, value, doAddHonor);
            break;
    }
}

GraveyardStruct const* BattlegroundTP::GetClosestGraveyard(Player* player)
{
    if (GetStatus() == STATUS_IN_PROGRESS)
      return sGraveyard->GetGraveyard(player->GetTeamId() == TEAM_ALLIANCE ? TP_GRAVEYARD_MIDDLE_ALLIANCE : TP_GRAVEYARD_MIDDLE_HORDE);
    else
      return sGraveyard->GetGraveyard(player->GetTeamId() == TEAM_ALLIANCE ? TP_GRAVEYARD_FLAGROOM_ALLIANCE : TP_GRAVEYARD_FLAGROOM_HORDE);
}

void BattlegroundTP::FillInitialWorldStates(WorldPacket& data)
{
  data << uint32(BG_TP_FLAG_CAPTURES_ALLIANCE) << uint32(GetTeamScore(TEAM_ALLIANCE));
  data << uint32(BG_TP_FLAG_CAPTURES_HORDE) << uint32(GetTeamScore(TEAM_HORDE));
  data << uint32(BG_TP_FLAG_CAPTURES_MAX) << uint32(BG_TP_MAX_TEAM_SCORE);

  data << uint32(BG_TP_STATE_TIMER_ACTIVE) << uint32(GetStatus() == STATUS_IN_PROGRESS);
  data << uint32(BG_TP_STATE_TIMER) << uint32(GetMatchTime());

  data << uint32(BG_TP_FLAG_STATE_HORDE) << uint32(GetFlagState(TEAM_HORDE));
  data << uint32(BG_TP_FLAG_STATE_ALLIANCE) << uint32(GetFlagState(TEAM_ALLIANCE));
}

TeamId BattlegroundTP::GetPrematureWinner()
{
    if (GetTeamScore(TEAM_ALLIANCE) > GetTeamScore(TEAM_HORDE))
        return TEAM_ALLIANCE;
    else if (GetTeamScore(TEAM_HORDE) > GetTeamScore(TEAM_ALLIANCE))
        return TEAM_HORDE;

    return Battleground::GetPrematureWinner();
}

uint32 BattlegroundTP::GetAssaultSpellId() const
{
    if ((GetFlagPickerGUID(TEAM_ALLIANCE) == 0 && GetFlagState(TEAM_ALLIANCE) != BG_TP_FLAG_STATE_ON_GROUND) || 
        (GetFlagPickerGUID(TEAM_HORDE) == 0 && GetFlagState(TEAM_HORDE) != BG_TP_FLAG_STATE_ON_GROUND) || 
        _bgEvents.GetNextEventTime(BG_TP_EVENT_BOTH_FLAGS_KEPT10) > 0)
        return 0;

    return _bgEvents.GetNextEventTime(BG_TP_EVENT_BOTH_FLAGS_KEPT15) > 0 ? BG_TP_SPELL_FOCUSED_ASSAULT : BG_TP_SPELL_BRUTAL_ASSAULT;
}

void BattlegroundTP::RemoveAssaultAuras()
{
    if (Player* player = ObjectAccessor::GetObjectInMap(GetFlagPickerGUID(TEAM_ALLIANCE), this->FindBgMap(), (Player*)NULL))
    {
        player->RemoveAurasDueToSpell(BG_TP_SPELL_FOCUSED_ASSAULT);
        player->RemoveAurasDueToSpell(BG_TP_SPELL_BRUTAL_ASSAULT);
    }
    if (Player* player = ObjectAccessor::GetObjectInMap(GetFlagPickerGUID(TEAM_HORDE), this->FindBgMap(), (Player*)NULL))
    {
        player->RemoveAurasDueToSpell(BG_TP_SPELL_FOCUSED_ASSAULT);
        player->RemoveAurasDueToSpell(BG_TP_SPELL_BRUTAL_ASSAULT);
    }
}

bool BattlegroundTP::CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* player, Unit const* target, uint32 miscValue)
{
    switch (criteriaId)
    {
        case BG_CRITERIA_CHECK_SAVE_THE_DAY:
            return GetFlagState(player->GetTeamId()) == BG_TP_FLAG_STATE_ON_BASE;
    }

    return CheckAchievementCriteriaMeet(criteriaId, player, target, miscValue);
}

class TwinPeaksWorld : public WorldScript
{
	public:
    	TwinPeaksWorld() : WorldScript("TwinPeaksWorld") { }
};

void AddTwinPeaksScripts() {
	new TwinPeaksWorld();

	// Add Twin Peaks to battleground list
	BattlegroundMgr::queueToBg[BATTLEGROUND_QUEUE_TP] = BATTLEGROUND_TP;
	BattlegroundMgr::bgToQueue[BATTLEGROUND_TP] = BATTLEGROUND_QUEUE_TP;
	BattlegroundMgr::bgtypeToBattleground[BATTLEGROUND_TP] = new BattlegroundTP;

	BattlegroundMgr::bgTypeToTemplate[BATTLEGROUND_TP] = [](Battleground *bg_t) -> Battleground * { return new BattlegroundTP(*(BattlegroundTP *)bg_t); };

	BattlegroundMgr::getBgFromTypeID[BATTLEGROUND_TP] = [](WorldPacket* data, Battleground::BattlegroundScoreMap::const_iterator itr2, Battleground* /* bg */) {
		*data << uint32(0x00000002);                                          // count of next fields
		*data << uint32(((BattlegroundTPScore*)itr2->second)->FlagCaptures);  // flag captures
		*data << uint32(((BattlegroundTPScore*)itr2->second)->FlagReturns);   // flag returns
	};

	BattlegroundMgr::getBgFromMap[726] = [](WorldPacket* data, Battleground::BattlegroundScoreMap::const_iterator itr2) {
		*data << uint32(0x00000002);                                            // count of next fields
		*data << uint32(((BattlegroundTPScore*)itr2->second)->FlagCaptures);    // flag captures
		*data << uint32(((BattlegroundTPScore*)itr2->second)->FlagReturns);     // flag returns
	};

	GameObject::gameObjectToEventFlag[208208] = GameObject::gameObjectToEventFlag[208209] = [](Player* player, GameObject* gameObject, Battleground* bg) {
		if (bg->GetBgTypeID(true) == BATTLEGROUND_TP) {
			bg->EventPlayerClickedOnFlag(player, gameObject);
		}
	};

	Player::bgZoneIdToFillWorldStates[5005] = [](Battleground* bg, WorldPacket& data) {
		if (bg && bg->GetBgTypeID(true) == BATTLEGROUND_TP) {
			bg->FillInitialWorldStates(data);
		}
	};
}
