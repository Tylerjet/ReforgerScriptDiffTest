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
	private int m_messageType;
	
	private SCR_AIInfoComponent m_targetInfoComponent;
	private SCR_CallsignCharacterComponent m_CallsignComponent;
	private SCR_AIInfoComponent m_agentInfoComponent;
	private AudioHandle m_audioHandle = AudioHandle.Invalid;
	private VoNComponent m_vonComponent;
	private ref array<float> m_signals = new array<float>();
	
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
		
		GetVariableIn(PORT_SPEAKER,speaker);
		
		if (!speaker)
			speaker = owner;
		
		AIGroup pGr = AIGroup.Cast(speaker);
		AIAgent pAg = AIAgent.Cast(speaker);
		 
		if (pGr)
		{
			speaker = pGr.GetLeaderEntity();
			speakerGroup = SCR_AIGroup.Cast(pGr);
			if (speakerGroup && speakerGroup.IsSlave())
				return ENodeResult.SUCCESS;	// skipping RadioProtocol for leader of player's group
		}	
		else if (pAg)
		{
			speaker = pAg.GetControlledEntity();
			speakerGroup = SCR_AIGroup.Cast(pAg.GetParentGroup());	
			if (speakerGroup && speaker == speakerGroup.GetLeaderEntity() && speakerGroup.IsSlave())
				return ENodeResult.SUCCESS;	// skipping RadioProtocol for leader of player's group
		}	
		
		if (!m_vonComponent && speaker)
		{
			m_vonComponent = VoNComponent.Cast(speaker.FindComponent(VoNComponent));
		}
			
		if (!m_vonComponent || !speaker)
			return ENodeResult.SUCCESS;
		
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(speaker.FindComponent(SignalsManagerComponent));
		if(!signalsManagerComponent)
			return ENodeResult.SUCCESS;
		
		IEntity target;
		vector knownLocation, myLocation = speaker.GetOrigin();
		AIAgent targetAgent;
			
		GetVariableIn(PORT_TARGET, target);
		GetVariableIn(PORT_LOCATION, knownLocation);
		targetAgent = AIAgent.Cast(target);
		
		if(targetAgent)
		{
			IEntity targetEntity = targetAgent.GetControlledEntity();
			if (!targetEntity)
			{
				NodeError(this, owner, "invalid target entity provided");
				return ENodeResult.FAIL;
			}
			m_CallsignComponent = SCR_CallsignCharacterComponent.Cast(targetEntity.FindComponent(SCR_CallsignCharacterComponent));
			m_targetInfoComponent = GetInfoComponent(targetEntity);
		}		
		
		m_signals.Clear();

		int SN_Seed = signalsManagerComponent.FindSignal("Seed");
		SetSignal(SN_Seed, Math.RandomFloat(0, 1));
		
		switch (m_messageType)
		{
			case ECommunicationType.REPORT_TARGET:
			{	
				if (!target)
					return ENodeResult.FAIL;
				SetSignal_TargetValues(target, signalsManagerComponent);
				SetSignal_SoldierCalledValues(signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_TARGET, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOVE:
			{
				SetSignal_SoldierCalledValues(signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL);
				if (SetSignal_PositionValues(target, signalsManagerComponent, knownLocation,myLocation))
					m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_MID, m_signals, NORMAL_PRIORITY);
				else
					m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOVE_CLOSE,  m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_RETURN:
			{
				SetSignal_SoldierCalledValues(signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_RETURN, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT:
			{
				if (!target)
					return ENodeResult.FAIL;
				SetSignal_TargetValues(target, signalsManagerComponent);
				SetSignal_SoldierCalledValues(signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_BOARD, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOUNT_AS:
			{
				if (!m_CallsignComponent)
					return ENodeResult.FAIL;
				int role;
				if (!GetVariableIn(PORT_INT, role))
					role = 0;
					
				SetSignal_MountAsValues(signalsManagerComponent, m_CallsignComponent.GetCharacterCallsignIndex(), RoleInVehicleToSoundRoleEnum(role));
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_ROLE, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_UNMOUNT:
			{
				SetSignal_SoldierCalledValues(signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL);	
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_MOUNT_GETOUT, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_STOP:
			{
				if (m_CallsignComponent)
					SetSignal_SoldierCalledValues(signalsManagerComponent, m_CallsignComponent.GetCharacterCallsignIndex());
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_STOP, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_FLANK:
			{
				if (!m_CallsignComponent)
					return ENodeResult.FAIL;
				SetSignal_SoldierCalledValues(signalsManagerComponent, m_CallsignComponent.GetCharacterCallsignIndex());
				SetSignal_FlankValues(signalsManagerComponent, 0);	
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_FLANK, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_CONTACT:
			{
				if (!target)
					return ENodeResult.FAIL;
				SetSignal_TargetValues(target, signalsManagerComponent);
				SetSignal_PositionValues(target, signalsManagerComponent, knownLocation,myLocation);
				SetSignal_FactionValues(signalsManagerComponent, 1);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_SPOTTED_MID, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_NO_AMMO:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_NOAMMO, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_CLEAR:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_CLEAR, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_ENGAGING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_ENGAGING, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_TARGET_DOWN:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_STATUS_TARGETDOWN, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_RELOADING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_RELOAD, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_MOVING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_MOVE, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_COVERING:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_REPORTS_ACTIONSTATUS_COVER, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_DEFEND:
			{
				SetSignal_SoldierCalledValues(signalsManagerComponent, SIGNAL_VALUE_SOLDIER_ALL);
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_DEFEND_POSITION, m_signals, NORMAL_PRIORITY);
				break;
			}
			case ECommunicationType.REPORT_NEGATIVE:
			{
				m_vonComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_NEGATIVEFEEDBACK_NEGATIVE, m_signals, NORMAL_PRIORITY);
				break;
			}
		}
			
		return ENodeResult.SUCCESS;
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
	
	private SCR_AIInfoComponent GetInfoComponent(IEntity entity)
	{
		if (!entity)
			return null;
		AIAgent agent = AIAgent.Cast(entity);
		if (entity && !agent)
		{
			AIControlComponent cc = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
			if (!cc)
				return null;
			agent = cc.GetAIAgent();
			if (!agent)
				return null;
		}
		SCR_AIInfoComponent info = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
		if (!info)
			return null;
		return info;		
	}
	
//  ----------------------------------	
	
	private void SetSignal_TargetValues(IEntity target, SignalsManagerComponent signalsManagerComponent)
	{
		if (target.IsInherited(Vehicle))
		{
			int SN_Vehicle = signalsManagerComponent.FindSignal("Vehicle");
			
			SetSignal(SN_Vehicle, Vehicle.Cast(target).m_eVehicleType);
		}
									
		else if (target.IsInherited(ChimeraCharacter))
		{
			int SN_Character = signalsManagerComponent.FindSignal("Character");
			int unitRole = InfoUnitRoleToSoundCharacterEnum(m_targetInfoComponent);
			
			SetSignal(SN_Character, unitRole);
		}	
		//TODO: do Miscellaneous - now not objects to attacks
	}
	
	private bool SetSignal_PositionValues(IEntity target, SignalsManagerComponent signalsManagerComponent, vector locationTarget, vector locationSelf)
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
			
			SetSignal(SN_TargetDistance, distance);
			SetSignal(SN_DirectionAngle, directionAngle);
			return true;
		}
		return false;
	}
	
	private void SetSignal_SoldierCalledValues(SignalsManagerComponent signalsManagerComponent, int soldierCalled)
	{
		int SN_SoldierCalled = signalsManagerComponent.FindSignal("SoldierCalled");
		
		SetSignal(SN_SoldierCalled, soldierCalled);
	}
		
	private void SetSignal_MountAsValues(SignalsManagerComponent signalsManagerComponent, int soldierCalledId, int soldierRole)
	{
		int SN_SoldierCalled = signalsManagerComponent.FindSignal("SoldierCalled");
		int SN_Role = signalsManagerComponent.FindSignal("Role");
		
		SetSignal(SN_SoldierCalled, soldierCalledId);
		SetSignal(SN_Role, soldierRole);
	}
	
	private void SetSignal_FlankValues(SignalsManagerComponent signalsManagerComponent, int flank)
	{
		int SN_Flank = signalsManagerComponent.FindSignal("Flank");
		
		SetSignal(SN_Flank, flank);
	}
	
	private void SetSignal_FactionValues(SignalsManagerComponent signalsManagerComponent, int faction)
	{
		int SN_Faction = signalsManagerComponent.FindSignal("Faction");
		
		SetSignal(SN_Faction, faction);
	}
	
	private void SetSignal(int index, float value)
	{
		m_signals.Insert(index);
		m_signals.Insert(value);
	}
};
