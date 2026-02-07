[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_BudgetEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/*

*/
class SCR_BudgetEditorComponent : SCR_BaseEditorComponent
{
	const int DEFAULT_MAX_BUDGET = 100;
	const int DEFAULT_MIN_COST = 1;
	
	[Attribute(desc: "Define maximum budget values for player")]
	private ref array<ref SCR_EntityBudgetValue> m_MaxBudgets;
	
	[Attribute(desc: "Define maximum budget values for player in SP local session (no admin mode, can't change max budget setting)")]
	private ref array<ref SCR_EntityBudgetValue> m_MaxBudgetsLocal;
	
	protected SCR_EditableEntityCore m_EntityCore;
		
	protected ref map<EEditableEntityBudget, int> m_CurrentBudgets = new map<EEditableEntityBudget, int>;
	protected ref map<EEditableEntityBudget, int> m_MinBudgetCost = new map<EEditableEntityBudget, int>;
	
	ref ScriptInvoker Event_OnBudgetUpdated = new ScriptInvoker;
	ref ScriptInvoker Event_OnBudgetPreviewUpdated = new ScriptInvoker;
	ref ScriptInvoker Event_OnBudgetPreviewReset = new ScriptInvoker;
	ref ScriptInvoker Event_OnBudgetMaxReached = new ScriptInvoker;
	
	//Delayed set max values
	protected ref map<EEditableEntityBudget, int> m_DelayedSetMaxBudgetsMap = new map<EEditableEntityBudget, int>;
	protected bool m_bListenToMaxBudgetDelay = false;
	
	ScriptInvoker GetOnBudgetUpdatedEvent()
	{
		return Event_OnBudgetUpdated;
	}
	ScriptInvoker GetOnBudgetPreviewUpdatedEvent()
	{
		return Event_OnBudgetPreviewUpdated;
	}
	ScriptInvoker GetOnBudgetPreviewResetEvent()
	{
		return Event_OnBudgetPreviewReset;
	}
	ScriptInvoker GetOnBudgetMaxReached()
	{
		return Event_OnBudgetMaxReached;
	}
	
	/*!
	Checks if budget is sufficient to place passed entity
	\param entity Entity to check, must be placed in world, for checking prefab budget cost use SCR_EditableEntityComponentClass.GetEntityBudgetCost()
	*/
	bool CanPlaceEntity(SCR_EditableEntityComponent entity, out EEditableEntityBudget blockingBudget)
	{
		if (!IsBudgetCapEnabled()) return true;
		
		array<ref SCR_EntityBudgetValue> entityBudgets = {};
		if (GetEntityPreviewBudgetCosts(SCR_EditableEntityUIInfo.Cast(entity.GetInfo()), entityBudgets))
		{
			return CanPlace(entityBudgets, blockingBudget);
		}
		else
		{
			return CanPlaceEntityType(entity.GetEntityType(), blockingBudget);
		}
		
		return false;
	}
	
	/*!
	Checks if budget is sufficient to place passed EditableEntityComponentClass source
	\param IEntityComponentSource SCR_EditableEntityComponentClass to check budget cost of prefab
	*/
	bool CanPlaceEntitySource(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool isPlacingPlayer = false, bool updatePreview = true, bool showBudgetMaxNotification = true)
	{
		if (!IsBudgetCapEnabled()) return true;
		
		if (editableEntitySource)
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (!GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts))
			{
				Rpc(CanPlaceOwner, false, showBudgetMaxNotification);
				return false;
			}
			// Clear budget cost when placing as player
			if (isPlacingPlayer)
			{
				budgetCosts.Clear();
			}
			
			if (updatePreview)
			{
				UpdatePreviewCost(budgetCosts);
			}
			
			return CanPlace(budgetCosts, blockingBudget, showBudgetMaxNotification);
		}
		return false;
	}
	
	/*!
	Fallback function in case no budget costs are defined on the entity
	\param entityType EEditableEntityType of the entity
	\return True if passed array is not empty and entity can be placed according to the passed values
	*/
	bool CanPlaceEntityType(EEditableEntityType entityType, out EEditableEntityBudget blockingBudget)
	{
		if (!IsBudgetCapEnabled()) return true;
		
		array<ref SCR_EntityBudgetValue> budgetCosts;
		GetEntityMinimumBudgetCost(entityType, budgetCosts);
		return CanPlace(budgetCosts, blockingBudget);
	}
	
	/*!
	Checks if budget is sufficient using passed budget costs
	\param budgetCosts array with budget costs values either read from entity in world or directly from prefab
	\return True if passed budgetCosts array is not empty and entity can be placed according to the passed values
	*/
	bool CanPlace(notnull array<ref SCR_EntityBudgetValue> budgetCosts, out EEditableEntityBudget blockingBudget, bool showBudgetMaxNotification = true)
	{
		if (!IsBudgetCapEnabled()) return true;
		
		foreach (SCR_EntityBudgetValue budgetCost : budgetCosts)
		{
			if (!CanPlace(budgetCost, blockingBudget, showBudgetMaxNotification))
			{
				return false;
			}
		}
		return true;
	}
	
	protected bool CanPlace(SCR_EntityBudgetValue budgetCost, out EEditableEntityBudget blockingBudget, bool showBudgetMaxNotification = true)
	{
		EEditableEntityBudget budgetType = budgetCost.GetBudgetType();
		if (GetCurrentBudget(budgetType) + budgetCost.GetBudgetValue() > GetMaxBudgetValue(budgetType))
		{
			blockingBudget = budgetType;			
			Event_OnBudgetMaxReached.Invoke();
			
			Rpc(CanPlaceOwner, false, showBudgetMaxNotification);
			return false;
		}
		else
		{
			Rpc(CanPlaceOwner, true, showBudgetMaxNotification);
			return true;
		}
		
	}
	
	bool GetEntityPreviewBudgetCosts(SCR_EditableEntityUIInfo entityUIInfo, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!entityUIInfo)
		{
			return false;
		}
		
		if (!entityUIInfo.GetEntityBudgetCost(budgetCosts))
		{
			GetEntityMinimumBudgetCost(entityUIInfo.GetEntityType(), budgetCosts);
		}
		
		array<ref SCR_EntityBudgetValue> entityChildrenBudgetCosts = {};
		entityUIInfo.GetEntityChildrenBudgetCost(entityChildrenBudgetCosts);
		
		SCR_EntityBudgetValue.MergeBudgetCosts(budgetCosts, entityChildrenBudgetCosts);
		
		return !budgetCosts.IsEmpty();
	}
	
	bool GetEntitySourcePreviewBudgetCosts(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		if (!editableEntitySource)
		{
			return false;
		}
		
 		SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (editableUIInfo)
		{
			editableUIInfo.InitFromSource(editableEntitySource);
			return GetEntityPreviewBudgetCosts(editableUIInfo, budgetCosts);
		}
		
		return false;
	}
	
	void GetEntityMinimumBudgetCost(EEditableEntityType entityType, out array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		EEditableEntityBudget entityBudgetType = m_EntityCore.GetBudgetForEntityType(entityType);
		int minBudgetCost = GetMinBudgetCost(entityBudgetType);
		
		budgetCosts = { new SCR_EntityBudgetValue(entityBudgetType, minBudgetCost)};
	}
	
	void UpdatePreviewCost(notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{
		ResetPreviewCost();
		foreach (SCR_EntityBudgetValue budgetCost : budgetCosts)
		{
			EEditableEntityBudget budgetType = budgetCost.GetBudgetType();
			int budgetValue = budgetCost.GetBudgetValue();
			float maxBudget = (float) GetMaxBudgetValue(budgetType);
			
			// Calculate current budget percentage
			int currentBudget = GetCurrentBudget(budgetType);
			float newBudgetPercent;
			float budgetChange;
			if (maxBudget != 0)
			{
				// Calculate budget percentage after entity is placed
				float currentBudgetPercent = currentBudget / maxBudget * 100;
				int newBudget = currentBudget + budgetValue;
				
				newBudgetPercent = newBudget / maxBudget * 100;
				// How much the budget percentage is increased
				budgetChange = newBudgetPercent - currentBudgetPercent;
			}
			else
			{
				newBudgetPercent = 100;
				budgetChange = 0; // Show no change when maxbudget is 0
			}
			
			Event_OnBudgetPreviewUpdated.Invoke(budgetType, newBudgetPercent, budgetChange);
		}
	}
	
	void ResetPreviewCost()
	{
		Event_OnBudgetPreviewReset.Invoke();
	}
	
	/*!
	Get max budget for this player per budget type
	*/
	int GetMaxBudgetValue(EEditableEntityBudget type)
	{
		SCR_EntityBudgetValue budgetValue;
		if (GetMaxBudget(type, budgetValue))
		{
			return budgetValue.GetBudgetValue();
		}
		return DEFAULT_MAX_BUDGET;
	}
	
	/*!
	Set max budget for this player for budget type
	*/
	void SetMaxBudgetValue(EEditableEntityBudget type, int newValue)
	{		
		SCR_EntityBudgetValue budgetValue;
		if (GetMaxBudget(type, budgetValue))
		{
			budgetValue.SetBudgetValue(newValue);
		}
		Rpc(UpdateMaxBudgetOwner, type, newValue);
	}
	
	int GetMinBudgetCost(EEditableEntityBudget type)
	{
		int minCost;
		if (m_MinBudgetCost.Find(type, minCost))
		{
			return minCost;
		}
		return DEFAULT_MIN_COST;
	}
	
	void DelayedSetMaxBudgetSetup(EEditableEntityBudget type, int newValue, int playerChangingBudget)
	{
		m_DelayedSetMaxBudgetsMap.Insert(type, newValue);
		
		if (!m_bListenToMaxBudgetDelay)
		{
			m_bListenToMaxBudgetDelay = true;			
			GetGame().GetCallqueue().CallLater(DelayedSetMaxBudget, 100, false, playerChangingBudget);
		}
	}
	
	protected void DelayedSetMaxBudget(int playerChangingBudget)
	{
		m_bListenToMaxBudgetDelay = false;
		SetMultiMaxBudgetValues(m_DelayedSetMaxBudgetsMap, playerChangingBudget);
		m_DelayedSetMaxBudgetsMap.Clear();
	}
	
	void SetMultiMaxBudgetValues(notnull map<EEditableEntityBudget, int> budgets, int playerChangingBudget)
	{		
		foreach(EEditableEntityBudget type, int budget: budgets)
   	 	{
       		SetMaxBudgetValue(type, budget);
   		}
		
		//Notification
		/*SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			int playerNotificationID = editorManager.GetPlayerID();
			SCR_NotificationsComponent.SendToGameMastersAndPlayer(playerNotificationID, ENotification.EDITOR_ATTRIBUTES_GM_BUDGET_CHANGED, playerChangingBudget, playerNotificationID);
		}*/
	}
	
	bool GetMaxBudget(EEditableEntityType type, out SCR_EntityBudgetValue budget)
	{
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			if (maxBudget.GetBudgetType() == type)
			{
				budget = maxBudget;
				return true;
			}
		}
		return false;
	}
	
	/*!
	Get last received budget value for type
	*/
	int GetCurrentBudget(EEditableEntityBudget type)
	{
		int budgetValue = 0;
		m_CurrentBudgets.Find(type, budgetValue);
		return budgetValue;
	}
	
	/*!
	Get default budget definitions from core, budget values not synced on client, use GetCurrentBudget for updated budget
	*/
	void GetBudgets(out notnull array<ref SCR_EditableEntityCoreBudgetSetting> budgets)
	{
		m_EntityCore.GetBudgets(budgets);
		
		foreach (SCR_EditableEntityCoreBudgetSetting budget : budgets)
		{
			m_MinBudgetCost.Set(budget.GetBudgetType(), budget.GetMinBudgetCost());
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void UpdateBudgetOwner(int budgetType, int budgetValue)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES))
		{
			PrintFormat("Updated budget received for type %1: %2", budgetType, budgetValue);
		}
		
		m_CurrentBudgets.Set(budgetType, budgetValue);
		Event_OnBudgetUpdated.Invoke(budgetType, budgetValue, GetMaxBudgetValue(budgetType));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void UpdateMaxBudgetOwner(int budgetType, int maxBudgetValue)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES))
		{
			PrintFormat("Updated maximum budget received for type %1: %2", budgetType, maxBudgetValue);
		}
		
		SCR_EntityBudgetValue maxBudget;
		if (GetMaxBudget(budgetType, maxBudget))
		{
			maxBudget.SetBudgetValue(maxBudgetValue);
		}
		Event_OnBudgetUpdated.Invoke(budgetType, GetCurrentBudget(budgetType), maxBudgetValue);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CanPlaceOwner(bool canPlace, bool showBudgetMaxNotification)
	{
		if (!canPlace)
		{
			Event_OnBudgetMaxReached.Invoke();
			
			if (showBudgetMaxNotification)
			{
				GetManager().SendNotification(ENotification.EDITOR_PLACING_BUDGET_MAX);
			}
		}
	}
	
	protected void OnEntityBudgetChanged(SCR_EditableEntityCoreBudgetSetting budget)
	{
		if (!budget) return;
		
		EEditableEntityBudget budgetType = budget.GetBudgetType();
		int updatedBudgetValue = budget.GetCurrentBudget();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES))
		{
			PrintFormat("New budget for type %1: %2", budgetType, updatedBudgetValue);
		}
		UpdateBudgetOwner(budgetType, updatedBudgetValue);
		Rpc(UpdateBudgetOwner, budgetType, updatedBudgetValue);
	}
	
	protected bool IsBudgetCapEnabled()
	{
	#ifdef ENABLE_DIAG
		return DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP);
	#else
		return true;
	#endif
	}
	
	protected override void EOnEditorInitServer()
	{
		super.EOnEditorInitServer();
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_EntityCore.Event_OnEntityBudgetChanged.Insert(OnEntityBudgetChanged);
		if (!Replication.IsRunning() && !m_MaxBudgetsLocal.IsEmpty())
		{
			m_MaxBudgets = m_MaxBudgetsLocal;
		}
	}
	
	protected override void EOnEditorInit()
	{
		super.EOnEditorInit();
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		
	#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES, "", "Log Budget Changes", "Editable Entities");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP, "", "Enable Budget Cap", "Editable Entities");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP, 1);
	#endif
	}
	
	void ~SCR_BudgetEditorComponent()
	{
		if (m_EntityCore && m_RplComponent && !m_RplComponent.IsProxy())
		{
			m_EntityCore.Event_OnEntityBudgetChanged.Remove(OnEntityBudgetChanged);
		}
		
	#ifdef ENABLE_DIAG
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_LOG_BUDGET_CHANGES);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ENTITIES_BUDGET_CAP);
	#endif
	}
};