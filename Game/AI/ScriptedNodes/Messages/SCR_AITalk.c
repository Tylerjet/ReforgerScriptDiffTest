enum ECommunicationType
{
	NONE,
	REPORT_TARGET,
	REPORT_MOVE,
	REPORT_RETURN,
	REPORT_MOUNT,
	REPORT_MOUNT_AS,
	REPORT_UNMOUNT,
	REPORT_STOP,
	REPORT_FLANK,
	REPORT_DEFEND,
	REPORT_CONTACT,	
	REPORT_NO_AMMO,
	REPORT_UNDER_FIRE,
	REPORT_CLEAR,
	REPORT_ENGAGING,
	REPORT_TARGET_DOWN,
	REPORT_RELOADING,
	REPORT_MOVING,
	REPORT_COVERING,
	REPORT_NEGATIVE,
};

class SCR_AITalk: AITaskScripted
{
	static const string PORT_SPEAKER = "SpeakerIn";
	static const string PORT_TARGET = "TargetIn";
	static const string PORT_LOCATION = "LocationIn";
	static const string PORT_INT = "EnumIn";
	static const int NORMAL_PRIORITY = 50;
	static const int SIGNAL_VALUE_SOLDIER_ALL = 1000;
	
	[Attribute("0", UIWidgets.ComboBox, "Message type", "", ParamEnumArray.FromEnum(ECommunicationType) )]
	private ECommunicationType m_messageType;
	
	[Attribute("1", UIWidgets.CheckBox, "Should check if nearby friendly speakers are speaking? Might be slightly perf. heavy")]
	private bool m_bCheckNearbySpeakers;
	
	[Attribute("3", UIWidgets.ComboBox, "In case speaker(s) are busy what node result?", "", ParamEnumArray.FromEnum(ENodeResult) )]
	private ENodeResult m_eNodeResult;
	
	[Attribute("0.5", UIWidgets.Slider, "How often to check for speaker(s) states", "0.1 2.0 0.1")]
	private float m_fCheckPeriod;
	
	private SCR_AICommunicationState m_speakerCommunicationState;	
	
	//private SCR_CallsignCharacterComponent m_CallsignComponent;
	private AudioHandle m_audioHandle = AudioHandle.Invalid;
	private VoNComponent m_vonComponent;
	private SignalsManagerComponent m_signalsManagerComponent;
	private bool m_bIgnoreSpeakerState = false; 		// should speak no matter if speaker (owner) is already speaking or not
	private float m_fTimer = 0; 						// internal timer for frames	
	
	
//	-----------------------------------
	override void OnInit(AIAgent owner)
	{
		m_audioHandle = AudioHandle.Invalid;
	}
	
//	-----------------------------------	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		IEntity speaker;
		SCR_AIGroup speakerGroup;
		// evaluating once and then whole node returns running for m_fCheckPeriod time, then new evaluation happens
		if (m_fTimer > 0)
		{
			m_fTimer += dt;
			if (m_fTimer > m_fCheckPeriod)
				m_fTimer = 0;
			return ENodeResult.RUNNING;
		}
		
		m_fTimer += dt;	
		
		//  start of conditions if speaking at all, if repeat test later
		GetVariableIn(PORT_SPEAKER,speaker);
		
		if (!speaker)
			speaker = owner;
		
		AIGroup pGr = AIGroup.Cast(speaker);
		AIAgent pAg = AIAgent.Cast(speaker);
		 
		if (pGr)
		{
			speaker = pGr.GetLeaderEntity();
			pAg = pGr.GetLeaderAgent();
			speakerGroup = SCR_AIGroup.Cast(pGr);
			if (speakerGroup && speakerGroup.IsSlave())
				return ENodeResult.SUCCESS;	// skipping RadioProtocol for leader of player's group
		}	
		else if (pAg)
		{
			speaker = pAg.GetControlledEntity();
			pGr = pAg.GetParentGroup();
			speakerGroup = SCR_AIGroup.Cast(pAg.GetParentGroup());	
			if (speakerGroup && speaker == speakerGroup.GetLeaderEntity() && speakerGroup.IsSlave())
				return ENodeResult.SUCCESS;	// skipping RadioProtocol for leader of player's group
		}
				
		if (!pGr || pGr.GetAgentsCount() <= 1 || !speaker)
			return ENodeResult.SUCCESS;
		
		if (!pAg.GetControlComponent().IsAIActivated() || pAg.GetLOD() > 0) // ignoring speaking further from any player
			return ENodeResult.SUCCESS;
		
		if (!m_speakerCommunicationState)
		{
			m_speakerCommunicationState = GetCommunicationState(speaker);
			if (!m_speakerCommunicationState)
				m_bIgnoreSpeakerState = true;
		}	
		
		if (!m_vonComponent)
		{
			m_vonComponent = VoNComponent.Cast(speaker.FindComponent(VoNComponent));
		}
		
		if (!m_vonComponent)
			return ENodeResult.SUCCESS;
		
		if (!m_signalsManagerComponent)
		{
			m_signalsManagerComponent = SignalsManagerComponent.Cast(speaker.FindComponent(SignalsManagerComponent));
		}	
		if (!m_signalsManagerComponent)
			return ENodeResult.SUCCESS;
		
		
		if (!m_bIgnoreSpeakerState && m_speakerCommunicationState.IsSpeaking())
			return m_eNodeResult;
		
		if (m_bCheckNearbySpeakers && NearbyFriendlyAgentsSpeaking(speaker))
			return m_eNodeResult;
		
		//  end of conditions if speaking at all
		
		IEntity targetEntity;
		AIAgent targetAgent;
		
		if (GetVariableIn(PORT_TARGET, targetEntity))
		{
			targetAgent = AIAgent.Cast(targetEntity);
			if (targetAgent)
			{
				targetEntity = targetAgent.GetControlledEntity();
				if (!targetEntity)
				{
					NodeError(this, owner, "invalid target entity provided");
					return ENodeResult.FAIL;
				}
			}
		}
		if (!SetSignalsAndTransmit(m_messageType, targetEntity, speaker.GetOrigin()))
			return ENodeResult.FAIL;
		
		if (m_speakerCommunicationState)
			m_speakerCommunicationState.StartSpeaking();
		
		return ENodeResult.SUCCESS;
	}
	
//  -----------------------------------
	bool NearbyFriendlyAgentsSpeaking(notnull IEntity speakerEntity, float distanceSq = 100.0)
	{
		array<AIAgent> aiAgents = {};
		
		SCR_ChimeraCharacter char;
		auto speakerChar = SCR_ChimeraCharacter.Cast(speakerEntity);
		if (!speakerChar) // speaker is a non-character?
			return false;
		
		GetGame().GetAIWorld().GetAIAgents(aiAgents);
		foreach (AIAgent agent : aiAgents)
		{
			SCR_ChimeraAIAgent chimeraAIAgent = SCR_ChimeraAIAgent.Cast(agent);
			if (!chimeraAIAgent) // group or non-chimera?
				continue;
			IEntity ctrlEnt = agent.GetControlledEntity();
			if (!ctrlEnt || ctrlEnt == speakerEntity) // same entity as speaker?
				continue;
			if (vector.DistanceSq(ctrlEnt.GetOrigin(), speakerEntity.GetOrigin()) > distanceSq) // too far?
				continue;
			char = SCR_ChimeraCharacter.Cast(ctrlEnt);
			if (!char) // not chimeraCharacter?
				continue;
			Faction ctrlEntFac = char.GetFaction();
			if (!ctrlEntFac || ctrlEntFac != speakerChar.GetFaction()) // no faction or not same faction as speaker?
				continue;
			SCR_AICommunicationState commState = chimeraAIAgent.m_InfoComponent.m_CommunicationState;
			if (!commState) // no agent info?
				continue;
			if (commState.IsSpeaking()) // is agent speaking? if yes we are done with our search
				return true;
		}
		return false;
	}
	
//	-----------------------------------	
	
	bool SetSignalsAndTransmit(ECommunicationType commType, IEntity targetEntity, vector myLocation)
	{
		ref array<float> signals = {};
		SCR_CallsignCharacterComponent callsignComponent;
		vector signalLocation;
		int SN_Seed = m_signalsManagerComponent.FindSignal("Seed");
		// randomize variant of the signal
		SetSignal(SN_Seed, Math.RandomFloat(0, 1), signals);
		// different signals depending on cummunication type
		if (targetEntity)
			 callsignComponent = SCR_CallsignCharacterComponent.Cast(targetEntity.FindComponent(SCR_CallsignCharacterComponent));
		
		switch (commType)
		{
			case ECommunicationType.REPORT_TARGET:
			{
				if (!targetEntity)
					return false;
				SetSignal_TargetValues(targetEntity, m_signalsManagerComponent, signals);
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL, signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_TARGET, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOVE:
			{
				if (!GetVariableIn(PORT_LOCATION, signalLocation))
					return false;
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL, signals);
				if (SetSignal_PositionValues(targetEntity, m_signalsManagerComponent, signalLocation, myLocation, signals))
					m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_MID, signals, NORMAL_PRIORITY);
				else
					m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_CLOSE,  signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_CONTACT:
			{
				if (!targetEntity || !GetVariableIn(PORT_LOCATION, signalLocation))
					return false;
				SetSignal_TargetValues(targetEntity, m_signalsManagerComponent, signals);
				SetSignal_PositionValues(targetEntity, m_signalsManagerComponent, signalLocation, myLocation, signals);
				SetSignal_FactionValues(m_signalsManagerComponent, 1, signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_SPOTTED_MID, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_RETURN:
			{
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL, signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_RETURN, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT:
			{
				if (!targetEntity)
					return false;
				SetSignal_TargetValues(targetEntity, m_signalsManagerComponent, signals);
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL, signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_BOARD, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT_AS:
			{
				if (!callsignComponent)
					return false;
				int role;
				if (!GetVariableIn(PORT_INT, role))
					role = 0;
				
				SetSignal_MountAsValues(m_signalsManagerComponent, callsignComponent.GetCharacterCallsignIndex(), RoleInVehicleToSoundRoleEnum(role), signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_ROLE, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_UNMOUNT:
			{
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL, signals);	
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_GETOUT, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_STOP:
			{
				if (callsignComponent)
					SetSignal_SoldierCalledValues(m_signalsManagerComponent, callsignComponent.GetCharacterCallsignIndex(), signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_STOP, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_FLANK:
			{
				if (!callsignComponent)
					return false;
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, callsignComponent.GetCharacterCallsignIndex(), signals);
				SetSignal_FlankValues(m_signalsManagerComponent, 0, signals);	
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_FLANK, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_NO_AMMO:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_NOAMMO, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_CLEAR:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_CLEAR, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_ENGAGING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_ENGAGING, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_TARGET_DOWN:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_TARGETDOWN, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_RELOADING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_RELOAD, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOVING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_MOVE, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_COVERING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_COVER, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_DEFEND:
			{
				SetSignal_SoldierCalledValues(m_signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL, signals);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_DEFEND_POSITION, signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_NEGATIVE:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_NEGATIVEFEEDBACK_NEGATIVE, signals, NORMAL_PRIORITY);
				break;
			}
		}
		return true;
	}
//	-----------------------------------
	override bool VisibleInPalette()
    {
        return true;
    }
	
//	-----------------------------------	
	protected static ref TStringArray s_aVarsIn = {
		PORT_SPEAKER,
		PORT_TARGET,
		PORT_LOCATION,
		PORT_INT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
//	-----------------------------------
	
	private int InfoUnitRoleToSoundCharacterEnum(SCR_AIInfoComponent info)
	{
		if (!info)
			return -1;
		else if (info.HasRole(EUnitRole.MACHINEGUNNER))
			return ECP_Characters.MACHINE_GUNNER;
		else if (info.HasRole(EUnitRole.RIFLEMAN))
			return ECP_Characters.SOLDIER;
		return 0;
	}
	
//	-----------------------------------	
	private int RoleInVehicleToSoundRoleEnum(int role)
	{
		switch (role)
		{
			case ECompartmentType.Pilot:
			{
				return ECP_VehicleRoles.DRIVER;
			}
			case ECompartmentType.Turret:
			{
				return ECP_VehicleRoles.GUNNER;
			}
			case ECompartmentType.Cargo:
			{
				return ECP_VehicleRoles.PASSANGER;
			}
		}
		return role;
	}
	
//	-----------------------------------
	private SCR_AIInfoComponent GetInfoComponent(IEntity entity)
	{
		if (!entity)
			return null;
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(entity);
		if (entity && !agent)
		{
			AIControlComponent cc = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
			if (!cc)
				return null;
			agent = SCR_ChimeraAIAgent.Cast(cc.GetAIAgent());
			if (!agent)
				return null;
		}
		return agent.m_InfoComponent;	
	}
	
//	-----------------------------------	
	private SCR_AICommunicationState GetCommunicationState(IEntity entity)
	{
		if (!entity)
			return null;
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(entity);
		if (entity && !agent)
		{
			AIControlComponent cc = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
			if (!cc)
				return null;
			agent = SCR_ChimeraAIAgent.Cast(cc.GetAIAgent());
			if (!agent)
				return null;
		}
		return agent.m_InfoComponent.m_CommunicationState;	
	}
	
//  ----------------------------------	
	
	private void SetSignal_TargetValues(IEntity target, SignalsManagerComponent signalsManagerComponent, array<float> signals)
	{
		if (target.IsInherited(Vehicle))
		{
			int SN_Vehicle = signalsManagerComponent.FindSignal("Vehicle");
			
			SetSignal(SN_Vehicle, Vehicle.Cast(target).m_eVehicleType, signals);
		}
									
		else if (target.IsInherited(ChimeraCharacter))
		{
			int SN_Character = signalsManagerComponent.FindSignal("Character");
			SCR_AIInfoComponent targetInfoComponent = GetInfoComponent(target);
			int unitRole = InfoUnitRoleToSoundCharacterEnum(targetInfoComponent);
			
			SetSignal(SN_Character, unitRole, signals);
		}	
		//TODO: do Miscellaneous - now not objects to attacks
	}
	
	private bool SetSignal_PositionValues(IEntity target, SignalsManagerComponent signalsManagerComponent, vector locationTarget, vector locationSelf, array<float> signals)
	{
		if (target)
			locationTarget = target.GetOrigin();
		
		float distance = vector.Distance(locationTarget,locationSelf);
		if (distance > 40.0)
		{
			vector dirVec = locationTarget - locationSelf;
			float directionAngle = dirVec.ToYaw();
	
			int SN_TargetDistance = signalsManagerComponent.FindSignal("TargetDistance");
			int SN_DirectionAngle = signalsManagerComponent.FindSignal("DirectionAngle");
			
			SetSignal(SN_TargetDistance, distance, signals);
			SetSignal(SN_DirectionAngle, directionAngle, signals);
			return true;
		}
		return false;
	}
	
	private void SetSignal_SoldierCalledValues(SignalsManagerComponent signalsManagerComponent, int soldierCalled, array<float> signals)
	{
		int SN_SoldierCalled = signalsManagerComponent.FindSignal("SoldierCalled");
		
		SetSignal(SN_SoldierCalled, soldierCalled, signals);
	}
		
	private void SetSignal_MountAsValues(SignalsManagerComponent signalsManagerComponent, int soldierCalledId, int soldierRole, array<float> signals)
	{
		int SN_SoldierCalled = signalsManagerComponent.FindSignal("SoldierCalled");
		int SN_Role = signalsManagerComponent.FindSignal("Role");
		
		SetSignal(SN_SoldierCalled, soldierCalledId, signals);
		SetSignal(SN_Role, soldierRole, signals);
	}
	
	private void SetSignal_FlankValues(SignalsManagerComponent signalsManagerComponent, int flank, array<float> signals)
	{
		int SN_Flank = signalsManagerComponent.FindSignal("Flank");
		
		SetSignal(SN_Flank, flank, signals);
	}
	
	private void SetSignal_FactionValues(SignalsManagerComponent signalsManagerComponent, int faction, array<float> signals)
	{
		int SN_Faction = signalsManagerComponent.FindSignal("Faction");
		
		SetSignal(SN_Faction, faction, signals);
	}
	
	private void SetSignal(int index, float value, notnull array<float> signals)
	{
		signals.Insert(index);
		signals.Insert(value);
	}
	
//	-----------------------------------
	override protected bool CanReturnRunning()
	{
		return true;
	}
};
