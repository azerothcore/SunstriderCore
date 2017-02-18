//Adapted and updated from TrinityCore : 2014-01-24

#ifndef TRINITY_SMARTSCRIPT_H
#define TRINITY_SMARTSCRIPT_H

#include "Common.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Unit.h"
#include "ConditionMgr.h"
#include "CreatureTextMgr.h"
#include "Spell.h"
#include "GridNotifiers.h"

#include "SmartScriptMgr.h"

class TC_GAME_API SmartScript
{
    public:
        SmartScript();
        ~SmartScript();

        void OnInitialize(WorldObject* obj, AreaTriggerEntry const* at = nullptr);
        void GetScript();
        void FillScript(SmartAIEventList e, WorldObject* obj, AreaTriggerEntry const* at);

        void ProcessEventsFor(SMART_EVENT e, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        void ProcessEvent(SmartScriptHolder& e, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        bool CheckTimer(SmartScriptHolder const& e) const;
        void RecalcTimer(SmartScriptHolder& e, uint32 min, uint32 max);
        void UpdateTimer(SmartScriptHolder& e, uint32 const diff);
        void InitTimer(SmartScriptHolder& e);
        void ProcessAction(SmartScriptHolder& e, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        void ProcessTimedAction(SmartScriptHolder& e, uint32 const& min, uint32 const& max, Unit* unit = nullptr, uint32 var0 = 0, uint32 var1 = 0, bool bvar = false, const SpellInfo* spell = nullptr, GameObject* gob = nullptr);
        /* Strips the list depending on the given target flags and the target type
        To improve: 
            Make it so that target flags are directly filtering target during search instead of altering the resulting list. 
            In the current form, target as "get the closest creature" do not interact well with target flags because they return only one target
            which can then be filtered out by target flags. Instead, we'd prefer returning the closest creature that match target flags.
        */
        void FilterByTargetFlags(SMARTAI_TARGETS type, SMARTAI_TARGETS_FLAGS flags, ObjectList& list, WorldObject const* caster);
        bool IsTargetAllowedByTargetFlags(WorldObject const* target, SMARTAI_TARGETS_FLAGS flags, WorldObject const* caster, SMARTAI_TARGETS type);
        //May return null, must be deleted after usage
        ObjectList* GetTargets(SmartScriptHolder const& e, Unit* invoker = nullptr);
        //returns a NEW object list, the called must delete if after usage
        ObjectList* GetWorldObjectsInDist(float dist);
        void InstallTemplate(SmartScriptHolder const& e);
        SmartScriptHolder CreateEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, PhaseMask phaseMask = PhaseMask(0));
        void AddEvent(SMART_EVENT e, uint32 event_flags, uint32 event_param1, uint32 event_param2, uint32 event_param3, uint32 event_param4, SMART_ACTION action, uint32 action_param1, uint32 action_param2, uint32 action_param3, uint32 action_param4, uint32 action_param5, uint32 action_param6, SMARTAI_TARGETS t, uint32 target_param1, uint32 target_param2, uint32 target_param3, PhaseMask phaseMask = PhaseMask(0));
        void SetPathId(uint32 id) { mPathId = id; }
        uint32 GetPathId() const { return mPathId; }
        WorldObject* GetBaseObject()
        {
            WorldObject* obj = nullptr;
            if (me)
                obj = me;
            else if (go)
                obj = go;
            return obj;
        }

        static bool IsUnit(WorldObject* obj)
        {
            return obj && (obj->GetTypeId() == TYPEID_UNIT || obj->GetTypeId() == TYPEID_PLAYER);
        }

        static bool IsPlayer(WorldObject* obj)
        {
            return obj && obj->GetTypeId() == TYPEID_PLAYER;
        }

        static bool IsCreature(WorldObject* obj)
        {
            return obj && obj->GetTypeId() == TYPEID_UNIT;
        }

        static bool IsCharmedCreature(WorldObject* obj)
        {
            if (!obj)
                return false;

            if (Creature* creatureObj = obj->ToCreature())
                return creatureObj->IsCharmed();

            return false;
        }

        static bool IsGameObject(WorldObject* obj)
        {
            return obj && obj->GetTypeId() == TYPEID_GAMEOBJECT;
        }

        void OnUpdate(const uint32 diff);
        void OnMoveInLineOfSight(Unit* who);

        Unit* DoSelectLowestHpFriendly(float range, uint32 MinHPDiff);
        void DoFindFriendlyCC(std::list<Creature*>& _list, float range);
        void DoFindFriendlyMissingBuff(std::list<Creature*>& list, float range, uint32 spellid);
        Unit* DoFindClosestOrFurthestFriendlyInRange(float range, bool playerOnly, bool nearest = true);

        void StoreTargetList(ObjectList* targets, uint32 id)
        {
            if (!targets)
                return;

            if (mTargetStorage->find(id) != mTargetStorage->end())
            {
                // check if already stored
                if ((*mTargetStorage)[id]->Equals(targets))
                    return;

                delete (*mTargetStorage)[id];
            }

            (*mTargetStorage)[id] = new ObjectGuidList(targets, GetBaseObject());
        }

        bool IsSmart(Creature* c = nullptr)
        {
            bool smart = true;
            if (c && c->GetAIName() != SMARTAI_AI_NAME)
                smart = false;

            if (!me || me->GetAIName() != SMARTAI_AI_NAME)
                smart = false;

            if (!smart)
                TC_LOG_ERROR("sql.sql","SmartScript: Action target Creature (GUID: %u Entry: %u) is not using SmartAI, action skipped to prevent crash.", c ? c->GetDBTableGUIDLow() : (me ? me->GetDBTableGUIDLow() : 0), c ? c->GetEntry() : (me ? me->GetEntry() : 0));

            return smart;
        }

        bool IsSmartGO(GameObject* g = nullptr)
        {
            bool smart = true;
            if (g && g->GetAIName() != SMARTAI_GOBJECT_AI_NAME)
                smart = false;

            if (!go || go->GetAIName() != SMARTAI_GOBJECT_AI_NAME)
                smart = false;
            if (!smart)
                TC_LOG_ERROR("sql.sql","SmartScript: Action target GameObject (GUID: %u Entry: %u) is not using SmartGameObjectAI, action skipped to prevent crash.", g ? g->GetDBTableGUIDLow() : (go ? go->GetDBTableGUIDLow() : 0), g ? g->GetEntry() : (go ? go->GetEntry() : 0));

            return smart;
        }

        ObjectList* GetTargetList(uint32 id)
        {
            auto itr = mTargetStorage->find(id);
            if (itr != mTargetStorage->end())
                return (*itr).second->GetObjectList();

            return nullptr;
        }

        void StoreCounter(uint32 id, uint32 value, uint32 reset)
        {
            CounterMap::iterator itr = mCounterList.find(id);
            if (itr != mCounterList.end())
            {
                if (reset == 0)
                    itr->second += value;
                else
                    itr->second = value;
            }
            else
                mCounterList.insert(std::make_pair(id, value));

            ProcessEventsFor(SMART_EVENT_COUNTER_SET, nullptr, id);
        }

        uint32 GetCounterValue(uint32 id) const
        {
            CounterMap::const_iterator itr = mCounterList.find(id);
            if (itr != mCounterList.end())
                return itr->second;
            return 0;
        }

        GameObject* FindGameObjectNear(WorldObject* searchObject, uint32 guid) const
        {
            /*
            auto bounds = searchObject->GetMap()->GetGameObjectBySpawnIdStore().equal_range(guid);
            if (bounds.first == bounds.second)
                return nullptr;

            return bounds.first->second;
            */

            GameObject *pGameObject = nullptr;

            CellCoord p(Trinity::ComputeCellCoord(searchObject->GetPositionX(), searchObject->GetPositionY()));
            Cell cell(p);
            cell.data.Part.reserved = ALL_DISTRICT;

            Trinity::GameObjectWithDbGUIDCheck goCheck(*searchObject, guid);
            Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck> checker(pGameObject, goCheck);
            
            TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > objectChecker(checker);
            cell.Visit(p, objectChecker, *searchObject->GetMap());

            return pGameObject;
        }

        Creature* FindCreatureNear(WorldObject* pSearchObject, uint32 guid) const
        {
            Creature *crea = nullptr;
            CellCoord p(Trinity::ComputeCellCoord(pSearchObject->GetPositionX(), pSearchObject->GetPositionY()));
            Cell cell(p);
            cell.data.Part.reserved = ALL_DISTRICT;

            Trinity::CreatureWithDbGUIDCheck target_check(pSearchObject, guid);
            Trinity::CreatureSearcher<Trinity::CreatureWithDbGUIDCheck> checker(crea, target_check);

            TypeContainerVisitor<Trinity::CreatureSearcher <Trinity::CreatureWithDbGUIDCheck>, GridTypeMapContainer > unit_checker(checker);
            cell.Visit(p, unit_checker, *pSearchObject->GetMap(), *pSearchObject, pSearchObject->GetVisibilityRange());

            return crea;
        }

        ObjectListMap* mTargetStorage;

        void OnReset();
        void ResetBaseObject()
        {
            if (meOrigGUID)
            {
                if (Creature* m = HashMapHolder<Creature>::Find(meOrigGUID))
                {
                    me = m;
                    go = nullptr;
                }
            }
            if (goOrigGUID)
            {
                if (GameObject* o = HashMapHolder<GameObject>::Find(goOrigGUID))
                {
                    me = nullptr;
                    go = o;
                }
            }
            goOrigGUID = 0;
            meOrigGUID = 0;
        }

        uint32 GetPhase() { return mEventPhase; }

        uint32 GetLastProcessedActionId() { return mLastProcessedActionId; }

        //TIMED_ACTIONLIST (script type 9 aka script9)
        void SetScript9(SmartScriptHolder& e, uint32 entry);
        Unit* GetLastInvoker(Unit* invoker = nullptr);
        uint64 mLastInvoker;
        uint32 mLastProcessedActionId;
        typedef std::unordered_map<uint32, uint32> CounterMap;
        CounterMap mCounterList;

    private:
        void IncPhase(int32 p)
        {
            uint32 previous = mEventPhase;

            if (p >= 0)
                mEventPhase += std::min<uint32>(SMART_EVENT_PHASE_COUNT, mEventPhase + p); // protect phase from overflowing
            else
                DecPhase(abs(p));

            if(mEventPhase != previous)
                ProcessEventsFor(SMART_EVENT_ENTER_PHASE, nullptr, mEventPhase);
        }

        void DecPhase(int32 p) 
        { 
            if(mEventPhase > (uint32)p)
                mEventPhase -= (uint32)p; 
            else
                mEventPhase = 0;
        }

        bool IsInPhase(PhaseMask phaseMask) const 
        { 
            if (mEventPhase == 0)
                return false;

            return ((1 << (mEventPhase - 1)) & phaseMask) != 0; 
        }

        void SetPhase(uint32 p = 0) 
        { 
            uint32 previous = mEventPhase;
            mEventPhase = p;
            if(mEventPhase != previous)
                ProcessEventsFor(SMART_EVENT_ENTER_PHASE, nullptr, mEventPhase);
        }

        SmartAIEventList mEvents;
        SmartAIEventList mInstallEvents;
        SmartAIEventList mTimedActionList;
        bool isProcessingTimedActionList;
        Creature* me;
        uint64 meOrigGUID;
        GameObject* go;
        uint64 goOrigGUID;
        AreaTriggerEntry const* trigger;
        SmartScriptType mScriptType;
        uint32 mEventPhase;

        std::unordered_map<int32, int32> mStoredDecimals;
        uint32 mPathId;
        SmartAIEventStoredList  mStoredEvents;
        std::vector<uint32>mRemIDs;

        uint32 mTextTimer;
        uint32 mLastTextID;
        uint32 mTalkerEntry;
        bool mUseTextTimer;

        SMARTAI_TEMPLATE mTemplate;
        void InstallEvents();

        void RemoveStoredEvent(uint32 id)
        {
            if (!mStoredEvents.empty())
            {
                for (auto i = mStoredEvents.begin(); i != mStoredEvents.end(); ++i)
                {
                    if (i->event_id == id)
                    {
                        mStoredEvents.erase(i);
                        return;
                    }
                }
            }
        }
        SmartScriptHolder FindLinkedEvent(uint32 link)
        {
            if (!mEvents.empty())
            {
                for (auto & mEvent : mEvents)
                {
                    if (mEvent.event_id == link)
                    {
                        return mEvent;
                    }
                }
            }
            SmartScriptHolder s;
            return s;
        }
};

#endif
