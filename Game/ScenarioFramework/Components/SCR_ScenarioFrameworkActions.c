class SCR_ContainerActionTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkAction", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] input
	//! \param[out] output
	void SplitStringByUpperCase(string input, out string output)
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

// TODO: make this a generic action which can be used anywhere anytime (i.e. on task finished, etc)
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Graph, params: "-1 10000 1", desc: "How many times this action can be performed if this gets triggered? Value -1 for infinity")]
	int					m_iMaxNumberOfActivations;

	IEntity				m_Entity;
	int					m_iNumberOfActivations;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	void Init(IEntity entity)
	{
		if (!SCR_BaseTriggerEntity.Cast(entity))
		{
			OnActivate(entity);
			m_Entity = entity;
			return;
		}

		m_Entity = entity;
		ScriptInvoker pOnActivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnActivate();
		if (pOnActivateInvoker)
			pOnActivateInvoker.Insert(OnActivate);

		ScriptInvoker pOnDeactivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnDeactivate();
		if (pOnDeactivateInvoker)
			pOnDeactivateInvoker.Insert(OnActivate);		//registering OnDeactivate to OnActivate - we need both changes
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanActivate()
	{
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (m_Entity)
				Print(string.Format("ScenarioFramework Action: Maximum number of activations reached for Action %1 attached on %2. Action won't do anything.", this, m_Entity.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: Maximum number of activations reached for Action %1. Action won't do anything.", this), LogLevel.ERROR);

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \out entity
	//! \return
	bool ValidateInputEntity(IEntity object, SCR_ScenarioFrameworkGet getter, out IEntity entity)
	{
		if (!getter && object)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("ScenarioFramework Action: Action %1 attached on %2 is not called from layer and won't do anything.", this, object.GetName()), LogLevel.ERROR);
				return false;
			}

			entity = layer.GetSpawnedEntity();
		}
		else
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
			if (!entityWrapper)
			{
				if (object)
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

				return false;
			}

			entity = IEntity.Cast(entityWrapper.GetValue());
		}

		if (!entity)
		{
			if (object)
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	void OnActivate(IEntity object);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] aObjectsNames
	//! \param[in] eActivationType
	void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType)
	{
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;

		foreach (string sObjectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!object)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName), LogLevel.ERROR);
				continue;
			}

			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", sObjectName), LogLevel.ERROR);
				continue;
			}

			layer.Init(null, eActivationType);
			layer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionIncrementCounter : SCR_ScenarioFrameworkActionBase
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Counter to increment")]
	string				m_sCounterName;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity entity)
	{
		if (!m_sCounterName)
			return;

		super.Init(entity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
		if (!entity)
		{
			PrintFormat("ScenarioFramework Action: Could not find %1 for Action %2", m_sCounterName, this, LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLogic counter = SCR_ScenarioFrameworkLogic.Cast(entity.FindComponent(SCR_ScenarioFrameworkLogic));
		if (counter)
		{
			counter.OnInput(1, object);
			return;
		}

		SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
		if (logicCounter)
			logicCounter.OnInput(1, object);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjects : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "These objects will spawn once the trigger becomes active.")]
	ref array<string> 	m_aNameOfObjectsToSpawnOnActivation;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SpawnObjects(m_aNameOfObjectsToSpawnOnActivation, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityPosition : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be teleported (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;

	[Attribute(defvalue: "0 0 0", desc: "Position that the entity will be teleported to")]
	vector 	m_vDestination;

	[Attribute(desc: "Name of the entity that above selected entity will be teleported to (Optional)")]
	ref SCR_ScenarioFrameworkGet m_DestinationEntityGetter;

	[Attribute(defvalue: "0 0 0", desc: "Position that will be used in relation to the entity for the position to teleport to (Optional)")]
	vector 	m_vDestinationEntityRelativePosition;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;

		if (!m_DestinationEntityGetter)
		{
			entity.SetOrigin(m_vDestination);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> destinationEntityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_DestinationEntityGetter.Get());
		if (!destinationEntityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Destination Entity Getter has issues for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		IEntity destinationEntity = IEntity.Cast(destinationEntityWrapper.GetValue());
		if (!destinationEntity)
		{
			Print(string.Format("ScenarioFramework Action: Destination Entity could not be found for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		entity.SetOrigin(destinationEntity.GetOrigin() + m_vDestinationEntityRelativePosition);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDeleteEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be deleted (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_EntityHelper.DeleteEntityAndChildren(entity);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeLayerTerminationStatus : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the layer to change the termination status")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "If layer will be terminated or not")]
	bool m_bTerminated;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layer)
				layer.SetIsTerminated(m_bTerminated);

			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Getter has issues for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity could not be found for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not LayerBase for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		layer.SetIsTerminated(m_bTerminated);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionKillEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be killed (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "If target entity is Character, it will randomize ragdoll upon death")]
	bool m_bRandomizeRagdoll;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_DamageManagerComponent damageMananager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
		if (damageMananager)
			damageMananager.Kill(Instigator.CreateInstigator(object));

		if (!m_bRandomizeRagdoll)
			return;

		CharacterAnimationComponent animationComponent = CharacterAnimationComponent.Cast(entity.FindComponent(CharacterAnimationComponent));
		if (!animationComponent)
		{
			Print(string.Format("ScenarioFramework Action: Entity does not have animation component needed for action %1. Action won't randomize the ragdoll.", this), LogLevel.ERROR);
			return;
		}

		Math.Randomize(-1);

		vector randomDir = "0 0 0";
		randomDir[0] = Math.RandomIntInclusive(1, 3);
		randomDir[1] = Math.RandomIntInclusive(1, 3);
		randomDir[2] = Math.RandomIntInclusive(1, 3);

		animationComponent.AddRagdollEffectorDamage("1 1 1", randomDir, Math.RandomFloatInclusive(0, 50), Math.RandomFloatInclusive(0, 10), Math.RandomFloatInclusive(0, 20));
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionEndMission : SCR_ScenarioFrameworkActionBase
{
	[Attribute(UIWidgets.CheckBox, desc: "If true, it will override any previously set game over type with selected one down bellow")]
	bool		m_bOverrideGameOverType;

	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))]
	EGameOverTypes			m_eOverriddenGameOverType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		if (m_bOverrideGameOverType)
			manager.SetMissionEndScreen(m_eOverriddenGameOverType);

		manager.Finish();
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionWaitAndExecute : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "How long to wait before executing action")]
	int							m_iDelayInSeconds;

	[Attribute(desc: "If this is set to a number larger than  Delay In Seconds, it will randomize resulted delay between these two values")]
	int m_iDelayInSecondsMax;

	[Attribute(UIWidgets.CheckBox, desc: "If true, it will activate actions in looped manner using Delay settings as the frequency. If randomized, it will randomize the time each time it loops.")]
	bool m_bLooped;

	[Attribute(defvalue: "1", desc: "Which actions will be executed once set time passes", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	protected int m_iDelay;
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	void ExecuteActions(IEntity object)
	{
		if (m_bLooped)
		{
			m_iDelay = m_iDelayInSeconds;
			Math.Randomize(-1);
			if (m_iDelayInSecondsMax > m_iDelayInSeconds)
				m_iDelay = Math.RandomIntInclusive(m_iDelayInSeconds, m_iDelayInSecondsMax);
		}

		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(object);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		m_iDelay = m_iDelayInSeconds;
		Math.Randomize(-1);
		if (m_iDelayInSecondsMax > m_iDelayInSeconds)
			m_iDelay = Math.RandomIntInclusive(m_iDelayInSeconds, m_iDelayInSecondsMax);

		//Used to delay the call as it is the feature of this action
		GetGame().GetCallqueue().CallLater(ExecuteActions, m_iDelay * 1000, m_bLooped, object);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLoopOverNotRandomlySelectedLayers : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Use GetRandomLayerBase")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "1", desc: "Which actions will be executed for each layer that was not randomly selected", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Missing Getter for action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkGetRandomLayerBase randomLayerGetter = SCR_ScenarioFrameworkGetRandomLayerBase.Cast(m_Getter);
		if (!randomLayerGetter)
		{
			Print(string.Format("ScenarioFramework Action: Used wrong Getter for Action %1. Use GetRandomLayerBase instead.", this), LogLevel.ERROR);
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

		string excludedEntity = entity.GetName();
		IEntity notSelectedEntity;

		foreach (string layer : randomLayerGetter.m_aNameOfLayers)
		{
			if (layer == excludedEntity)
				continue;

			notSelectedEntity = m_Getter.FindEntityByName(layer);
			if (!notSelectedEntity)
				continue;

			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(notSelectedEntity);
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCompareCounterAndExecute : SCR_ScenarioFrameworkActionBase
{

	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	SCR_EScenarioFrameworkComparisonOperator			m_eComparisonOperator;

	[Attribute(desc: "Value")]
	int							m_iValue;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Counter to increment")]
	string						m_sCounterName;

	[Attribute(defvalue: "1", desc: "What to do once counter is reached", UIWidgets.Auto, category: "OnActivate")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1 with provided name %1.", this, m_sCounterName), LogLevel.ERROR);
			return;
		}

		int counterValue;
		string className = entity.ClassName();
		if (className == "SCR_ScenarioFrameworkLogicCounter")
		{
			SCR_ScenarioFrameworkLogicCounter logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(entity);
			counterValue = logicCounter.m_iCnt;
		}

		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (counterValue < m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (counterValue <= m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (counterValue == m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 		&& (counterValue >= m_iValue)) ||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 			&& (counterValue > m_iValue))
		)
		{
			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(object);
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetMissionEndScreen : SCR_ScenarioFrameworkActionBase
{
	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))]
	EGameOverTypes			m_eGameOverType;

	[Attribute()]
	LocalizedString m_sSubtitle;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(gamemode.FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		manager.SetMissionEndScreen(m_eGameOverType);

		SCR_GameOverScreenManagerComponent gameOverScreenMgr = SCR_GameOverScreenManagerComponent.Cast(gamemode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverScreenMgr)
			return;

		SCR_GameOverScreenConfig m_GameOverScreenConfig = gameOverScreenMgr.GetGameOverConfig();
		if (!m_GameOverScreenConfig)
			return;

		SCR_BaseGameOverScreenInfo targetScreenInfo;
		m_GameOverScreenConfig.GetGameOverScreenInfo(m_eGameOverType, targetScreenInfo);
		if (!targetScreenInfo)
			return;

		SCR_BaseGameOverScreenInfoOptional optionalParams = targetScreenInfo.GetOptionalParams();
		if (!optionalParams)
			return;

		optionalParams.m_sSubtitle = m_sSubtitle;
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetBriefingEntryText : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute()]
	string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{
			if (journalEntry.GetEntryID() != m_iEntryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (!targetJournalEntry)
			return;

			targetJournalEntry.SetEntryText(WidgetManager.Translate(m_sTargetText));
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAppendBriefingEntryText : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute()]
	string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{
			if (journalEntry.GetEntryID() != m_iEntryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (!targetJournalEntry)
			return;

		string finalText = targetJournalEntry.GetEntryText() + "<br/>" + "<br/>" + m_sTargetText;
			targetJournalEntry.SetEntryText(finalText);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAppendBriefingEntryTextBasedOnTask : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute(desc: "From which task to fetch text")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{
			if (journalEntry.GetEntryID() != m_iEntryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (!targetJournalEntry)
			return;

		array<string> previousStrings = {};
		array<string> taskStrings = {};
		previousStrings = respawnBriefing.GetBriefingStringParamByID(m_iEntryID);
		if (previousStrings)
			taskStrings.InsertAll(previousStrings);

		taskStrings.Insert(task.GetTaskExecutionBriefing());
		taskStrings.Insert(task.GetSpawnedEntityName());

		respawnBriefing.RewriteEntry_SA(m_sFactionKey, m_iEntryID, targetJournalEntry.GetEntryText(), taskStrings);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetBriefingEntryTextBasedOnGeneratedTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute(desc: "Text that you want to use. Leave empty if you want to utilize the one set in config.")]
	string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{
			if (journalEntry.GetEntryID() != m_iEntryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);

		array<string> taskStrings = {};
		foreach (SCR_BaseTask task : tasks)
		{
			taskStrings.Insert(task.GetTitle());
			taskStrings.Insert("");
		}

		string tasksToShow;
		foreach (SCR_BaseTask task : tasks)
		{
			tasksToShow = tasksToShow + "<br/>" + string.Format(task.GetTitle());
		}

		if (!targetJournalEntry)
			return;

		if (m_sTargetText.IsEmpty())
			m_sTargetText = targetJournalEntry.GetEntryText();

		respawnBriefing.RewriteEntry_SA(m_sFactionKey, m_iEntryID, m_sTargetText, taskStrings);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetExecutionEntryTextBasedOnGeneratedTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	FactionKey m_sFactionKey;

	[Attribute()]
	int m_iEntryID;

	[Attribute(desc: "Text that you want to use. Leave empty if you want to utilize the one set in config.")]
	string m_sTargetText;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;

		SCR_RespawnBriefingComponent respawnBriefing = SCR_RespawnBriefingComponent.Cast(gamemode.FindComponent(SCR_RespawnBriefingComponent));
		if (!respawnBriefing)
			return;

		SCR_JournalSetupConfig journalSetupConfig = respawnBriefing.GetJournalSetup();
		if (!journalSetupConfig)
			return;

		SCR_JournalConfig journalConfig = journalSetupConfig.GetJournalConfig(m_sFactionKey);
		if (!journalConfig)
			return;

		array<ref SCR_JournalEntry> journalEntries = {};
		journalEntries = journalConfig.GetEntries();
		if (journalEntries.IsEmpty())
			return;

		SCR_JournalEntry targetJournalEntry;
		foreach (SCR_JournalEntry journalEntry : journalEntries)
		{
			if (journalEntry.GetEntryID() != m_iEntryID)
				continue;

			targetJournalEntry = journalEntry;
			break;
		}

		if (!targetJournalEntry)
			return;

		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);

		array<SCR_ScenarioFrameworkTask> frameworkTasks = {};
		foreach (SCR_BaseTask task : tasks)
		{
			if (SCR_ScenarioFrameworkTask.Cast(task))
				frameworkTasks.Insert(SCR_ScenarioFrameworkTask.Cast(task));
		}

		string tasksToShow;
		array<string> taskStrings = {};
		foreach (SCR_ScenarioFrameworkTask frameworkTask : frameworkTasks)
		{
			taskStrings.Insert(frameworkTask.GetTaskExecutionBriefing());
			taskStrings.Insert(frameworkTask.GetSpawnedEntityName());
		}

		if (m_sTargetText.IsEmpty())
			m_sTargetText = targetJournalEntry.GetEntryText();

		respawnBriefing.RewriteEntry_SA(m_sFactionKey, m_iEntryID, m_sTargetText, taskStrings);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionFeedParamToTaskDescription : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the slot task to influence the description parameter")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Which Prefabs and how many of them will be converted to a description string")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!m_Getter && object)
		{
			entity = object;
		}
		else if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
				return;

			entity = IEntity.Cast(entityWrapper.GetValue());
			if (!entity)
				return;
		}

		SCR_ScenarioFrameworkSlotTask slotTask = SCR_ScenarioFrameworkSlotTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotTask));
		if (!slotTask)
			return;

		string descriptionExtension;

		Resource resource;
		IEntitySource entitySource;
		string displayName;
		IEntityComponentSource editableEntitySource;
		IEntityComponentSource weaponEntitySource;
		IEntityComponentSource inventoryEntitySource;

		foreach (SCR_ScenarioFrameworkPrefabFilterCount filter : m_aPrefabFilter)
		{
			resource = Resource.Load(filter.m_sSpecificPrefabName);
			if (!resource || !resource.IsValid())
				continue;

			entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
			if (!entitySource)
				return;

			editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
			weaponEntitySource = SCR_ComponentHelper.GetWeaponComponentSource(entitySource);
			inventoryEntitySource = SCR_ComponentHelper.GetInventoryItemComponentSource(entitySource);

			if (editableEntitySource)
			{
				SCR_EditableEntityUIInfo editableEntityUiInfo = SCR_EditableEntityComponentClass.GetInfo(editableEntitySource);
				if (editableEntityUiInfo)
					displayName = editableEntityUiInfo.GetName();
			}
			else if (weaponEntitySource)
			{
				WeaponUIInfo weaponEntityUiInfo = SCR_ComponentHelper.GetWeaponComponentInfo(weaponEntitySource);
				if (weaponEntityUiInfo)
					displayName = weaponEntityUiInfo.GetName();
			}
			else if (inventoryEntitySource)
			{
				SCR_ItemAttributeCollection inventoryEntityUiInfo = SCR_ComponentHelper.GetInventoryItemInfo(inventoryEntitySource);
				if (inventoryEntityUiInfo)
				{
					UIInfo uiInfo = inventoryEntityUiInfo.GetUIInfo();
					if (uiInfo)
						displayName = uiInfo.GetName();
					else
						continue;
				}
				else
				{
					continue;
				}
			}

			int count = filter.m_iPrefabCount;
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(descriptionExtension))
				descriptionExtension += count.ToString() + "x " + displayName;
			else
				descriptionExtension += ", " + count.ToString() + "x " + displayName;
		}

		slotTask.m_TaskLayer.m_SupportEntity.SetSpawnedEntityName(slotTask.m_TaskLayer.m_Task, descriptionExtension);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowHint : SCR_ScenarioFrameworkActionBase
{
	[Attribute()]
	string		m_sTitle;

	[Attribute()]
	string		m_sText;

	[Attribute(defvalue: "15")]
	int			m_iTimeout;

	[Attribute()]
	FactionKey m_sFactionKey;

	[Attribute(desc: "Getter to get either a specific player or array of player entities")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerID = -1;
		if (EntityUtils.IsPlayer(object))
			playerID = playerManager.GetPlayerIdFromControlledEntity(object);

		array<IEntity> aEntities;

		if (m_Getter)
		{
			// Getter takes the priority. We set it back to -1 in case that object was player.
			playerID = -1;

			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!arrayOfEntitiesWrapper)
					return;

					aEntities = array<IEntity>.Cast(arrayOfEntitiesWrapper.GetValue());
					if (!aEntities)
						return;
	}
			else
			{
				IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
				if (entityFrom)
					playerID = playerManager.GetPlayerIdFromControlledEntity(entityFrom);
			}
		}

		if (!aEntities)
		{
			manager.ShowHint(m_sText, m_sTitle, m_iTimeout, m_sFactionKey, playerID);
		}
		else
		{
			foreach (IEntity entity : aEntities)
			{
				if (!EntityUtils.IsPlayer(entity))
					continue;

				playerID = playerManager.GetPlayerIdFromControlledEntity(entity);
				manager.ShowHint(m_sText, m_sTitle, m_iTimeout, m_sFactionKey, playerID);
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowPopupNotification : SCR_ScenarioFrameworkActionBase
{
	[Attribute()]
	string m_sTitle;

	[Attribute()]
	string m_sText;

	[Attribute()]
	FactionKey m_sFactionKey;

	[Attribute(desc: "Getter to get either a specific player or array of player entities")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerID = -1;
		if (EntityUtils.IsPlayer(object))
			playerID = playerManager.GetPlayerIdFromControlledEntity(object);

		array<IEntity> aEntities;

		if (m_Getter)
		{
			// Getter takes the priority. We set it back to -1 in case that object was player.
			playerID = -1;

			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!arrayOfEntitiesWrapper)
					return;

					aEntities = array<IEntity>.Cast(arrayOfEntitiesWrapper.GetValue());
					if (!aEntities)
						return;
			}
			else
			{
				IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
				if (entityFrom)
					playerID = playerManager.GetPlayerIdFromControlledEntity(entityFrom);
			}
		}

		if (!aEntities)
		{
			manager.PopUpMessage(m_sText, m_sTitle, m_sFactionKey, playerID);
		}
		else
		{
			foreach (IEntity entity : aEntities)
			{
				if (!EntityUtils.IsPlayer(entity))
					continue;

				playerID = playerManager.GetPlayerIdFromControlledEntity(entity);
				manager.PopUpMessage(m_sText, m_sTitle, m_sFactionKey, playerID);
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnClosestObjectFromList : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "The closest one from the list will be spawned")]
	ref array<string> 	m_aListOfObjects;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!m_Getter)
		{
			if (object)
			{
				entity = object;
			}
			else
			{
			Print(string.Format("ScenarioFramework Action: The object the distance is calculated from is missing!"), LogLevel.ERROR);
			return;
		}
		}

		if (!entity)
		{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			{
				Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
				return;
			}

			entity = IEntity.Cast(entityWrapper.GetValue());
		}

		IEntity entityInList;
		SCR_ScenarioFrameworkLayerBase selectedLayer;
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Getter returned null object. Random object spawned instead."), LogLevel.WARNING);
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_aListOfObjects[m_aListOfObjects.GetRandomIndex()]);

			entityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				return;
			}

			SpawnObjects(aRandomObjectToSpawn, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
			return;
		}

		IEntity closestEntity;
		float fDistance = float.MAX;
		foreach (string sObjectName : m_aListOfObjects)
		{
			entityInList = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Object %1 doesn't exist", sObjectName), LogLevel.ERROR);
				continue;
			}
			
			selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(entityInList.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!selectedLayer)
				continue;

			float fActualDistance = Math.AbsFloat(vector.Distance(entity.GetOrigin(), entityInList.GetOrigin()));

			if (fActualDistance < fDistance)
			{
				closestEntity = entityInList;
				fDistance = fActualDistance;
			}
		}

		if (!closestEntity)
			return;

		selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(closestEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));

		if (selectedLayer)
		{
			selectedLayer.Init(null, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
			selectedLayer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
		}
		else
		{
			Print(string.Format("ScenarioFramework Action: Can't spawn slot %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", closestEntity.GetName()), LogLevel.ERROR);
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjectBasedOnDistance : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Measure distance from what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "It will select only objects that are at least x amount of meters away")]
	int m_iMinDistance;

	[Attribute(desc: "You can also set max distance to setup the hard limit of the max distance - but be aware that there might be a situation where it would not spawn anything.")]
	int m_iMaxDistance;

	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "List of objects that are to be compared")]
	ref array<string> 	m_aListOfObjects;

	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all objects, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkSpawnChildrenType))]
	SCR_EScenarioFrameworkSpawnChildrenType m_SpawnObjects;

	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1")]
	int m_iRandomPercent;

	//------------------------------------------------------------------------------------------------
	void SpawnRandomObject(notnull array<string> aObjectsNames)
	{
		IEntity object = GetGame().GetWorld().FindEntityByName(aObjectsNames.GetRandomElement());
		if (!object)
		{
			Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", object), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", object), LogLevel.ERROR);
			return;
		}

		layer.Init(null, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnRandomMultipleObjects(notnull array<string> aObjectsNames)
	{
		array<SCR_ScenarioFrameworkLayerBase> aChildren = {};
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;
		SCR_ScenarioFrameworkLayerBase cachedLayer;
		foreach (string objectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(objectName);
			if (!object)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", objectName), LogLevel.ERROR);
				continue;
			}

			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", objectName), LogLevel.ERROR);
				continue;
			}

			if (!cachedLayer)
				cachedLayer = layer;

			if (!aChildren.Contains(layer))
				aChildren.Insert(layer);
		}

		if (aChildren.IsEmpty())
			return;

		if (m_SpawnObjects == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
			m_iRandomPercent = Math.Ceil(cachedLayer.GetPlayersCount() / cachedLayer.GetMaxPlayersForGameMode() * 100);

		int randomMultipleNumber = Math.Round(aObjectsNames.Count() * 0.01 * m_iRandomPercent);
		SCR_ScenarioFrameworkLayerBase child;
		for (int i = 1; i <= randomMultipleNumber; i++)
		{
			if (aChildren.IsEmpty())
				break;

			Math.Randomize(-1);
			child = aChildren.GetRandomElement();
			child.Init(null, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
			child.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
			aChildren.RemoveItem(child);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entityFrom;
		if (!m_Getter)
		{
			if (object)
			{
				entityFrom = object;
			}
			else
			{
				Print(string.Format("ScenarioFramework Action: The object the distance is calculated from is missing!"), LogLevel.ERROR);
				return;
			}
		}

		array<IEntity> aEntities = {};

		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!arrayOfEntitiesWrapper)
				{
					Print(string.Format("ScenarioFramework Action: Issue with Array Getter detected for Action %1.", this), LogLevel.ERROR);
					return;
				}

					aEntities = array<IEntity>.Cast(arrayOfEntitiesWrapper.GetValue());
					if (!aEntities)
					{
						Print(string.Format("ScenarioFramework Action: Issue with retrieved array detected for Action %1.", this), LogLevel.ERROR);
						return;
					}

				if (!entityFrom && entityWrapper)
					entityFrom = IEntity.Cast(entityWrapper.GetValue());
			}
		}

		bool entitiesAreEmpty = aEntities.IsEmpty();

		Math.Randomize(-1);
		IEntity entityInList;
		SCR_ScenarioFrameworkLayerBase selectedLayer;
		if (!entityFrom && entitiesAreEmpty)
		{
			Print(string.Format("ScenarioFramework Action: Getter returned null object. Random object spawned instead."), LogLevel.WARNING);
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_aListOfObjects[m_aListOfObjects.GetRandomIndex()]);

			entityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Object %1 doesn't exist", aRandomObjectToSpawn[0]), LogLevel.ERROR);
				return;
			}

			SpawnObjects(aRandomObjectToSpawn, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
			return;
		}

		array<string> aObjectsNames = {};

		foreach (string objectName : m_aListOfObjects)
		{
			entityInList = GetGame().GetWorld().FindEntityByName(objectName);
			if (!entityInList)
			{
				Print(string.Format("ScenarioFramework Action: Object %1 doesn't exist", objectName), LogLevel.ERROR);
				continue;
			}

			if (entitiesAreEmpty)
			{
				float fActualDistance = Math.AbsFloat(vector.Distance(entityFrom.GetOrigin(), entityInList.GetOrigin()));

				if (fActualDistance <= m_iMaxDistance && fActualDistance >= m_iMinDistance)
					aObjectsNames.Insert(objectName)
			}
			else
			{
				bool entityInRange;
				foreach (IEntity targetEntity : aEntities)
				{
					float fActualDistance = Math.AbsFloat(vector.Distance(targetEntity.GetOrigin(), entityInList.GetOrigin()));

					if (fActualDistance <= m_iMaxDistance && fActualDistance >= m_iMinDistance)
					{
						entityInRange = true;
					}
					else
					{
						entityInRange = false;
						break;
					}
				}

				if (entityInRange)
					aObjectsNames.Insert(objectName)
			}
		}

		if (m_SpawnObjects == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			//We need to introduce slight delay for the randomization by time seed to occur
			GetGame().GetCallqueue().CallLater(SpawnRandomObject, Math.RandomInt(1, 10), false, aObjectsNames);
			return;
		}

		if (m_SpawnObjects == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			foreach (string objectName : aObjectsNames)
			{
				entityInList = GetGame().GetWorld().FindEntityByName(objectName);
				selectedLayer = SCR_ScenarioFrameworkLayerBase.Cast(entityInList.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (selectedLayer)
				{
					SCR_ScenarioFrameworkArea area = selectedLayer.GetParentArea();
					if (!area)
						continue;

					area.SetAreaSelected(true);
					SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(selectedLayer);
					if (layerTask)
						selectedLayer.GetParentArea().SetLayerTask(layerTask);

					selectedLayer.Init(area, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
					selectedLayer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
				}
				else
				{
					Print(string.Format("ScenarioFramework Action: Can't spawn slot %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", selectedLayer.GetName()), LogLevel.ERROR);
				}
			}

			return;
		}

		SpawnRandomMultipleObjects(aObjectsNames);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionItemSafeguard : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions that will be executed when target item is dropped", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemDropped;
	
	[Attribute(desc: "Actions that will be executed when target item is possesed by someone/something", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemPossessed;
	
	protected IEntity m_ItemEntity;
	
	//------------------------------------------------------------------------------------------------
	void OnItemPossessed(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_ItemEntity)
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemPossessed)
		{
			action.OnActivate(pStorageOwner.GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnItemDropped(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_ItemEntity)
			return;

		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemDropped)
		{
			action.OnActivate(pStorageOwner.GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemCarrierChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		EventHandlerManagerComponent eventHandlerMgr;
		if (oldSlot)
		{
			eventHandlerMgr = EventHandlerManagerComponent.Cast(oldSlot.GetOwner().FindComponent(EventHandlerManagerComponent));
			if (eventHandlerMgr)
				eventHandlerMgr.RemoveScriptHandler("OnDestroyed", this, OnDestroyed);
		}
		
		if (newSlot)
		{
			eventHandlerMgr = EventHandlerManagerComponent.Cast(newSlot.GetOwner().FindComponent(EventHandlerManagerComponent));
			if (eventHandlerMgr)
				eventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to remove task item from destroyed entity inventory and drop it to the ground
	protected void OnDestroyed(IEntity destroyedEntity)
	{
		if (!destroyedEntity)
			return;
		
		if (!m_ItemEntity)
			return;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_ItemEntity.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		InventoryStorageSlot parentSlot = invComp.GetParentSlot();
		if (!parentSlot)
			return;
		
		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(destroyedEntity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		if (!inventoryComponent.Contains(m_ItemEntity))
			return;
		
		inventoryComponent.TryRemoveItemFromStorage(m_ItemEntity, parentSlot.GetStorage());
		m_ItemEntity.SetOrigin(destroyedEntity.GetOrigin());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDisconnected(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;
			
		OnDestroyed(player);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterPlayer(int playerID, IEntity playerEntity)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;

		SCR_InventoryStorageManagerComponent inventoryComponent = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;

		inventoryComponent.m_OnItemAddedInvoker.Insert(OnItemPossessed);
		inventoryComponent.m_OnItemRemovedInvoker.Insert(OnItemDropped);
			
		EventHandlerManagerComponent eventHandlerMgr = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerMgr)
			eventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!ValidateInputEntity(object, m_Getter, m_ItemEntity))
			return;

		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_ItemEntity.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		invComp.m_OnParentSlotChangedInvoker.Insert(OnItemCarrierChanged);
			
		array<int> aPlayerIDs = {};
		int iNrOfPlayersConnected = GetGame().GetPlayerManager().GetPlayers(aPlayerIDs); 
					
		foreach (int playerID : aPlayerIDs)
		{
			RegisterPlayer(playerID, null);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(RegisterPlayer);
		gameMode.GetOnPlayerDisconnected().Insert(OnDisconnected);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAddItemToInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCountNoInheritance> m_aPrefabFilter

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		entity.GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
		Math3D.AnglesToMatrix(angles, spawnParams.Transform);

		foreach (SCR_ScenarioFrameworkPrefabFilterCountNoInheritance prefabFilter : m_aPrefabFilter)
		{
			for (int i = 0; i < prefabFilter.m_iPrefabCount; i++)
			{
				Resource resource = Resource.Load(prefabFilter.m_sPrefabName);
				if (!resource)
					continue;

				IEntity item = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
				if (!item)
					continue;

				SCR_InvCallBackCheck pInvCallback = new SCR_InvCallBackCheck();
				pInvCallback.m_sNameMaster = entity.GetName();
				pInvCallback.m_sNameItem = item.GetName();

				inventoryComponent.TryInsertItem(item, EStoragePurpose.PURPOSE_ANY, pInvCallback)
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRemoveItemFromInventory : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		//Due to how invokers are setup, deletion is sometimes triggered before said item is actually in said inventory
		GetGame().GetCallqueue().CallLater(OnActivateCalledLater, 1000, false, object);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	void OnActivateCalledLater(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		array<IEntity> items = {};
		inventoryComponent.GetItems(items);
		if (items.IsEmpty())
			return;

		Resource resource;
		BaseContainer prefabContainer;
		array<IEntity> itemsToRemove = {};
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilter : m_aPrefabFilter)
		{
			resource = Resource.Load(prefabFilter.m_sSpecificPrefabName);
			if (!resource.IsValid())
				continue;

			prefabContainer = resource.GetResource().ToBaseContainer();
			if (!prefabContainer)
				continue;

			int count;
			int targetCount = prefabFilter.m_iPrefabCount;

			bool includeInheritance = prefabFilter.m_bIncludeChildren;

			foreach (IEntity item : items)
			{
				if (!item)
					continue;

				EntityPrefabData prefabData = item.GetPrefabData();
				if (!prefabData)
					continue;

				BaseContainer container = prefabData.GetPrefab();
				if (!container)
					continue;

				if (!includeInheritance)
				{
					if (container != prefabContainer)
						continue;

					itemsToRemove.Insert(item);
					count++;
					if (count == targetCount)
						break;
				}
				else
				{
					while (container)
					{
						if (container == prefabContainer)
						{
							itemsToRemove.Insert(item);
							count++;
							break;
						}

						container = container.GetAncestor();
					}
				}

				if (count == targetCount)
					break;
			}

			for (int i = itemsToRemove.Count() - 1; i >= 0; i--)
			{
				inventoryComponent.TryDeleteItem(itemsToRemove[i]);
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCountInventoryItemsAndExecuteAction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Which Prefabs and how many out of each will be added to the inventory of target entity")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	[Attribute(UIWidgets.Auto, desc: "If conditions from Prefab Filter are true, it will execute these actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsToExecute;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Inventory Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		array<IEntity> items = {};
		inventoryComponent.GetItems(items);
		if (items.IsEmpty())
			return;

		bool countCondition;
		Resource resource;
		BaseContainer prefabContainer;
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilter : m_aPrefabFilter)
		{
			resource = Resource.Load(prefabFilter.m_sSpecificPrefabName);
			if (!resource.IsValid())
				continue;

			prefabContainer = resource.GetResource().ToBaseContainer();
			if (!prefabContainer)
				continue;

			int count;
			int targetCount = prefabFilter.m_iPrefabCount;
			bool includeInheritance = prefabFilter.m_bIncludeChildren;
			countCondition = false;

			foreach (IEntity item : items)
			{
				if (!item)
					continue;

				EntityPrefabData prefabData = item.GetPrefabData();
				if (!prefabData)
					continue;

				BaseContainer container = prefabData.GetPrefab();
				if (!container)
					continue;

				if (!includeInheritance)
				{
					if (container != prefabContainer)
						continue;

					count++;
					if (count == targetCount)
					{
						countCondition = true;
						break;
					}
				}
				else
				{
					while (container)
					{
						if (container == prefabContainer)
						{
							count++;
							break;
						}

						container = container.GetAncestor();
					}
				}

				if (count == targetCount)
				{
					countCondition = true;
					break;
				}
			}

			//If just one prefab filter is false, we don't count any further and return
			if (!countCondition)
				return;
		}

		if (countCondition)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsToExecute)
			{
				action.OnActivate(object);
			}
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLockOrUnlockAllTargetVehiclesInTrigger : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Slot which spawns the trigger")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "true", desc: "If set to true, it will lock all vehicles, if set to false it will unlock all vehicles")]
	bool m_bLock;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[out] trigger
	//! \return
	bool CanActivateTriggerVariant(IEntity object, out SCR_CharacterTriggerEntity trigger)
	{
		trigger = SCR_CharacterTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] trigger
	void LockOrUnlockAllVehicles(SCR_CharacterTriggerEntity trigger = null)
	{
		if (!trigger)
			return;

		array<IEntity> entitesInside = {};
		trigger.GetEntitiesInside(entitesInside);
		if (entitesInside.IsEmpty())
			return;

		foreach (IEntity entity : entitesInside)
		{
			if (!Vehicle.Cast(entity))
				continue;

			SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(entity.FindComponent(SCR_VehicleSpawnProtectionComponent));
			if (m_bLock)
			{
				spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
				spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
				spawnProtectionComponent.SetVehicleOwner(-2);
			}
			else
			{
				spawnProtectionComponent.SetProtectOnlyDriverSeat(true);
				spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_CannotEnterVehicle-UC");
				spawnProtectionComponent.ReleaseProtection();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_CharacterTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			LockOrUnlockAllVehicles(trigger);
			return;
		}

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

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		trigger = SCR_CharacterTriggerEntity.Cast(entityFrom);
		if (trigger)
		{
			LockOrUnlockAllVehicles(trigger);
			return;
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionLockOrUnlockVehicle : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(defvalue: "true", desc: "If set to true, it will lock the vehicle, if set to false it will unlock the vehicle")]
	bool m_bLock;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		if (!Vehicle.Cast(entity))
			return;

		SCR_VehicleSpawnProtectionComponent spawnProtectionComponent = SCR_VehicleSpawnProtectionComponent.Cast(entity.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (!spawnProtectionComponent)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Spawn Protection Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Spawn Protection Component Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		if (m_bLock)
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(false);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_BuildBlocked-UC");
			spawnProtectionComponent.SetVehicleOwner(-2);
		}
		else
		{
			spawnProtectionComponent.SetProtectOnlyDriverSeat(true);
			spawnProtectionComponent.SetReasonText("#AR-Campaign_Action_CannotEnterVehicle-UC");
			spawnProtectionComponent.ReleaseProtection();
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionFailTaskIfVehiclesInTriggerDestroyed : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Trigger Getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Target Layer Task. If this is attached to the Layer Task you intend to work with, you can leave it empty")]
	string m_sTargetLayerTask;

	[Attribute(defvalue: "false", desc: "If set to true, it will fail the task only when player caused the destruction")]
	bool m_bCausedByPlayer;

	bool m_bAlreadyDestroyed;
	SCR_ScenarioFrameworkTask m_Task;
	ref array<IEntity> m_aTargetEntities = {};

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[out] trigger
	//! \return
	bool CanActivateTriggerVariant(IEntity object, out SCR_CharacterTriggerEntity trigger)
	{
		trigger = SCR_CharacterTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[in] trigger
	void AddListener(IEntity object = null, SCR_CharacterTriggerEntity trigger = null)
	{
		if (!object)
			return;

		if (!trigger)
			return;

		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layerTask)
			return;

		m_Task = layerTask.GetTask();
		if (!m_Task)
			return;

		array<IEntity> entitesInside = {};
		trigger.GetEntitiesInside(entitesInside);
		if (entitesInside.IsEmpty())
			return;

		foreach (IEntity entity : entitesInside)
		{
			if (!Vehicle.Cast(entity))
				continue;

			SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
			if (objectDmgManager)
			{
				m_aTargetEntities.Insert(entity);
				objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || m_bAlreadyDestroyed)
			return;

		if (m_bCausedByPlayer)
		{
			bool destroyedByPlayer;
			foreach (IEntity entity : m_aTargetEntities)
			{
				SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent));
				if (!objectDmgManager)
					continue;

				if (objectDmgManager.GetState() == EDamageState.DESTROYED)
				{

					Instigator instigator = objectDmgManager.GetInstigator();
					if (!instigator)
						continue;

					InstigatorType instigatorType = instigator.GetInstigatorType();
					if (instigatorType && instigatorType == InstigatorType.INSTIGATOR_PLAYER)
					{
						destroyedByPlayer = true;
						break;
					}
				}
			}

			if (!destroyedByPlayer)
				return;
		}

		SCR_BaseTaskSupportEntity supportEntity = SCR_BaseTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity));
		if (!supportEntity)
			return;

		m_bAlreadyDestroyed = true;
		if (m_Task)
			supportEntity.FailTask(m_Task);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_sTargetLayerTask.IsEmpty())
		{
			IEntity entity = GetGame().GetWorld().FindEntityByName(m_sTargetLayerTask);
			if (entity)
				object = entity;
		}

		SCR_CharacterTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			AddListener(object, trigger);
			return;
		}

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

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;

		}

		trigger = SCR_CharacterTriggerEntity.Cast(entityFrom);
		if (trigger)
			AddListener(object, trigger);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnCompartmentEnteredOrLeft : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If true, we execute actions On Compartmented Entered. Otherwise On Compartment Left")]
	bool m_bEnteredOrLeft;

	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "(Optional) If used, it will get executed only when specific entity enters the compartment")]
	ref SCR_ScenarioFrameworkGet m_OccupantGetter;

	[Attribute(desc: "(Optional) If used, it will get executed only when specific compartment slots are entered")]
	ref array<int> m_aSlotIDs;

	[Attribute(desc: "Actions that will be executed on compartment entered", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	IEntity m_OccupantEntity;

	//------------------------------------------------------------------------------------------------
	//! \param[in] vehicle
	//! \param[in] mgr
	//! \param[in] occupant
	//! \param[in] managerId
	//! \param[in] slotID
	void OnCompartmentEnteredOrLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (m_OccupantEntity && occupant != m_OccupantEntity)
			return;

		if (m_aSlotIDs && !m_aSlotIDs.IsEmpty() && !m_aSlotIDs.Contains(slotID))
			return;

		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		if (m_OccupantGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> occupantWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_OccupantGetter.Get());
			if (!occupantWrapper)
				return;

			m_OccupantEntity = IEntity.Cast(occupantWrapper.GetValue());
			if (!m_OccupantEntity)
				return;
		}

		if (Vehicle.Cast(entity))
		{
			EventHandlerManagerComponent ehManager = EventHandlerManagerComponent.Cast(entity.FindComponent(EventHandlerManagerComponent));
			if (!ehManager)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Event Handler Manager Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Event Handler Manager Component not found for Action %1.", this), LogLevel.ERROR);

				return;
			}

			if (m_bEnteredOrLeft)
				ehManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEnteredOrLeft, true, false);
			else
				ehManager.RegisterScriptHandler("OnCompartmentLeft", this, OnCompartmentEnteredOrLeft, true, false);
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnEngineStartedOrStop : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If true, we execute actions On Engine Started. Otherwise On Engine Stop")]
	bool m_bStartedOrStop;

	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions that will be executed on one of these circumstances", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	void OnEngineStartedOrStop()
	{
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		VehicleControllerComponent_SA vehicleController = VehicleControllerComponent_SA.Cast(entity.FindComponent(VehicleControllerComponent_SA));
		if (!vehicleController)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Vehicle Controller Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Vehicle Controller Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		if (m_bStartedOrStop)
				vehicleController.GetOnEngineStart().Insert(OnEngineStartedOrStop);
			else
				vehicleController.GetOnEngineStop().Insert(OnEngineStartedOrStop);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionToggleLights : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate lights with (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: ELightType.Head.ToString(), UIWidgets.ComboBox, desc: "Which lights to be toggled", "", ParamEnumArray.FromEnum(ELightType))]
	ELightType m_eLightType;

	[Attribute(defvalue: "1", desc: "If true, light will be turned on. Otherwise it will turn it off.")]
	bool m_bTurnedOn;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		BaseLightManagerComponent lightManager = BaseLightManagerComponent.Cast(entity.FindComponent(BaseLightManagerComponent));
		if (!lightManager)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Light Manager Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Light Manager Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		lightManager.SetLightsState(m_eLightType, m_bTurnedOn);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionToggleEngine : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to turn on/off the engine (Optional if action is attached on Slot that spawns target entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "1", desc: "If true, engine will be turned on. Otherwise it will turn it off.")]
	bool m_bTurnedOn;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		VehicleWheeledSimulation_SA vehicleSimulation = VehicleWheeledSimulation_SA.Cast(entity.FindComponent(VehicleWheeledSimulation_SA));
		if (!vehicleSimulation)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Vehicle Wheeled Simulation Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Vehicle Wheeled Simulation Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		if (m_bTurnedOn)
			vehicleSimulation.EngineStart();
		else
			vehicleSimulation.EngineStop();
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDamageWheel : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate fuel (Optional if action is attached on Slot that spawns target entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "", desc: "Name of Slots that are defined on the SlotManagerComponent on target vehicle")]
	ref array<string> m_aSlotNamesOnSlotManager;

	[Attribute(defvalue: "100", desc: "Health Percentage to be set for target wheels", UIWidgets.Graph, "0 100 1")]
	int m_iHealthPercentage;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
		if (!slotManager)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Slot Manager Component not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Slot Manager Component not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		foreach (string slotName : m_aSlotNamesOnSlotManager)
		{
			EntitySlotInfo slotInfo = slotManager.GetSlotByName(slotName);
			if (!slotInfo)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Name of the slot %1 not found on target entity for Action %2 attached on %3.", slotName, this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Name of the slot %1 not found on target entity for Action %1.", slotName, this), LogLevel.ERROR);

				return;
			}

			IEntity wheelEntity = slotInfo.GetAttachedEntity();
			if (!wheelEntity)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Retrieving target wheel entity failed for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Retrieving target wheel entity failed for Action %1.", this), LogLevel.ERROR);

				return;
			}

			SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(wheelEntity);
			if (!damageManager)
			{
				if (object)
						Print(string.Format("ScenarioFramework Action: Retrieving Damage Manager of target entity failed for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
					else
						Print(string.Format("ScenarioFramework Action: Retrieving Damage Manager of target entity failed for Action %1.", this), LogLevel.ERROR);

				return;
			}

			damageManager.SetHealthScaled(m_iHealthPercentage * 0.01);
		}
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetFuelPercentage : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to manipulate fuel (Optional if action is attached on Slot that spawns target entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "75", desc: "Percentage of a fuel to be set.", UIWidgets.Graph, "0 100 1")]
	int m_iFuelPercentage;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		array<SCR_FuelManagerComponent> fuelManagers = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(entity, fuelManagers);
		if (fuelManagers.IsEmpty())
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: No Fuel Managers found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: No Fuel Managers found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		SCR_FuelManagerComponent.SetTotalFuelPercentageOfFuelManagers(fuelManagers, m_iFuelPercentage * 0.01, 0, SCR_EFuelNodeTypeFlag.IS_FUEL_STORAGE);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTriggerActivationPresence : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger")]
	TA_EActivationPresence	m_eActivationPresence;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[out] trigger
	//! \return
	bool CanActivateTriggerVariant(IEntity object, out SCR_CharacterTriggerEntity trigger)
	{
		trigger = SCR_CharacterTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_CharacterTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			trigger.SetActivationPresence(m_eActivationPresence);
			return;
		}

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

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(layer);
		if (area)
		{
			trigger = SCR_CharacterTriggerEntity.Cast(area.GetTrigger());
		}
		else
		{
			IEntity entity = layer.GetSpawnedEntity();
			if (!BaseGameTriggerEntity.Cast(entity))
			{
				Print(string.Format("ScenarioFramework Action: SlotTrigger - The selected prefab is not trigger!"), LogLevel.ERROR);
				return;
			}
			trigger = SCR_CharacterTriggerEntity.Cast(entity);
		}

		trigger.SetActivationPresence(m_eActivationPresence);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeLayerActivationType : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

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

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		layer.SetActivationType(m_eActivationType);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRestoreLayerToDefault : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Layer to be restored to default (Optional if action is attached on layer that is supposed to be restored to default)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(defvalue: "true", desc: "If checked, it will also restore child layers to default state as well.")]
	bool m_bIncludeChildren;
	
	[Attribute(desc: "If checked, it will reinit the layer after the restoration")]
	bool m_bReinitAfterRestoration;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		layer.RestoreToDefault(m_bIncludeChildren, m_bReinitAfterRestoration);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPrepareAreaFromDynamicDespawn : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "If set to false, area will be despawned")]
	bool m_bStaySpawned;

	[Attribute(defvalue: "750", desc: "How close at least one observer camera must be in order to trigger dynamic spawn/despawn")]
	int m_iDynamicDespawnRange;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Area for Action %1.", this), LogLevel.ERROR);
			return;
		}

		area.SetDynamicDespawnRange(m_iDynamicDespawnRange);
		manager.PrepareAreaSpecificDynamicDespawn(area, m_bStaySpawned);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRemoveAreaFromDynamicDespawn : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "If set to false, area will be despawned")]
	bool m_bStaySpawned;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Area for Action %1.", this), LogLevel.ERROR);
			return;
		}

		manager.RemoveAreaSpecificDynamicDespawn(area, m_bStaySpawned);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySound : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Sound to play.")]
	string 			m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		GetGame().GetCallqueue().CallLater(manager.PlaySoundOnEntity, 2000, false, null, m_sSound);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySoundOnEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play the sound on (Optional if action is attached on Slot that spawns target entity")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Sound to play.")]
	string 			m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		GetGame().GetCallqueue().CallLater(manager.PlaySoundOnEntity, 2000, false, entity, m_sSound);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionIntroVoicelineBasedOnTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Sound to play.")]
	string m_sSound;

	[Attribute(desc: "(Optional) If getter is provided, sound will come from the provided entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	ref array<int> m_aAffectedPlayers = {};

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerID = playerManager.GetPlayerIdFromControlledEntity(object);
		if (m_aAffectedPlayers.Contains(playerID))
			return;

		m_aAffectedPlayers.Insert(playerID);

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		EntityID entityID;
		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (entityWrapper)
			{
				IEntity entity = IEntity.Cast(entityWrapper.GetValue());
				if (entity)
					entityID = entity.GetID();
			}
		}

		manager.PlayIntroVoiceline(playerID, m_sSound, entityID);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionProcessVoicelineEnumAndString : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the enum to work with")]
	string m_sTargetEnum;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (!taskManager)
			return;

		array<SCR_BaseTask> tasks = {};
		taskManager.GetTasks(tasks);

		typename targetEnum = m_sTargetEnum.ToType();
		foreach (SCR_BaseTask task : tasks)
		{
			SCR_ScenarioFrameworkTask frameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
			if (frameworkTask)
				manager.ProcessVoicelineEnumAndString(targetEnum, frameworkTask.m_sTaskIntroVoiceline)
		}
	}
}

//! Changes Task State (For example you can use it to finish or fail task)
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTaskState : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute("1", UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum(SCR_TaskState))]
	SCR_TaskState m_eTaskState;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

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

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		SCR_ScenarioFrameworkTaskSupportEntity supportEntity = task.GetSupportEntity();
		if (!supportEntity)
		{
			if (object)
					Print(string.Format("ScenarioFramework Action: Task support entity not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Task not found for Action %1.", this), LogLevel.ERROR);

			return;
		}

		//Task system is a mess, so this is why we have to tackle it this abysmal way:
		if (m_eTaskState == SCR_TaskState.FINISHED)
			supportEntity.FinishTask(task);
		else if (m_eTaskState == SCR_TaskState.REMOVED)
			supportEntity.FailTask(task);
		else if (m_eTaskState == SCR_TaskState.CANCELLED)
			supportEntity.CancelTask(task.GetTaskID());
		else
			task.SetState(m_eTaskState);
		
		SCR_ScenarioFrameworkLayerTask layerTask = task.GetLayerTask();
		if (!layerTask)
			return;
		
		layerTask.SetLayerTaskState(m_eTaskState);
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionResetCounter : SCR_ScenarioFrameworkActionBase
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!object)
		{
			Print(string.Format("ScenarioFramework Action: Object not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLogicCounter counter = SCR_ScenarioFrameworkLogicCounter.Cast(object.FindComponent(SCR_ScenarioFrameworkLogicCounter));
		if (!counter)
		{
			Print(string.Format("ScenarioFramework Action: Counter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
			
		counter.Reset();
	}
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionExecuteFunction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Object the method will be called")]
	ref SCR_ScenarioFrameworkGet		m_ObjectToCallTheMethodFrom;

	[Attribute(desc: "Method to call")]
	string			m_sMethodToCall;

	[Attribute(desc: "Parameter1 to pass (string only)")]
	string		m_sParameter;

	[Attribute(desc: "Parameter2 to pass (string only)")]
	string		m_sParameter2;

	[Attribute(desc: "Parameter3 to pass (string only)")]
	string		m_sParameter3;

	[Attribute(desc: "Parameter4 to pass (string only)")]
	string		m_sParameter4;

	[Attribute(desc: "Parameter5 to pass (string only)")]
	string		m_sParameter5;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_ObjectToCallTheMethodFrom.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkArea));
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerBase));
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (layer)
			GetGame().GetCallqueue().CallByName(layer, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else if (layerBase)
			GetGame().GetCallqueue().CallByName(layerBase, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else if (area)
			GetGame().GetCallqueue().CallByName(area, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else
			GetGame().GetCallqueue().CallByName(entityWrapper.GetValue(), m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
	}
}
