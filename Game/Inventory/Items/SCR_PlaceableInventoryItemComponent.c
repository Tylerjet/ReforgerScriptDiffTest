[EntityEditorProps(category: "GameScripted/Components", description: "ScriptWizard generated script file.")]
class SCR_PlaceableInventoryItemComponentClass : InventoryItemComponentClass
{
	
};

//------------------------------------------------------------------------------------------------
class SCR_PlaceableInventoryItemComponent : SCR_BaseInventoryItemComponent
{
	vector m_vMat[4];
	bool m_bUseTransform = false;
	
	protected bool m_bCanBeGarbageCollected;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeInserted()
	{
		return m_bCanBeGarbageCollected;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoPlaceItem(vector right, vector up, vector forward, vector position)
	{
		IEntity item = GetOwner();
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;
		
		itemComponent.EnablePhysics();
		itemComponent.ActivateOwner(true);
		
		m_vMat[0] = right;
		m_vMat[1] = up;
		m_vMat[2] = forward;
		m_vMat[3] = position;
		m_bUseTransform = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaceItem(vector right, vector up, vector forward, vector position)
	{
		m_bCanBeGarbageCollected = false;
		Rpc(RPC_DoPlaceItem, right, up, forward, position);
		RPC_DoPlaceItem(right, up, forward, position);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OverridePlacementTransform(IEntity caller, out vector computedTransform[4])
	{
		ActivateOwner(true);
		EnablePhysics();
		
		if (m_bUseTransform)
		{
			Math3D.MatrixCopy(m_vMat, computedTransform);
			m_bUseTransform = false;
			return true;
		}
		
		m_bCanBeGarbageCollected = true;
		return false;
	}
};