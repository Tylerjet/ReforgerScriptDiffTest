//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BaseTaskSupportClass
{
	protected typename m_TypeName;
	
	[Attribute("{1844F5647E6D772A}UI/layouts/Tasks/TaskListEntry.layout")]
	protected ResourceName m_TaskDescriptionWidgetResource;
	
	[Attribute()]
	protected ResourceName m_TaskPrefab;
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask CreateTask()
	{
		return SCR_BaseTask.Cast(GetGame().SpawnEntityPrefab(GetTaskPrefab()));
	}
	
	//------------------------------------------------------------------------------------------------
	void Initialize()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//Called when task list is opened
	void OnTaskListOpen(SCR_UITaskManagerComponent uiTaskManagerComponent)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_BaseTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetTaskDescriptionWidgetResource()
	{
		return m_TaskDescriptionWidgetResource;
	}
	
	//------------------------------------------------------------------------------------------------
	Resource GetTaskPrefab()
	{
		if (m_TaskPrefab.IsEmpty())
			return null;
		
		return Resource.Load(m_TaskPrefab);
	}
	
	//------------------------------------------------------------------------------------------------
	typename GetTypename()
	{
		return m_TypeName;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterContextualMenuCallbacks()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		RegisterContextualMenuCallbacks();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BaseTaskSupportClass()
	{
		SCR_MapEntity.GetMapInstance().GetOnMapOpen().Insert(OnMapOpen);
		m_TypeName = SCR_BaseTask;
	}
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignTaskSupportClass : SCR_BaseTaskSupportClass
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_CampaignTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignTaskSupportClass()
	{
		m_TypeName = SCR_CampaignTask;
	}
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_EvacuateTaskSupportClass : SCR_RequestedTaskSupportClass
{
	static const string NOWHERE_TO_RUN = "#AR-CampaignTasks_RequestImpossiblePosition-UC";
	static const string NO_SIGNAL = "#AR-CampaignTasks_RequestImpossibleSignal-UC";
	
	[Attribute("1000", desc: "Minimum distance of a base from the place this task is requested from. (in m)")]
	protected float m_fMinDistanceFromStart;
	
	[Attribute("50", desc: "Maximum distance of the rescuer from the evacuated player, once a rescue base is reached. (in m)")]
	protected float m_fMaxDistanceFromRequester;
	
	//------------------------------------------------------------------------------------------------
	override bool CanRequest()
	{
		return (super.CanRequest() && HasValidBase());
	}
	
	//------------------------------------------------------------------------------------------------
	override void Request()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.GetInstance();
		if (!ctxMenu)
			return;
		
		vector position = ctxMenu.GetContextMenuWorldPosition();
		
		IEntity requesterEntity = playerController.GetMainEntity();
		if (!requesterEntity)
			return;
		
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
			return;
		
		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
		if (!requesterFaction)
			return;
		
		vector requesterOrigin = requesterEntity.GetOrigin();
		array<SCR_CampaignBase> bases = new array<SCR_CampaignBase>();
		
		SCR_CampaignBaseManager.GetInstance().GetFilteredBases(SCR_EvacuateTask.BASES_FILTER, bases);
		
		bool foundValidBase = false;
		for (int i = 0; i < bases.Count(); i++)
		{
			if (requesterFaction != bases[i].GetOwningFaction())
				continue;
			
			float baseToStartDistance = vector.Distance(bases[i].GetOrigin(), requesterOrigin);
			
			if (baseToStartDistance > SCR_EvacuateTask.GetMinDistanceFromStart())
			{
				foundValidBase = true;
				break;
			}
		}
		
		if (!foundValidBase)
		{
			SCR_PopUpNotification.GetInstance().PopupMsg(NOWHERE_TO_RUN);
			return;
		}
		
		// Replace this with better solution
		//--------------------------------------
		//--------------------------------------
		if (!SCR_UIRequestEvacTaskComponent.IsInRange(requesterFaction, requesterEntity))
		{
			SCR_PopUpNotification.GetInstance().PopupMsg(NO_SIGNAL);
			return;
		}
		//--------------------------------------
		//--------------------------------------
		
		// Find task network component to send RPC to server
		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
		if (!taskNetworkComponent)
			return;
		
		taskNetworkComponent.RequestEvacuation(position);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMinDistanceFromStart()
	{
		return m_fMinDistanceFromStart;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxDistanceFromRequester()
	{
		return m_fMaxDistanceFromRequester;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasValidBase()
	{
		IEntity requesterEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!requesterEntity)
			return false;
		
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
			return false;
		
		Faction requesterFaction = factionAffiliationComponent.GetAffiliatedFaction();
		if (!requesterFaction)
			return false;
		
		bool foundValidBase = false;
		
		vector requesterOrigin = requesterEntity.GetOrigin();
		array<SCR_CampaignBase> bases = new array<SCR_CampaignBase>();
		
		SCR_CampaignBaseManager.GetInstance().GetFilteredBases(SCR_EvacuateTask.BASES_FILTER, bases);
		
		for (int i = 0; i < bases.Count(); i++)
		{
			if (requesterFaction != bases[i].GetOwningFaction())
				continue;
			
			float baseToStartDistance = vector.Distance(bases[i].GetOrigin(), requesterOrigin);
			
			if (baseToStartDistance > SCR_EvacuateTask.GetMinDistanceFromStart())
			{
				foundValidBase = true;
				break;
			}
		}
		
		return foundValidBase;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_EvacuateTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EvacuateTaskSupportClass()
	{
		m_TypeName = SCR_EvacuateTask;
	}
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RefuelTaskSupportClass : SCR_RequestedTaskSupportClass
{
	[Attribute("0.5", params: "0 1 0.01", desc: "Player can request refuel once fuel in the tank is below this value. (x Max fuel in tank)")]
	protected float m_fFuelLimit;
	
	//------------------------------------------------------------------------------------------------
	override bool CanRequest()
	{
		if (!super.CanRequest())
			return false;
		
		SCR_BaseTaskExecutor localTaskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (!localTaskExecutor)
			return false;
		
		if (!SCR_RefuelTask.CheckRefuelRequestConditions(localTaskExecutor))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Request()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.GetInstance();
		if (!ctxMenu)
			return;
		
		vector position = ctxMenu.GetContextMenuWorldPosition();
		
		// Find task network component to send RPC to server
		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
		if (!taskNetworkComponent)
			return;
		
		if (!SCR_UIRequestEvacTaskComponent.HasSignal(playerController))
			return;
		
		taskNetworkComponent.RequestRefuel(position);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFuelLimit()
	{
		return m_fFuelLimit;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_RefuelTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RefuelTaskSupportClass()
	{
		m_TypeName = SCR_RefuelTask;
	}
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignBuildingTaskSupportClass : SCR_BaseTaskSupportClass
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_CampaignBuildingTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignBuildingTaskSupportClass()
	{
		m_TypeName = SCR_CampaignBuildingTask;
	}
}
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_RequestedTaskSupportClass : SCR_BaseTaskSupportClass
{
	[Attribute("{75C912A1C89BE6C2}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonText.layout")]
	protected ResourceName m_UIRequestButtonResource;
	
	[Attribute("0", UIWidgets.ComboBox, "Request type", "", ParamEnumArray.FromEnum(SCR_EUIRequestType))]
	protected SCR_EUIRequestType m_eRequestType;
	
	[Attribute("", UIWidgets.LocaleEditBox, "Request button text.")]
	protected LocalizedString m_sRequestButtonText;
	
	//------------------------------------------------------------------------------------------------
	vector GetRequestPosition()
	{
		// By default we take map to world position - from context menu
		
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanRequest()
	{
		return (!GetTaskManager().GetLocallyRequestedTask());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRequestButtonText(notnull TextWidget textWidget)
	{
		textWidget.SetTextFormat(m_sRequestButtonText);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnContextualEntryShow(SCR_MapContextualMenuEntry entry, Widget button)
	{
		TextWidget textWidget = TextWidget.Cast(button.FindAnyWidget("Text"));
		if (!textWidget)
			return;
		
		SetRequestButtonText(textWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	void Request()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RegisterContextualMenuCallbacks()
	{
		if (m_sRequestButtonText.IsEmpty())
			return;
		
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapContextualMenuUI));
		if (!ctxMenu)
			return;
		
		SCR_MapContextualMenuRequestedTaskEntry entry = new SCR_MapContextualMenuRequestedTaskEntry(m_sRequestButtonText);
		if (!entry)
			return;
		
		ctxMenu.ContextInsertDynamic(entry);
		
		entry.SetSupportClass(this);
		entry.m_OnClick.Insert(Request);
		entry.m_OnShow.Insert(OnContextualEntryShow);
	}
	
	//------------------------------------------------------------------------------------------------
	Widget CreateButtonAndSetPadding(Widget parentWidget, inout array<Widget> widgets, float left = 4, float top = 4, float right = 4, float bottom = 4)
	{
		Widget requestButtonLayout = GetGame().GetWorkspace().CreateWidgets(m_UIRequestButtonResource, parentWidget);
		if (!requestButtonLayout)
			return null;
		
		ButtonWidget requestButton = ButtonWidget.Cast(requestButtonLayout.FindAnyWidget("Button"));
		if (requestButton)
		{
			SCR_UIRequestEvacTaskComponent requestTaskComponent = SCR_UIRequestEvacTaskComponent.Cast(requestButton.FindHandler(SCR_UIRequestEvacTaskComponent));
			if (requestTaskComponent)
				requestTaskComponent.SetRequestType(m_eRequestType);
		}
		
		VerticalLayoutSlot.SetPadding(requestButtonLayout, left, top, right, bottom);
		
		TextWidget textWidget = TextWidget.Cast(requestButtonLayout.FindAnyWidget("Text"));
		if (textWidget)
			SetRequestButtonText(textWidget);
		
		widgets.Insert(requestButtonLayout);
		
		return requestButtonLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_RequestedTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RequestedTaskSupportClass()
	{
		m_TypeName = SCR_RequestedTask;
	}
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CampaignDefendTaskSupportClass : SCR_RequestedTaskSupportClass
{
	[Attribute("300", desc: "From how far can an ally request reinforcements. (in m)")]
	protected float m_fMinAllyDistance;
	
	[Attribute("400", desc: "How far an ally can be to receive rewards for this task. (in m)")]
	protected float m_fMaxAllyDistance;
	
	[Attribute("300", desc: "How far an enemy has to be, for an ally to be able to request reinforcements. (in m)")]
	protected float m_fMinEnemyDistance;
	
	[Attribute("400", desc: "How far an enemy has to be, to be outside of the radius of an existing defend task. (in m)")]
	protected float m_fMaxEnemyDistance;
	
	// This only holds living characters, no need to check whether they are alive!!
	protected ref map<SCR_CampaignFaction, ref array<SCR_ChimeraCharacter>> m_mFactionSortedCharacters = new map<SCR_CampaignFaction, ref array<SCR_ChimeraCharacter>>();
	
	ref ScriptInvoker m_OnCharacterDeath = new ScriptInvoker();
	
	protected SCR_CampaignBase m_ClosestBase;
	
	//------------------------------------------------------------------------------------------------
	override bool CanRequest()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return false;
		
		if (!SCR_CampaignDefendTask.CheckDefendRequestConditions(playerController, m_ClosestBase))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Request()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		SCR_UIRequestEvacTaskComponent.RequestReinforcements(playerController);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetRequestButtonText(notnull TextWidget textWidget)
	{
		if (!m_ClosestBase)
			return;
		
		textWidget.SetTextFormat(m_sRequestButtonText, m_ClosestBase.GetBaseName());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnControllableCreated(IEntity controllable)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controllable);
		if (!character)
			return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
		if (!faction)
			return;
		
		array<SCR_ChimeraCharacter> factionCharacters = GetCharactersByFaction(faction);
		if (!factionCharacters)
		{
			factionCharacters = new array<SCR_ChimeraCharacter>();
			m_mFactionSortedCharacters.Insert(faction, factionCharacters);
		}
		
		factionCharacters.Insert(character);
		
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(character.FindComponent(SCR_CharacterControllerComponent));
		if (!characterControllerComponent)
			return;
		
		if (characterControllerComponent.m_OnPlayerDeathWithParam)
			characterControllerComponent.m_OnPlayerDeathWithParam.Insert(OnCharacterDeath);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCharacterDeath(SCR_CharacterControllerComponent characterControllerComponent, IEntity instigator)
	{
		SCR_ChimeraCharacter character = characterControllerComponent.GetCharacter();
		if (!character)
			return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
		if (!faction)
			return;
		
		array<SCR_ChimeraCharacter> factionCharacters = m_mFactionSortedCharacters.Get(faction);
		if (!factionCharacters)
			return;
		
		factionCharacters.RemoveItem(character);
		
		m_OnCharacterDeath.Invoke(character);
	}
	
	//------------------------------------------------------------------------------------------------
	//Only for cases when character is removed before dying
	void OnControllableDeleted(IEntity controllable)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controllable);
		if (!character)
			return;
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(character.GetFaction());
		if (!faction)
			return;
		
		array<SCR_ChimeraCharacter> factionCharacters = m_mFactionSortedCharacters.Get(faction);
		if (!factionCharacters)
			return;
		
		if (factionCharacters.Find(character) > -1)
			factionCharacters.RemoveItem(character);
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ChimeraCharacter> GetCharactersByFaction(SCR_CampaignFaction faction)
	{
		array<SCR_ChimeraCharacter> characters = m_mFactionSortedCharacters.Get(faction);
		
		if (!characters)
			return characters;
		
		// TODO matousvoj1: Proper removal of these items once OnDelete is called on entities
		// Added to avoid nulls in arrays
		for (int i = characters.Count() - 1; i >= 0; i--)
		{
			if (!characters[i])
				characters.Remove(i);
		}
		
		return characters;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMinAllyDistance()
	{
		return m_fMinAllyDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxAllyDistance()
	{
		return m_fMaxAllyDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMinEnemyDistance()
	{
		return m_fMinEnemyDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxEnemyDistance()
	{
		return m_fMaxEnemyDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTaskListOpen(SCR_UITaskManagerComponent uiTaskManagerComponent)
	{
		if (!CanRequest())
			return;
		
		if (!m_ClosestBase)
			return;
		
		Widget button = CreateButtonAndSetPadding(uiTaskManagerComponent.GetParentWidget(), uiTaskManagerComponent.GetWidgetsArray());
		if (!button)
			return;
		
		TextWidget textWidget = TextWidget.Cast(button.FindAnyWidget("Text"));
		if (textWidget)
			SetRequestButtonText(textWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_CampaignDefendTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Initialize()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.GetOnControllableSpawned().Insert(OnControllableCreated);
		gameMode.GetOnControllableDeleted().Insert(OnControllableDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignDefendTaskSupportClass()
	{
		m_TypeName = SCR_CampaignDefendTask;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignDefendTaskSupportClass()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnControllableSpawned().Remove(OnControllableCreated);
			gameMode.GetOnControllableDeleted().Remove(OnControllableDeleted);
		}
		
		for (int i = m_mFactionSortedCharacters.Count() - 1; i >= 0; i--)
		{
			array<SCR_ChimeraCharacter> characters = m_mFactionSortedCharacters.GetElement(i);
			if (!characters)
				continue;
			
			for (int j = characters.Count() - 1; j >= 0; j--)
			{
				if (!characters[j])
					continue;
				
				SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(characters[j].FindComponent(SCR_CharacterControllerComponent));
				if (!characterControllerComponent)
					return;
				
				if (characterControllerComponent.m_OnPlayerDeathWithParam)
					characterControllerComponent.m_OnPlayerDeathWithParam.Remove(OnCharacterDeath);
			}
		}
	}
};

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_TransportTaskSupportClass : SCR_RequestedTaskSupportClass
{
	[Attribute("#AR-Tasks_RequestFromTransport")]
	protected LocalizedString m_sRequestFromText;
	
	[Attribute("#AR-Tasks_RequestToTransport")]
	protected LocalizedString m_sRequestToText;
	
	[Attribute("50", "How far can the assignee be from the requester when the task is evaluated.")]
	protected float m_fMaxDistanceAssignee;
	
	[Attribute("50", "How far from the destination is the task evaluated.")]
	protected float m_fMaxDistanceDestination;
	
	protected bool m_bSetFromPosition;
	protected vector m_vFromPosition;
	protected vector m_vToPosition;
	
	//------------------------------------------------------------------------------------------------
	float GetMaxDistanceAssigneeSq()
	{
		return m_fMaxDistanceAssignee * m_fMaxDistanceAssignee;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxDistanceDestinationSq()
	{
		return m_fMaxDistanceDestination * m_fMaxDistanceAssignee;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TransportTask CreateNewTransportTask(int requesterID, vector fromPosition, vector toPosition)
	{
		SCR_TransportTask transportTask = SCR_TransportTask.Cast(CreateTask());
		if (!transportTask)
			return null;
		
		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
		
		transportTask.SetRequester(requester);
		transportTask.SetTargetPosition(toPosition);
		
		return transportTask;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RegisterContextualMenuCallbacks()
	{
		if (m_sRequestButtonText.IsEmpty())
			return;
		
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapContextualMenuUI));
		if (!ctxMenu)
			return;
		
		SCR_MapContextualMenuRequestedTaskEntry entry = new SCR_MapContextualMenuRequestedTaskEntry(m_sRequestButtonText);
		if (!entry)
			return;
		
		ctxMenu.ContextInsertDynamic(entry);
		
		entry.SetSupportClass(this);
		entry.m_OnClick.Insert(Request);
		entry.m_OnShow.Insert(OnContextualEntryShow);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetToPosition()
	{
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapContextualMenuUI));
		if (!ctxMenu)
			return;
		
		m_vToPosition = ctxMenu.GetContextMenuWorldPosition();
		m_bSetFromPosition = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetFromPosition()
	{
		SCR_MapContextualMenuUI ctxMenu = SCR_MapContextualMenuUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapContextualMenuUI));
		if (!ctxMenu)
			return;
		
		m_vFromPosition = ctxMenu.GetContextMenuWorldPosition();
		m_bSetFromPosition = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanRequest()
	{
		return super.CanRequest();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetRequestButtonText(notnull TextWidget textWidget)
	{
		if (!m_bSetFromPosition)
			textWidget.SetTextFormat(m_sRequestFromText);
		else
			textWidget.SetTextFormat(m_sRequestToText);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Request()
	{
		if (!m_bSetFromPosition)
		{
			SetFromPosition();
			return;
		}
		
		SetToPosition();
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		// Find task network component to send RPC to server
		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
		if (!taskNetworkComponent)
			return;
		
		if (!SCR_UIRequestEvacTaskComponent.HasSignal(playerController))
			return;
		
		taskNetworkComponent.RequestTransport(m_vFromPosition, m_vToPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskData CreateTaskData()
	{
		return new SCR_TransportTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TransportTaskSupportClass()
	{
		m_TypeName = SCR_TransportTask;
	}
};