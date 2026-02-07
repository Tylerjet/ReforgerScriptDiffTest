//------------------------------------------------------------------------------------------------
class SCR_TaskDeliverClass: CP_TaskClass
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
class SCR_TaskDeliver : CP_Task
{	
	protected string							m_sDeliveryTriggerName;
	
	protected int								m_iObjectState = 0;	//see the table above
	protected  SCR_BaseTriggerEntity			m_pTriggerDeliver;
	protected bool								m_bDeliveryItemFound;
		
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
		IEntity pEnt = GetGame().GetWorld().FindEntityByName(m_sDeliveryTriggerName);
		if (!pEnt)
			return;
		SetDeliveryTrigger(SCR_BaseTriggerEntity.Cast(pEnt));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryTrigger(SCR_BaseTriggerEntity pTrigger)
	{
		if (!pTrigger)
			return;
		m_pTriggerDeliver = pTrigger;
		pTrigger.GetOnActivate().Insert(OnDeliveryTriggerActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTriggerNameToDeliver(string pTriggerName)
	{
		m_sDeliveryTriggerName = pTriggerName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTriggerNameToDeliver()
	{
		return m_sDeliveryTriggerName;
	}
	
	//------------------------------------------------------------------------------------------------	
	void OnDeliveryTriggerActivated(IEntity pEnt)
	{
		if (pEnt == m_pAsset)
		{
			m_pSupportEntity.FinishTask(this);
		}
		else
		{
			InventoryStorageManagerComponent pInventoryComponent = InventoryStorageManagerComponent.Cast(pEnt.FindComponent(InventoryStorageManagerComponent));
			if (!pInventoryComponent)
				return;
			array<IEntity> aItems = {};
			pInventoryComponent.GetItems(aItems);
			if (!aItems.IsEmpty())
			{
				foreach (IEntity pEntity : aItems)
				{
					if (pEntity == m_pAsset)
						m_pSupportEntity.FinishTask(this);		
				}
			}
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectPossessed(IEntity pItem, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!pItem)
			return;
		if (pItem != m_pAsset)
			return;			
		
		UpdateTaskTitleAndDescription(1);
		if (!m_bDeliveryItemFound)
		{
			m_bDeliveryItemFound = true;
			SetState(SCR_TaskState.PROGRESSED);
		}
		
		PrintFormat("CP: ->Task: Item %1 was taken by %2. Task %3 updated.", pItem.GetPrefabData().GetPrefabName(), pStorageOwner.GetOwner(), m_pLayer.GetTask().GetTitle());	
	}
	
	//------------------------------------------------------------------------------------------------
	void OnObjectDropped(IEntity pItem, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!pItem)
			return;
		if (pItem != m_pAsset)
			return;
		
		GarbageManager garbageMan = GetGame().GetGarbageManager();
		if (garbageMan && garbageMan.IsInserted(pItem))
			garbageMan.Withdraw(pItem);
		
		SetState(SCR_TaskState.UPDATED);
		UpdateTaskTitleAndDescription(0);
		PrintFormat("CP: ->Task: Item %1 was dropped by %2. Task %3 updated.", pItem.GetPrefabData().GetPrefabName(), pStorageOwner.GetOwner(), m_pLayer.GetTask().GetTitle());
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
		if (!m_pTriggerDeliver) 
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
				m_pSupportEntity.MoveTask(m_pAsset.GetOrigin(), this.GetTaskID());
			}
		}
		else
		{
			if (iPossessed == 1)
			{
				m_iObjectState = 5;			
				m_pSupportEntity.MoveTask(m_pTriggerDeliver.GetOrigin(), this.GetTaskID());
			}
			else
			{
				//dropped
				m_iObjectState = 6;
				m_pSupportEntity.MoveTask(m_pAsset.GetOrigin(), this.GetTaskID());
			}
		}
								
		CP_SlotTask pSubject = m_pLayer.GetTaskSubject();
		if (!pSubject)
			return;
		
		m_pSupportEntity.SetTaskTitle(this, pSubject.GetTaskTitle(m_iObjectState));
		m_pSupportEntity.SetTaskDescription(this, pSubject.GetTaskDescription(m_iObjectState));	
		
		//m_pLayer.OnTaskStateChanged(GetTaskState(), GetTaskState());			
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
		
		InventoryItemComponent pInvComp = InventoryItemComponent.Cast(m_pAsset.FindComponent(InventoryItemComponent));
		if (!pInvComp)
			return;
		
		InventoryStorageSlot parentSlot = pInvComp.GetParentSlot();
		if (!parentSlot)
			return;
		
		InventoryStorageManagerComponent pInventoryComponent = InventoryStorageManagerComponent.Cast(destroyedEntity.FindComponent(InventoryStorageManagerComponent));
		if (!pInventoryComponent)
			return;
		
		array<IEntity> entityItems = {};
		pInventoryComponent.GetItems(entityItems);
		if (!entityItems.IsEmpty())
		{
			foreach (IEntity pEntity : entityItems)
			{
				if (pEntity == m_pAsset)
				{
					pInventoryComponent.TryRemoveItemFromStorage(m_pAsset, parentSlot.GetStorage());
					m_pAsset.SetOrigin(destroyedEntity.GetOrigin());
					return;	
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDisconnected(int iPlayerID)
	{
		IEntity pPlayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerID);
		if (!pPlayer)
			return;
			
		OnDestroyed(pPlayer);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterPlayer(int iPlayerID)
	{
		IEntity pPlayer = GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerID);
		if (!pPlayer)
			return;
		PrintFormat("CP: player %1 registered", iPlayerID);
		SCR_InventoryStorageManagerComponent pInventoryComponent = SCR_InventoryStorageManagerComponent.Cast(pPlayer.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!pInventoryComponent)
			return;
		pInventoryComponent.m_OnItemAddedInvoker.Insert(OnObjectPossessed);
		pInventoryComponent.m_OnItemRemovedInvoker.Insert(OnObjectDropped);
			
		EventHandlerManagerComponent EventHandlerMgr = EventHandlerManagerComponent.Cast(pPlayer.FindComponent(EventHandlerManagerComponent));
		if (EventHandlerMgr)
			EventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
	}	
		
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_CP_TaskDeliverSupportEntity))
		{
			Print("CP: Task Destroy support entity not found in the world, task won't be created!");
			return false;
		}
		m_pSupportEntity = SCR_CP_TaskDeliverSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_CP_TaskDeliverSupportEntity));
		return m_pSupportEntity != null;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
					
		//Register all players to invoke the callback when any of them takes or drop the items from the inventory
		//TODO: add event to players on JIP
		
		if (!m_pAsset)
			return;
		InventoryItemComponent pInvComp = InventoryItemComponent.Cast(m_pAsset.FindComponent(InventoryItemComponent));
		if (!pInvComp)
			return;
		
		pInvComp.m_OnParentSlotChangedInvoker.Insert(OnItemCarrierChanged);
			
		array<int> aPlayerIDs = {};
		int iNrOfPlayersConnected = GetGame().GetPlayerManager().GetPlayers(aPlayerIDs); 
					
		foreach (int i : aPlayerIDs)
			RegisterPlayer(i);
		SetDeliveryTrigger();
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(RegisterPlayer);
		gameMode.GetOnPlayerDisconnected().Insert(OnDisconnected);
	}
}
