[EntityEditorProps(category: "GameScripted/ScenarioFramework", description: "")]
class SCR_ScenarioFrameworkInventoryLoaderClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_InvCallBackCheck : SCR_InvCallBack
{
	string m_sNameMaster = "";
	string m_sNameItem = "";
	
	//------------------------------------------------------------------------------------------------
	override void OnFailed()
	{
		PrintFormat("ScenarioFramework: The item %1 could not be inserted into %2! Check name of the entity and be sure the entity has a space in the inventory", m_sNameItem, m_sNameMaster, LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	override void OnComplete()
	{
		//PrintFormat("ScenarioFramework: The item %1 has been correctly inserted into %2 inventory", m_sNameItem, m_sNameMaster);
	}
}

class SCR_ScenarioFrameworkInventoryLoader : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(defvalue: "", desc: "Name of the entity whose inventory to be filled with")];
	protected string					m_sIDMaster;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		//m_bMutuallyExclusiveSpawn = false; // ignore this setting otherwise it won't work properly
		if (m_eActivationType != activation)
			return;
		
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				InvokeAllChildrenSpawned();
				return;
			}
		}
		
		super.Init(area, activation);
		
		IEntity master = GetGame().GetWorld().FindEntityByName(m_sIDMaster);
		if (!master)
			return;
		
		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent .Cast(master.FindComponent(InventoryStorageManagerComponent ));
		if (!inventoryComponent)
			return;
			
		IEntity item;
		SCR_InvCallBackCheck pInvCallback = new SCR_InvCallBackCheck();
		pInvCallback.m_sNameMaster = m_sIDMaster;
		foreach (SCR_ScenarioFrameworkLayerBase object : m_aChildren)
		{
			item = object.GetSpawnedEntity();
			if (!item || item == master)
				continue;

			pInvCallback.m_sNameItem = item.GetName();
			//InventoryStorageManagerComponent inventoryComponent.InsertItem(item, null, null, pInvCallback);
			inventoryComponent.TryInsertItem(item, EStoragePurpose.PURPOSE_ANY, pInvCallback);
		}
	}
}
