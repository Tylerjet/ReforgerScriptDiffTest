//------------------------------------------------------------------------------------------------
class SCR_ContainerAIActionTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkAIAction", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SplitStringByUpperCase(string input, out string output)
	{
		output = "";
		bool wasPreviousUpperCase;
		int asciiChar;
		for (int i, count = input.Length(); i < count; i++)
		{
			asciiChar = input.ToAscii(i);
			bool isLowerCase = (asciiChar > 96 && asciiChar < 123);
			if (i > 0 && !wasPreviousUpperCase && !isLowerCase)
			{
				output += " " + asciiChar.AsciiToString();
				wasPreviousUpperCase = true;
			}
			else
			{
				 if (isLowerCase)
					wasPreviousUpperCase = false;
				output += asciiChar.AsciiToString();
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI Actions that will be executed on AIs spawned from that AI slot
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAI : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity for AI Action - if left empty, it will attempt to retrieve it from the ")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "AI actions that will be executed on target AI")];
	ref array<ref SCR_ScenarioFrameworkAIAction> m_aAIActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!m_Getter)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layer)
			{
				IEntity entity = layer.GetSpawnedEntity();
				if (!entity)
				{
					Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
					return;
				}
				
				SCR_AIGroup targetAIGroup = SCR_AIGroup.Cast(entity);
				if (!targetAIGroup)
				{
					Print(string.Format("ScenarioFramework Action: AI Group not found for Action %1.", this), LogLevel.ERROR);
					return;
				}
		
				foreach (SCR_ScenarioFrameworkAIAction AIAction : m_aAIActions)
				{
					AIAction.Init(targetAIGroup, object);
				}
			}

			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_AIGroup targetAIGroup = SCR_AIGroup.Cast(entity);
		if (!targetAIGroup)
		{
			Print(string.Format("ScenarioFramework Action: AI Group not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		foreach (SCR_ScenarioFrameworkAIAction AIAction : m_aAIActions)
		{
			AIAction.Init(targetAIGroup, object);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action base class
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAction
{
	SCR_AIGroup m_AIGroup;
	IEntity m_IEntity;

	//------------------------------------------------------------------------------------------------
	void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		if (!targetAIGroup)
			return;
		
		m_AIGroup = targetAIGroup;
		m_IEntity = entity;

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate();
}

//------------------------------------------------------------------------------------------------
//! AI action to add waypoint
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAddWaypoint : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "Target entity for waypoint")];
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	AIWaypoint m_Waypoint;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
		{
			layer.GetOnAllChildrenSpawned().Insert(OnWaypointSpawned);
			return;
		}
		
		m_Waypoint = AIWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_AIGroup.AddWaypoint(m_Waypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		layer.GetOnAllChildrenSpawned().Remove(OnWaypointSpawned);
		
		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
			return;
		
		m_Waypoint = AIWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
				
		m_AIGroup.AddWaypoint(m_Waypoint);
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to execute action on specific waypoint completetion
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionOnWaypointCompleted : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "Target entity for waypoint")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions that will be executed upon provided waypoint is completed for provided AI group")];
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnWaypointCompleted;
	
	AIWaypoint m_Waypoint;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
		{
			layer.GetOnAllChildrenSpawned().Insert(OnWaypointSpawned);
			return;
		}
		
		m_Waypoint = AIWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_AIGroup.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_AIGroup.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
		layer.GetOnAllChildrenSpawned().Remove(OnWaypointSpawned);
		
		IEntity spawnedEntity = layer.GetSpawnedEntity();
		if (!spawnedEntity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_Waypoint = AIWaypoint.Cast(spawnedEntity);
		if (!m_Waypoint)
		{
			Print(string.Format("ScenarioFramework Action: Waypoint not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		m_AIGroup.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnWaypointCompleted(AIWaypoint waypoint)
	{
		if (waypoint != m_Waypoint)
			return;
		
		m_AIGroup.GetOnWaypointCompleted().Remove(OnWaypointCompleted);
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnWaypointCompleted)
		{
			action.Init(m_IEntity);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to execute action on specific waypoint completetion
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionOnThreatStateChanged : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EAIThreatState.THREATENED.ToString(), UIWidgets.ComboBox, "On what Threat State will actions be activated", "", ParamEnumArray.FromEnum(EAIThreatState), category: "Common")]
	EAIThreatState m_eAIThreatState;
	
	[Attribute(desc: "Actions that will be executed upon provided waypoint is completed for provided AI group")];
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnThreatStateChanged;
	
	ref array<ref SCR_AIThreatSystem> m_aAIThreatSystems = {};
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (int i, AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
			
			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (!combatComponent)
				continue;

			SCR_AIInfoComponent infoComponent = combatComponent.GetAIInfoComponent();
			if (!infoComponent)
				continue;
			
			SCR_AIThreatSystem threatSystem = infoComponent.GetThreatSystem();
			if (!threatSystem)
				continue;
			
			m_aAIThreatSystems.Insert(threatSystem);
			threatSystem.GetOnThreatStateChanged().Insert(OnThreatStateChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnThreatStateChanged(EAIThreatState prevState, EAIThreatState newState)
	{
		if (newState != m_eAIThreatState)
			return;
		
		foreach(SCR_AIThreatSystem threatSystem : m_aAIThreatSystems)
		{
			threatSystem.GetOnThreatStateChanged().Remove(OnThreatStateChanged);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnThreatStateChanged)
		{
			action.Init(m_IEntity);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set skill
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetSkill : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EAISkill.REGULAR.ToString(), UIWidgets.ComboBox, "AI skill in combat", "", ParamEnumArray.FromEnum(EAISkill), category: "Common")]
	EAISkill m_eAISkill;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetAISkill(m_eAISkill);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set combat type
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetCombatType : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EAICombatType.NORMAL.ToString(), UIWidgets.ComboBox, "AI combat type", "", ParamEnumArray.FromEnum(EAICombatType), category: "Common")]
	EAICombatType m_eAICombatType;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetCombatType(m_eAICombatType);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set hold fire
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetHoldFire : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", desc: "If AI in the group should hold fire")]
	bool m_bHoldFire;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetHoldFire(m_bHoldFire);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set perception factor
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetPerceptionFactor : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Sets perception ability. Affects speed at which perception detects targets. Bigger value means proportionally faster detection.", params: "0 100 0.001", category: "Common")]
	float m_fPerceptionFactor;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (combatComponent)
				combatComponent.SetPerceptionFactor(m_fPerceptionFactor);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set formation
[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetFormation : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, "AI formation", "", ParamEnumArray.FromEnum(SCR_EAIGroupFormation), category: "Common")]
	SCR_EAIGroupFormation m_eAIGroupFormation;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (!formComp)
		{
			Print(string.Format("ScenarioFramework Action: AI Formation Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		formComp.SetFormation(SCR_Enum.GetEnumName(SCR_EAIGroupFormation, m_eAIGroupFormation));
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set character stance
//[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetCharacterStance : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: ECharacterStance.STAND.ToString(), UIWidgets.ComboBox, "AI character stance", "", ParamEnumArray.FromEnum(ECharacterStance), category: "Common")]
	ECharacterStance m_eAICharacterStance;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AIInfoComponent infoComponent = SCR_AIInfoComponent.Cast(agentEntity.FindComponent(SCR_AIInfoComponent));
			if (infoComponent)
				infoComponent.SetStance(m_eAICharacterStance);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! AI action to set movement type
//[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetMovementType : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EMovementType.WALK.ToString(), UIWidgets.ComboBox, "AI group formation", "", ParamEnumArray.FromEnum(EMovementType), category: "Common")]
	EMovementType m_eAIMovementType;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;

			SCR_AIInfoComponent infoComponent = SCR_AIInfoComponent.Cast(agentEntity.FindComponent(SCR_AIInfoComponent));
			if (infoComponent)
				infoComponent.SetMovementType(m_eAIMovementType);
		}
	}
}
