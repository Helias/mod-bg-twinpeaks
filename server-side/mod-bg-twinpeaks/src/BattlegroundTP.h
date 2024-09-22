/*
 * Copyright (C) ArkCORE
 * Copyright (C) 2019+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: http://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
*/

#ifndef __BATTLEGROUNDTP_H
#define __BATTLEGROUNDTP_H

#include "Battleground.h"

enum TwinPeaksStrings {
    LANG_BG_TP_START_TWO_MINUTES        = 1230,
    LANG_BG_TP_START_ONE_MINUTE         = 1231,
    LANG_BG_TP_START_HALF_MINUTE        = 1232,
    LANG_BG_TP_HAS_BEGUN                = 1233,
    LANG_BG_TP_CAPTURED_HF              = 1234,
    LANG_BG_TP_CAPTURED_AF              = 1235,
    LANG_BG_TP_DROPPED_HF               = 1236,
    LANG_BG_TP_DROPPED_AF               = 1237,
    LANG_BG_TP_RETURNED_AF              = 1238,
    LANG_BG_TP_RETURNED_HF              = 1239,
    LANG_BG_TP_PICKEDUP_HF              = 1240,
    LANG_BG_TP_PICKEDUP_AF              = 1241,
    LANG_BG_TP_F_PLACED                 = 1242,
    LANG_BG_TP_ALLIANCE_FLAG_RESPAWNED  = 1243,
    LANG_BG_TP_HORDE_FLAG_RESPAWNED     = 1244
    // FREE IDS 1245-1249
};

enum BG_TP_Events
{
    BG_TP_EVENT_UPDATE_GAME_TIME    = 1,
    BG_TP_EVENT_NO_TIME_LEFT        = 2,
    BG_TP_EVENT_RESPAWN_BOTH_FLAGS  = 3,
    BG_TP_EVENT_ALLIANCE_DROP_FLAG  = 4,
    BG_TP_EVENT_HORDE_DROP_FLAG     = 5,
    BG_TP_EVENT_BOTH_FLAGS_KEPT10   = 6,
    BG_TP_EVENT_BOTH_FLAGS_KEPT15   = 7
};

enum BattlegroundCriteriaId
{
    BG_CRITERIA_CHECK_RESILIENT_VICTORY,
    BG_CRITERIA_CHECK_SAVE_THE_DAY,
    BG_CRITERIA_CHECK_EVERYTHING_COUNTS,
    BG_CRITERIA_CHECK_AV_PERFECTION,
    BG_CRITERIA_CHECK_DEFENSE_OF_THE_ANCIENTS,
    BG_CRITERIA_CHECK_NOT_EVEN_A_SCRATCH,
};

enum BG_TP_TimerOrScore
{
    BG_TP_MAX_TEAM_SCORE    = 3,
    BG_TP_TOTAL_GAME_TIME   = 25*MINUTE*IN_MILLISECONDS,
    BG_TP_FLAG_RESPAWN_TIME = 23*IN_MILLISECONDS,
    BG_TP_FLAG_DROP_TIME    = 10*IN_MILLISECONDS,
    BG_TP_SPELL_FORCE_TIME  = 10*MINUTE*IN_MILLISECONDS,
    BG_TP_SPELL_BRUTAL_TIME = 15*MINUTE*IN_MILLISECONDS
};

enum BG_TP_Sound
{
    BG_TP_SOUND_FLAG_CAPTURED_ALLIANCE  = 8173,
    BG_TP_SOUND_FLAG_CAPTURED_HORDE     = 8213,
    BG_TP_SOUND_FLAG_PLACED             = 8232,
    BG_TP_SOUND_FLAG_RETURNED           = 8192,
    BG_TP_SOUND_HORDE_FLAG_PICKED_UP    = 8212,
    BG_TP_SOUND_ALLIANCE_FLAG_PICKED_UP = 8174,
    BG_TP_SOUND_FLAGS_RESPAWNED         = 8232
};

enum BG_TP_SpellId
{
    BG_TP_SPELL_HORDE_FLAG              = 23333,
    BG_TP_SPELL_HORDE_FLAG_DROPPED      = 85003,
    BG_TP_SPELL_HORDE_FLAG_PICKED       = 61266,    // fake spell, does not exist but used as timer start event
    BG_TP_SPELL_ALLIANCE_FLAG           = 23335,
    BG_TP_SPELL_ALLIANCE_FLAG_DROPPED   = 85004,
    BG_TP_SPELL_ALLIANCE_FLAG_PICKED    = 61265,    // fake spell, does not exist but used as timer start event
    BG_TP_SPELL_FOCUSED_ASSAULT         = 46392,
    BG_TP_SPELL_BRUTAL_ASSAULT          = 46393
};

enum BG_TP_WorldStates
{
    BG_TP_FLAG_UNK_ALLIANCE       = 1545,
    BG_TP_FLAG_UNK_HORDE          = 1546,
    BG_TP_FLAG_CAPTURES_ALLIANCE  = 6101,
    BG_TP_FLAG_CAPTURES_HORDE     = 6102,
    BG_TP_FLAG_CAPTURES_MAX       = 6100,
    BG_TP_FLAG_STATE_HORDE        = 6110,
    BG_TP_FLAG_STATE_ALLIANCE     = 6111,
    BG_TP_STATE_TIMER             = 6120,
    BG_TP_STATE_TIMER_ACTIVE      = 6121
};

enum BG_TP_ObjectTypes
{
    BG_TP_OBJECT_DOOR_A_1        = 0,
    BG_TP_OBJECT_DOOR_A_2        = 1,
    BG_TP_OBJECT_DOOR_A_3        = 2,
    BG_TP_OBJECT_DOOR_A_4        = 3,
    BG_TP_OBJECT_DOOR_H_1        = 4,
    BG_TP_OBJECT_DOOR_H_2        = 5,
    BG_TP_OBJECT_DOOR_H_3        = 6,
    BG_TP_OBJECT_DOOR_H_4        = 7,
    BG_TP_OBJECT_A_FLAG          = 8,
    BG_TP_OBJECT_H_FLAG          = 9,
    BG_TP_OBJECT_SPEEDBUFF_1     = 10,
    BG_TP_OBJECT_SPEEDBUFF_2     = 11,
    BG_TP_OBJECT_REGENBUFF_1     = 12,
    BG_TP_OBJECT_REGENBUFF_2     = 13,
    BG_TP_OBJECT_BERSERKBUFF_1   = 14,
    BG_TP_OBJECT_BERSERKBUFF_2   = 15,
    BG_TP_OBJECT_MAX             = 16
};

enum BG_TP_ObjectEntry
{
    BG_OBJECT_DOOR_A_1_TP_ENTRY          = 206655,
    BG_OBJECT_DOOR_A_2_TP_ENTRY          = 206654,
    BG_OBJECT_DOOR_A_3_TP_ENTRY          = 206653,
    BG_OBJECT_DOOR_A_4_TP_ENTRY          = 206653,
    BG_OBJECT_DOOR_H_1_TP_ENTRY          = 208205,
    BG_OBJECT_DOOR_H_2_TP_ENTRY          = 208206,
    BG_OBJECT_DOOR_H_3_TP_ENTRY          = 208206,
    BG_OBJECT_DOOR_H_4_TP_ENTRY          = 208207,
    BG_OBJECT_A_FLAG_TP_ENTRY            = 179830,
    BG_OBJECT_H_FLAG_TP_ENTRY            = 179831,
    BG_OBJECT_A_FLAG_GROUND_TP_ENTRY     = 208208,
    BG_OBJECT_H_FLAG_GROUND_TP_ENTRY     = 208209
};

enum BG_TP_FlagState
{
    BG_TP_FLAG_STATE_ON_BASE      = 1,
    BG_TP_FLAG_STATE_ON_PLAYER    = 2,
    BG_TP_FLAG_STATE_ON_GROUND    = 3
};

enum BG_TP_Graveyards
{
    TP_GRAVEYARD_FLAGROOM_ALLIANCE  = 1726,
    TP_GRAVEYARD_FLAGROOM_HORDE     = 1727,
    TP_GRAVEYARD_START_ALLIANCE     = 1729, // not used
    TP_GRAVEYARD_START_HORDE        = 1728, // not used
    TP_GRAVEYARD_MIDDLE_ALLIANCE    = 1749,
    TP_GRAVEYARD_MIDDLE_HORDE       = 1750
};

enum BG_TP_CreatureTypes
{
    TP_SPIRIT_ALLIANCE    = 0,
    TP_SPIRIT_HORDE       = 1,

    BG_CREATURES_MAX_TP   = 2
};

enum BG_TP_Objectives
{
    TP_OBJECTIVE_CAPTURE_FLAG   = 42,
    TP_OBJECTIVE_RETURN_FLAG    = 44
};

#define TP_EVENT_START_BATTLE   8563

struct BattlegroundTPScore : public BattlegroundScore
{
    BattlegroundTPScore(Player *player) : BattlegroundScore(player), FlagCaptures(0), FlagReturns(0) { }
    ~BattlegroundTPScore() { }
    uint32 FlagCaptures;
    uint32 FlagReturns;
};

class BattlegroundTP : public Battleground
{
    friend class BattlegroundMgr;

    public:
        /* Construction */
        BattlegroundTP();
        ~BattlegroundTP();

        /* inherited from BattlegroundClass */
        void AddPlayer(Player* player);
        void StartingEventCloseDoors();
        void StartingEventOpenDoors();

        /* BG Flags */
        uint64 GetFlagPickerGUID(TeamId teamId) const { return _flagKeepers[teamId];  }
        void SetFlagPicker(uint64 guid, TeamId teamId) { _flagKeepers[teamId] = guid; }
        void RespawnFlagAfterDrop(TeamId teamId);
        uint8 GetFlagState(TeamId teamId) const { return _flagState[teamId]; }

        /* Battleground Events */
        void EventPlayerDroppedFlag(Player* player);
        void EventPlayerClickedOnFlag(Player* player, GameObject* gameObject);
        void EventPlayerCapturedFlag(Player* player);

        void RemovePlayer(Player* player);
        void HandleAreaTrigger(Player* player, uint32 trigger);
        void HandleKillPlayer(Player* player, Player* killer);
        bool SetupBattleground();
        void Init();
        void EndBattleground(TeamId winnerTeamId);
        GraveyardStruct const* GetClosestGraveyard(Player* player);

        void UpdateFlagState(TeamId teamId, uint32 value);
        void UpdatePlayerScore(Player* player, uint32 type, uint32 value, bool doAddHonor = true);
        void SetDroppedFlagGUID(uint64 guid, TeamId teamId) { _droppedFlagGUID[teamId] = guid; }
        uint64 GetDroppedFlagGUID(TeamId teamId) const { return _droppedFlagGUID[teamId];}
        void FillInitialWorldStates(WorldPacket& data);

        /* Scorekeeping */
        void AddPoints(TeamId teamId, uint32 points) { m_TeamScores[teamId] += points; }

        TeamId GetPrematureWinner();
        uint32 GetMatchTime() const { return 1 + (BG_TP_TOTAL_GAME_TIME - GetStartTime()) / (MINUTE*IN_MILLISECONDS); }
        uint32 GetAssaultSpellId() const;
        void RemoveAssaultAuras();

        /* Achievements*/
        bool CheckAchievementCriteriaMeet(uint32 criteriaId, Player const* source, Unit const* target = NULL, uint32 miscvalue1 = 0);

    private:
        EventMap _bgEvents;

        uint64 _flagKeepers[2];
        uint64 _droppedFlagGUID[2];
        uint8  _flagState[2];
        TeamId _lastFlagCaptureTeam;
        uint32 _reputationCapture;
        uint32 _honorWinKills;
        uint32 _honorEndKills;

        void PostUpdateImpl(uint32 diff);
};
#endif
