
//------------------------------------------------------------------------------------------------
class SCR_ContainerActionTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkAction", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}
	
	protected void SplitStringByUpperCase(string sInput, out string sOutput)
	{
		sOutput = "";
		int m;
		for (int i = 0; i < sInput.Length(); i++)
		{
			m = sInput.ToAscii(i);
			if (m < 97)	// lower case 
				sOutput += " ";
			sOutput += m.AsciiToString();
		}
	}
}

//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//TODO: make this a generic action which can be used anywhere anytime (i.e. on task finished, etc)
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBase
{	
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Graph, params: "-1 10000 1", desc: "How many times this action can be performed if this gets triggered? Value -1 for infinity")];
	protected int					m_iMaxNumberOfActivations;
	
	protected IEntity				m_Entity;
	protected int					m_iNumberOfActivations;
	
	//------------------------------------------------------------------------------------------------
	void Init(IEntity entity)
	{
		if (!SCR_BaseTriggerEntity.Cast(entity))
			return;
		
		m_Entity = entity;
		ScriptInvoker pOnActivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnActivate();
		if (pOnActivateInvoker)
			pOnActivateInvoker.Insert(OnActivate);
		
		ScriptInvoker pOnDeactivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnDeactivate();
		if (pOnDeactivateInvoker)
			pOnDeactivateInvoker.Insert(OnActivate);		//registering OnDeactivate to OnActivate - we need both changes 
	}	
	
	//------------------------------------------------------------------------------------------------
	bool CanActivate()
	{
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
			return false;
		
		m_iNumberOfActivations++;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate(IEntity object)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType)
	{ 
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;
		
		foreach (string sObjectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!object)
			{
				PrintFormat("ScenarioFramework: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName, LogLevel.ERROR);
				continue;
			}
			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				PrintFormat("ScenarioFramework: Can't spawn object - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", sObjectName, LogLevel.ERROR);
				continue;
			}
			layer.Init(null, eActivationType, false);
		}
	}
}	

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionIncrementCounter : SCR_ScenarioFrameworkActionBase
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Counter to increment")];
	protected string				m_sCounterName;
	
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
			return;
		
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

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjects : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "These objects will spawn once the trigger becomes active.")];
	protected ref array<string> 	m_aNameOfObjectsToSpawnOnActivation;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SpawnObjects(m_aNameOfObjectsToSpawnOnActivation, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityPosition : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the entity to be teleported")];
	protected ref SCR_ScenarioFrameworkGet m_EntityGetter;
	
	[Attribute(defvalue: "0 0 0", desc: "Position that the entity will be teleported to")];
	protected vector 	m_vDestination;
	
	[Attribute(desc: "Name of the entity that above selected entity will be teleported to (optional)")];
	protected ref SCR_ScenarioFrameworkGet m_DestinationEntityGetter;
	
	[Attribute(defvalue: "0 0 0", desc: "Position that will be used in relation to the entity for the position to teleport to (optional)")];
	protected vector 	m_vDestinationEntityRelativePosition;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_EntityGetter.Get());
		if (!entityWrapper)
			return;
			
		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;
		
		if (!m_DestinationEntityGetter)
		{
			entity.SetOrigin(m_vDestination);
			return;
		}	
		
		SCR_ScenarioFrameworkParam<IEntity> destinationEntityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_DestinationEntityGetter.Get());
		if (!entityWrapper)
			return;
			
		IEntity destinationEntity = IEntity.Cast(destinationEntityWrapper.GetValue());
		if (!destinationEntity)
			return;

		entity.SetOrigin(destinationEntity.GetOrigin() + m_vDestinationEntityRelativePosition);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDeleteEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be deleted")];
	protected ref SCR_ScenarioFrameworkGet m_Getter;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;
			
		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;
			
		delete entity;	
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionEndMission : SCR_ScenarioFrameworkActionBase
{
	[Attribute(UIWidgets.CheckBox, desc: "If true, it will override any previously set game over type with selected one down bellow")];
	protected bool		m_bOverrideGameOverType;
	
	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))];
	protected EGameOverTypes			m_eOverriddenGameOverType;
	
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

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionWaitAndExecute : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "How long to wait before executing action")];
	protected int							m_iDelayInSeconds;
	
	[Attribute(defvalue: "1", desc: "Which actions will be executed once set time passes", UIWidgets.Auto)];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;
	
	//------------------------------------------------------------------------------------------------
	void ExecuteActions(IEntity object)
	{
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
			
		//Used to delay the call as it is the feature of this action
		GetGame().GetCallqueue().CallLater(ExecuteActions, m_iDelayInSeconds * 1000, false, object);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCompareCounterAndExecute : SCR_ScenarioFrameworkActionBase
{

	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator			m_eComparisonOperator;
	
	[Attribute(desc: "Value")];
	protected int							m_iValue;

	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Counter to increment")];
	protected string						m_sCounterName;
	
	[Attribute(defvalue: "1", desc: "What to do once counter is reached", UIWidgets.Auto, category: "OnActivate")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sCounterName);
		if (!entity)
			return;
		
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

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetMissionEndScreen : SCR_ScenarioFrameworkActionBase
{
	
	[Attribute("1", UIWidgets.ComboBox, "Game Over Type", "", ParamEnumArray.FromEnum(EGameOverTypes))];
	protected EGameOverTypes			m_eGameOverType;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		manager.SetMissionEndScreen(m_eGameOverType);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowHint : SCR_ScenarioFrameworkActionBase
{
	[Attribute()];
	protected string		m_sTitle;
	
	[Attribute()];
	protected string		m_sText;
	
		
	[Attribute(defvalue: "15")];
	protected int			m_iTimeout;
	
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_HintManagerComponent.ShowCustomHint(m_sText, m_sTitle, m_iTimeout);
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnClosestObjectFromList : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;
	
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "The closest one from the list will be spawned")];
	protected ref array<string> 	m_aListOfObjects;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!m_Getter)
		{
			Print("ScenarioFramework: The object the distance is calculated from is missing!", LogLevel.ERROR);
			return;
		}
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		IEntity entityInList;
		SCR_ScenarioFrameworkLayerBase selectedLayer;
		if (!entityFrom)
		{
			Print("ScenarioFramework: Action: Getter returned null object. Random object spawned instead.", LogLevel.WARNING);
			array<string> aRandomObjectToSpawn = {};
			aRandomObjectToSpawn.Insert(m_aListOfObjects[m_aListOfObjects.GetRandomIndex()]);
			
			entityInList = GetGame().GetWorld().FindEntityByName(aRandomObjectToSpawn[0]);
			if (!entityInList)
			{
				PrintFormat("ScenarioFramework: Object %1 doesn't exist", aRandomObjectToSpawn[0], LogLevel.ERROR);
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
				PrintFormat("ScenarioFramework: Object %1 doesn't exist", sObjectName, LogLevel.ERROR);
				continue;
			}
			
			float fActualDistance = Math.AbsFloat(vector.Distance(entityFrom.GetOrigin(), entityInList.GetOrigin()));
		
			if ( fActualDistance < fDistance)
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
			selectedLayer.GetParentArea().SetAreaSelected(true);
			SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(selectedLayer);
			if (layerTask)
				selectedLayer.GetParentArea().SetLayerTask(layerTask);
			
			selectedLayer.Init(null, SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION, false);
		}
		else
		{
			PrintFormat("ScenarioFramework: Can't spawn slot %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", closestEntity.GetName(), LogLevel.ERROR);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTriggerActivationPresence : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;
	
	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(TA_EActivationPresence), category: "Trigger")]
	protected TA_EActivationPresence	m_eActivationPresence;
	
	//------------------------------------------------------------------------------------------------
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
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return;
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

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
				Print("ScenarioFramework: SlotTrigger - The selected prefab is not trigger!", LogLevel.ERROR);
				return;
			}
			trigger = SCR_CharacterTriggerEntity.Cast(entity);
		}

		trigger.SetActivationPresence(m_eActivationPresence);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySound : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Sound to play.")]		
	protected string 			m_sSound;
			
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

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySoundOnEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play the sound on")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;
	
	[Attribute(desc: "Sound to play.")]		
	protected string 			m_sSound;
			
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		GetGame().GetCallqueue().CallLater(manager.PlaySoundOnEntity, 2000, false, IEntity.Cast(entityWrapper.GetValue()), m_sSound);		
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionResetCounter : SCR_ScenarioFrameworkActionBase
{	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!object)
			return;
			
		SCR_ScenarioFrameworkLogicCounter counter = SCR_ScenarioFrameworkLogicCounter.Cast(object.FindComponent(SCR_ScenarioFrameworkLogicCounter));
		if (counter)
			counter.Reset();
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionExecuteFunction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Object the method will be called")];
	protected ref SCR_ScenarioFrameworkGet		m_ObjectToCallTheMethodFrom;
	
	[Attribute(desc: "Method to call")];
	protected string			m_sMethodToCall;
	
	[Attribute(desc: "Parameter to pass (string only)")];
	protected string		m_sParameter;

			
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_ObjectToCallTheMethodFrom.Get());
		if (!entityWrapper)
			return;
		
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkArea));
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerBase));
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (layer)
			GetGame().GetCallqueue().CallByName(layer, m_sMethodToCall, m_sParameter);
		else if (layerBase)
			GetGame().GetCallqueue().CallByName(layerBase, m_sMethodToCall, m_sParameter);
		else if (area)
			GetGame().GetCallqueue().CallByName(area, m_sMethodToCall, m_sParameter);
		else
			GetGame().GetCallqueue().CallByName(entityWrapper.GetValue(), m_sMethodToCall, m_sParameter);
	}
}

