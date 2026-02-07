//------------------------------------------------------------------------------------------------
class SCR_TaskDeliverClass: SCR_ScenarioFrameworkTaskClass
{
};

/*
			  TASK DELIVERY STATES
	-----------------------------------------
	| Delivery		| Item		| Item		|
	| Point exists 	| possesed	| Dropped 	|
	-----------------------------------------
	|	0			|	0		|	0		|	0
	-----------------------------------------
	|	0			|	0		|	1		|	1
	-----------------------------------------
	|	0			|	1		|	0		|	2
	-----------------------------------------
	|	1			|	0		|	0		|	4
	-----------------------------------------
	|	1			|	0		|	1		|	5
	-----------------------------------------
	|	1			|	1		|	0		|	6
	-----------------------------------------
*/

//------------------------------------------------------------------------------------------------
class SCR_TaskDeliver : SCR_ScenarioFrameworkTask
{	
	protected string							m_sDeliveryTriggerName;
	
	protected int								m_iObjectState = 0;	//see the table above
	protected  SCR_BaseTriggerEntity			m_TriggerDeliver;
	protected bool								m_bDeliveryItemFound;
	protected bool 								m_bTaskPositionUpdated;
		
	//------------------------------------------------------------------------------------------------
	// return 0 if item is not possesed and 1 if possesed (someone has it in his possesion)
	int GetTaskDeliverState()
	{
		return m_iObjectState;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryTrigger()
	{
		if (!GetGame().GetWorld())
			return;
		
		if (m_sDeliveryTriggerName.IsEmpty())
		{
			Print("ScenarioFramework: Task Deliver trigger is set with empty attribute", LogLevel.ERROR);
			return;
		}	

		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sDeliveryTriggerName);
		if (!entity)
			return;

		SetDeliveryTrigger(SCR_BaseTriggerEntity.Cast(entity));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryTrigger(SCR_BaseTriggerEntity trigger)
	{
		if (!trigger)
			return;

		m_TriggerDeliver = trigger;
		trigger.GetOnActivate().Insert(OnDeliveryTriggerActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTriggerNameToDeliver(string triggerName)
	{
		if (triggerName.IsEmpty())
		{
			Print("ScenarioFramework: Task Deliver trigger is set with empty attribute", LogLevel.ERROR);
			return;
		}
		
		m_sDeliveryTriggerName = triggerName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTriggerNameToDeliver()
	{
		return m_sDeliveryTriggerName;
	}
	
	//------------------------------------------------------------------------------------------------	
	void OnDeliveryTriggerActivated(IEntity entity)
	{
		if (!entity || !m_SupportEntity)
			return;
		
		if (entity == m_Asset)
		{
			m_SupportEntity.FinishTask(this);
			return;
		}
		
		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(entity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;

		array<IEntity> aItems = {};
		inventoryComponent.GetItems(aItems);
		if (aItems.IsEmpty())
			return;
		
		foreach (IEntity itemEntity : aItems)
		{
			if (itemEntity == m_Asset)
				m_SupportEntity.FinishTask(this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectPossessed(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_Asset)
			return;
		
		UpdateTaskTitleAndDescription(1);
		if (!m_bDeliveryItemFound)
		{
			m_bDeliveryItemFound = true;
			SetState(SCR_TaskState.PROGRESSED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectDropped(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_Asset)
			return;
		
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager && garbageManager.IsInserted(item))
			garbageManager.Withdraw(item);
		
		SetState(SCR_TaskState.UPDATED);
		UpdateTaskTitleAndDescription(0);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateTaskTitleAndDescription(int iPossessed = -1)
	{
		if (iPossessed == -1)
		{
			//the request comes from outside (most likely the Delivery point has been created)
			//check if the item has been already possessed or not
			if (m_iObjectState & 0x01)
				iPossessed = 1;
			else 
				iPossessed = 0;
		}
		
		//Decide based on the task state - see the table on top of the class
		if (!m_TriggerDeliver) 
		{
			//delivery point still doesn't exist
			if (iPossessed == 1)
			{
				m_iObjectState = 1;		
			}
			else
			{
				//dropped
				m_iObjectState = 2;	
				if (!m_bTaskPositionUpdated)
					UpdateDroppedTaskMarker();
			}	
		}
		else
		{
			if (iPossessed == 1)
			{
				m_iObjectState = 5;			
				m_SupportEntity.MoveTask(m_TriggerDeliver.GetOrigin(), this.GetTaskID());
			}
			else
			{
				//dropped
				m_iObjectState = 6;
				if (!m_bTaskPositionUpdated)
					UpdateDroppedTaskMarker();
			}
		}
								
		SCR_ScenarioFrameworkSlotTask subject = m_Layer.GetTaskSubject();
		if (!subject)
			return;
		
		m_SupportEntity.SetTaskTitle(this, subject.GetTaskTitle(m_iObjectState));
		m_SupportEntity.SetTaskDescription(this, subject.GetTaskDescription(m_iObjectState));	
		
		//m_Layer.OnTaskStateChanged(GetTaskState(), GetTaskState());			
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDroppedTaskMarker()
	{
		m_bTaskPositionUpdated = true;
		SCR_ScenarioFrameworkLayerTaskDeliver layerTaskDeliver = SCR_ScenarioFrameworkLayerTaskDeliver.Cast(m_Layer);
		if (!layerTaskDeliver)
			return;
					
		//We want to delay position movement of the Task marker on the map by given time
		GetGame().GetCallqueue().CallLater(MoveTaskMarkerPosition, 1000 * layerTaskDeliver.GetIntelMapMarkerUpdateDelay(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MoveTaskMarkerPosition()
	{
		m_bTaskPositionUpdated = false;
		m_SupportEntity.MoveTask(m_Asset.GetOrigin(), this.GetTaskID());
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateMapTaskIcon()
	{
		super.UpdateMapTaskIcon();
		if (!GetTaskIconkWidget())
			return;

		if (m_iObjectState == 1)
		{
			GetTaskIconkWidget().SetOpacity(0);	//hide the icon on map until the delivery point isn't created
			GetTaskIconkWidget().SetVisible(false);
		}
		else
		{
			GetTaskIconkWidget().SetOpacity(1);
			GetTaskIconkWidget().SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemCarrierChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		if (oldSlot)
		{
			EventHandlerManagerComponent EventHandlerMgr = EventHandlerManagerComponent.Cast(oldSlot.GetOwner().FindComponent(EventHandlerManagerComponent));
			if (EventHandlerMgr)
				EventHandlerMgr.RemoveScriptHandler("OnDestroyed", this, OnDestroyed);
		}
		
		if (newSlot)
		{
			EventHandlerManagerComponent EventHandlerMgr = EventHandlerManagerComponent.Cast(newSlot.GetOwner().FindComponent(EventHandlerManagerComponent));
			if (EventHandlerMgr)
				EventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to remove task item from destroyed entity inventory and drop it to the ground
	protected void OnDestroyed(IEntity destroyedEntity)
	{
		if (!destroyedEntity)
			return;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Asset.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		InventoryStorageSlot parentSlot = invComp.GetParentSlot();
		if (!parentSlot)
			return;
		
		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(destroyedEntity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		array<IEntity> entityItems = {};
		inventoryComponent.GetItems(entityItems);
		if (!entityItems.IsEmpty())
			return;
		
		foreach (IEntity entity : entityItems)
		{
			if (entity == m_Asset)
			{
				inventoryComponent.TryRemoveItemFromStorage(m_Asset, parentSlot.GetStorage());
				m_Asset.SetOrigin(destroyedEntity.GetOrigin());
				return;	
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDisconnected(int iPlayerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerID);
		if (!player)
			return;
			
		OnDestroyed(player);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterPlayer(int iPlayerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerID);
		if (!player)
			return;

		SCR_InventoryStorageManagerComponent inventoryComponent = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;

		inventoryComponent.m_OnItemAddedInvoker.Insert(OnObjectPossessed);
		inventoryComponent.m_OnItemRemovedInvoker.Insert(OnObjectDropped);
			
		EventHandlerManagerComponent EventHandlerMgr = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (EventHandlerMgr)
			EventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
	}	
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskDeliverSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDeliverSupportEntity));
		
		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Task Deliver support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		//Register all players to invoke the callback when any of them takes or drop the items from the inventory
		//TODO: add event to players on JIP
		
		if (!m_Asset)
			return;
			
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Asset.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		invComp.m_OnParentSlotChangedInvoker.Insert(OnItemCarrierChanged);
			
		array<int> aPlayerIDs = {};
		int iNrOfPlayersConnected = GetGame().GetPlayerManager().GetPlayers(aPlayerIDs); 
					
		foreach (int i : aPlayerIDs)
		{
			RegisterPlayer(i);
		}
		
		SetDeliveryTrigger();
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(RegisterPlayer);
		gameMode.GetOnPlayerDisconnected().Insert(OnDisconnected);
	}
}
