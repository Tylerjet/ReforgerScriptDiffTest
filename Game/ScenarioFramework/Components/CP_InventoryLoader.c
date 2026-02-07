[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class CP_InventoryLoaderClass : CP_LayerTaskClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	
*/

class SCR_InvCallBackCheck : SCR_InvCallBack
{
	string m_sNameMaster = "";
	string m_sNameItem = "";
	
	//------------------------------------------------------------------------------------------------
	override void OnFailed()
	{
		PrintFormat("CP: The item %1 could not be inserted into %2! Check name of the entity and be sure the entity has a space in the inventory", m_sNameItem, m_sNameMaster);
	}

	//------------------------------------------------------------------------------------------------
	override void OnComplete()
	{
		PrintFormat("CP: The item %1 has been correctly inserted into %2 inventory", m_sNameItem, m_sNameMaster);
	}
}

//------------------------------------------------------------------------------------------------
class CP_InventoryLoader : CP_LayerBase
{		
	
	[Attribute(defvalue: "", desc: "Name of the entity whose inventory to be filled with")];
	protected string					m_sIDMaster;
	
	
	//------------------------------------------------------------------------------------------------
	override void Init(CP_Area pArea = null, CP_EActivationType EActivation = CP_EActivationType.SAME_AS_PARENT, bool bInit = true)
	{
		//m_bMutuallyExclusiveSpawn = false; // ignore this setting otherwise it won't work properly
		if (m_EActivationType != EActivation)
			return;
		super.Init(pArea, EActivation);
		
		IEntity pMaster = GetGame().GetWorld().FindEntityByName(m_sIDMaster);
		if (!pMaster)
			return;
		
		InventoryStorageManagerComponent pInventoryComponent = InventoryStorageManagerComponent .Cast(pMaster.FindComponent(InventoryStorageManagerComponent ));
		if (!pInventoryComponent)
			return;
		IEntity pItem;
		SCR_InvCallBackCheck pInvCallback = new SCR_InvCallBackCheck;
		pInvCallback.m_sNameMaster = m_sIDMaster;
		foreach (CP_LayerBase pObject : m_aChildren)
		{
			pItem = pObject.GetSpawnedEntity();
			if (!pItem || pItem == pMaster)
				continue;
			pInvCallback.m_sNameItem = pItem.GetName();
			//InventoryStorageManagerComponent pInventoryComponent.InsertItem(pItem, null, null, pInvCallback);
			pInventoryComponent.TryInsertItem(pItem, EStoragePurpose.PURPOSE_ANY, pInvCallback);
		}
		
	}
	
	
	//------------------------------------------------------------------------------------------------
	void CP_InventoryLoader(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	} 

	//------------------------------------------------------------------------------------------------
	void ~CP_InventoryLoader()
	{
	}
}
