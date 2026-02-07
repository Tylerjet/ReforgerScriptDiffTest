//------------------------------------------------------------------------------------------------
class SCR_SeizingComponentClass : ScriptComponentClass
{
	[Attribute("{59A6F1EBC6C64F79}Prefabs/Logic/SeizingTrigger.et", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_sTriggerPrefab;

	ResourceName GetTriggerPrefab()
	{
		return m_sTriggerPrefab;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_SeizingComponent : ScriptComponent
{
	[Attribute("100")]
	protected float m_fAreaRadius;

	[Attribute("10")]
	protected float m_fMaximumSeizingTime;

	[Attribute("6")]
	protected float m_fMinimumSeizingTime;

	[Attribute("5", "How many characters need to be seizing at once to achieve the minimum seizing time.")]
	protected int m_iMaximumSeizingCharacters;

	[Attribute("0", desc: "How long after respawn is player able to start seizing or defending.")]
	protected float m_fRespawnCooldownPeriod;

	[Attribute("0", desc: "Allow seizing for playable factions only.")]
	protected bool m_bIgnoreNonPlayableAttackers;

	[Attribute("0", desc: "Allow defending for playable factions only.")]
	protected bool m_bIgnoreNonPlayableDefenders;

	static const float TRIGGER_CHECK_PERIOD_IDLE = 3;
	static const float TRIGGER_CHECK_PERIOD_ACTIVE = 1;

	[RplProp(onRplName: "OnSeizingEndTimestampChanged")]
	protected float m_fSeizingEndTimestamp;

	protected float m_fSeizingStartTimestamp;
	protected float m_fCurrentSeizingTime;
	protected SCR_Faction m_PrevailingFaction;
	protected BaseGameTriggerEntity m_Trigger;
	protected RplComponent m_RplComponent;
	protected bool m_bCharacterPresent;
	protected SCR_FactionAffiliationComponent m_FactionControl;
	protected float m_fTimer = Math.RandomFloat(-TRIGGER_CHECK_PERIOD_IDLE / 5, TRIGGER_CHECK_PERIOD_IDLE / 5);
	protected int m_iSeizingCharacters;
	protected ref map<int, float> m_mSpawnTimers = new map<int, float>();

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	protected void EvaluatePrevailingFaction()
	{
		array<IEntity> presentEntities = {};
		m_Trigger.QueryEntitiesInside();
		int presentEntitiesCnt = m_Trigger.GetEntitiesInside(presentEntities);
		m_bCharacterPresent = presentEntitiesCnt != 0;

		// Nobody is here, no need to evaluate
		if (!m_bCharacterPresent)
		{
			if (m_PrevailingFaction)
			{
				m_PrevailingFaction = null;
				OnPrevailingFactionChanged();
			}

			return;
		}

		map<SCR_Faction, int> factionsPresence = new map<SCR_Faction, int>();
		SCR_Faction evaluatedEntityFaction;

		int factionCnt;

		// Go through all entities and check their factions
		for (int i = 0; i < presentEntitiesCnt; i++)
		{
			evaluatedEntityFaction = EvaluateEntityFaction(presentEntities[i]);

			if (!evaluatedEntityFaction)
				continue;

			factionCnt = factionsPresence.Get(evaluatedEntityFaction);

			// If faction is not yet registered, do it now - otherwise just increase its presence counter
			if (factionCnt == 0)
				factionsPresence.Insert(evaluatedEntityFaction, 1);
			else
				factionsPresence.Set(evaluatedEntityFaction, factionCnt + 1);
		}

		SCR_Faction prevailingFaction;
		int highestAttackingPresence;
		int highestDefendingPresence;
		int curSeizingCharacters;
		int presence;

		// Evaluate the highest attacking presence
		for (int i = 0, cnt = factionsPresence.Count(); i < cnt; i++)
		{
			// Non-playable attackers are not allowed
			if (m_bIgnoreNonPlayableAttackers && !factionsPresence.GetKey(i).IsPlayable())
				continue;

			presence = factionsPresence.GetElement(i);

			if (presence > highestAttackingPresence)
			{
				highestAttackingPresence = presence;
				prevailingFaction = factionsPresence.GetKey(i);
			}
			else if (presence == highestAttackingPresence)	// When 2 or more factions have the same presence, none should prevail
			{
				prevailingFaction = null;
			}
		}

		// Evaluate the highest defending presence
		if (prevailingFaction)
		{
			for (int i = 0, cnt = factionsPresence.Count(); i < cnt; i++)
			{
				// Non-playable defenders are not allowed
				if (m_bIgnoreNonPlayableDefenders && !factionsPresence.GetKey(i).IsPlayable())
					continue;

				// This faction is already considered attacking
				if (factionsPresence.GetKey(i) == prevailingFaction)
					continue;

				highestDefendingPresence = Math.Max(factionsPresence.GetElement(i), highestDefendingPresence);
			}

			// Get net amount of players effectively seizing (clamp for max attackers attribute)
			if (prevailingFaction && highestAttackingPresence > highestDefendingPresence)
			{
				curSeizingCharacters = Math.Min(highestAttackingPresence - highestDefendingPresence, m_iMaximumSeizingCharacters);
			}
			else
			{
				prevailingFaction = null;
				curSeizingCharacters = 0;
			}
		}

		if (prevailingFaction != m_PrevailingFaction)
		{
			m_iSeizingCharacters = curSeizingCharacters;
			m_PrevailingFaction = prevailingFaction;
			OnPrevailingFactionChanged();
		}
		else if (prevailingFaction && curSeizingCharacters != m_iSeizingCharacters)
		{
			m_iSeizingCharacters = curSeizingCharacters;
			RefreshSeizingTimer();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Faction EvaluateEntityFaction(IEntity ent)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);

		if (!char)
			return null;

		CharacterControllerComponent comp = CharacterControllerComponent.Cast(char.FindComponent(CharacterControllerComponent));

		if (!comp)
			return null;

		if (comp.IsDead())
			return null;

		// Handle after-respawn cooldown
		if (m_fRespawnCooldownPeriod > 0)
		{
			int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);

			if (playerId != 0 && m_mSpawnTimers.Contains(playerId))
			{
				if (m_mSpawnTimers.Get(playerId) > Replication.Time())
					return null;
				else
					m_mSpawnTimers.Remove(playerId)
			}
		}

		SCR_Faction entityFaction = SCR_Faction.Cast(char.GetFaction());

		return entityFaction;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrevailingFactionChanged()
	{
		if (m_FactionControl.GetAffiliatedFaction() == m_PrevailingFaction || !m_PrevailingFaction)
		{
			if (m_fSeizingEndTimestamp != 0)
			{
				OnCaptureInterrupt();
				m_fSeizingEndTimestamp = 0;
				m_fSeizingStartTimestamp = 0;
			}
		}
		else
		{
			m_fSeizingStartTimestamp = Replication.Time();
			RefreshSeizingTimer();
			OnCaptureStart();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshSeizingTimer()
	{
		float seizingTimeVar = m_fMaximumSeizingTime - m_fMinimumSeizingTime;
		float deduct;

		if (m_iMaximumSeizingCharacters > 1)	// Avoid division by 0
		{
			float deductPerPlayer = seizingTimeVar / (m_iMaximumSeizingCharacters - 1);
			deduct = deductPerPlayer * (m_iSeizingCharacters - 1);
		}

		m_fSeizingEndTimestamp = m_fSeizingStartTimestamp + ((m_fMaximumSeizingTime - deduct) * 1000);
		Replication.BumpMe();
		OnSeizingEndTimestampChanged();
	}

	//------------------------------------------------------------------------------------------------
	float GetSeizingEndTimestamp()
	{
		return m_fSeizingEndTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCaptureStart()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCaptureInterrupt()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCaptureFinish()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSeizingEndTimestampChanged()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		if (!controlledEntity)
			return;

		// Player did not spawn in the area, ignore them
		if (vector.DistanceSqXZ(controlledEntity.GetOrigin(), GetOwner().GetOrigin()) > Math.Pow(m_fAreaRadius * 2, 2))
			return;

		m_mSpawnTimers.Set(playerId, Replication.Time() + (m_fRespawnCooldownPeriod * 1000))
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		// All functionality is server-side
		if (IsProxy())
			return;

		// Attributes check
		if (m_fAreaRadius <= 0)
		{
			Print("SCR_SeizingComponent: Invalid area radius (" + m_fAreaRadius + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (m_fMaximumSeizingTime < 0 || m_fMinimumSeizingTime < 0 || m_fMaximumSeizingTime < m_fMinimumSeizingTime)
		{
			Print("SCR_SeizingComponent: Invalid seizing time setting (" + m_fMinimumSeizingTime + ", " + m_fMaximumSeizingTime + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (m_iMaximumSeizingCharacters <= 0)
		{
			Print("SCR_SeizingComponent: Invalid maximum seizing characters (" + m_iMaximumSeizingCharacters + ")! Terminating...", LogLevel.ERROR);
			return;
		}

		if (!GetGame().InPlayMode())
			return;

		SCR_SeizingComponentClass componentData = SCR_SeizingComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;

		m_FactionControl = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));

		if (!m_FactionControl)
		{
			Print("SCR_SeizingComponent: Owner is missing SCR_FactionAffiliationComponent! Terminating...", LogLevel.ERROR);
			return;
		}

		Resource triggerResource = Resource.Load(componentData.GetTriggerPrefab());

		if (!triggerResource)
		{
			Print("SCR_SeizingComponent: Trigger resource failed to load! Terminating...", LogLevel.ERROR);
			return;
		}

		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = GetOwner().GetOrigin();

		// Spawn the trigger locally on server
		m_Trigger = BaseGameTriggerEntity.Cast(GetGame().SpawnEntityPrefabLocal(triggerResource, GetGame().GetWorld(), params));

		if (!m_Trigger)
		{
			Print("SCR_SeizingComponent: Trigger failed to spawn! Terminating...", LogLevel.ERROR);
			return;
		}

		m_Trigger.SetSphereRadius(m_fAreaRadius);

		// Register after-respawn cooldown method
		if (m_fRespawnCooldownPeriod > 0)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

			if (gameMode)
				gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		}

		SetEventMask(owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_fSeizingEndTimestamp != 0 && Replication.Time() >= m_fSeizingEndTimestamp)
		{
			m_fSeizingEndTimestamp = 0;
			m_fSeizingStartTimestamp = 0;

			if (m_FactionControl.GetAffiliatedFaction() != m_PrevailingFaction)
				OnCaptureFinish();
			else
				OnCaptureInterrupt();
		}

		m_fTimer += timeSlice;
		float baseTimer;

		// Switch to idle mode when nobody is in the area
		if (m_bCharacterPresent)
			baseTimer = TRIGGER_CHECK_PERIOD_ACTIVE;
		else
			baseTimer = TRIGGER_CHECK_PERIOD_IDLE;

		if (m_fTimer < baseTimer)
			return;

		// Evaluation for all bases should not be done at once, randomize the timer a bit
		m_fTimer = Math.RandomFloat(-baseTimer / 4, 0);

		EvaluatePrevailingFaction();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SeizingComponent()
	{
		if (m_Trigger)
			delete m_Trigger;

		m_mSpawnTimers.Clear();
		m_mSpawnTimers = null;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawned);
	}
};
