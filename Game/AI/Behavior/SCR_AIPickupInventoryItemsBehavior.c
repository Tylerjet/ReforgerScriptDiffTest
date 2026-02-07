class SCR_AIPickupInventoryItemsBehavior : SCR_AIBehaviorBase
{
	protected static const string PICKUP_POSITION_PORT = "PickupPosition";
	static const string MAGAZINE_WELL_TYPE_PORT = "MagazineWellType";
	
	ref SCR_BTParam<vector> m_vPickupPosition = new SCR_BTParam<vector>(PICKUP_POSITION_PORT);
	ref SCR_BTParam<typename> m_MagazineWellType = new SCR_BTParam<typename>(MAGAZINE_WELL_TYPE_PORT);
	
	ref SCR_AIMoveIndividuallyBehavior m_MoveBehavior;
	
	//-------------------------------------------------------------------------------------------------------------
	void SCR_AIPickupInventoryItemsBehavior(SCR_AIBaseUtilityComponent utility, bool prioritize, SCR_AIActivityBase groupActivity,
		vector pickupPosition, typename magazineWellType)
	{
		m_fPriority = SCR_AIActionBase.PRIORITY_BEHAVIOR_PICKUP_INVENTORY_ITEMS;
		m_sBehaviorTree = "{8522FD17F6E08C47}AI/BehaviorTrees/Chimera/Soldier/PickupInventoryItems.bt";
		m_eType = EAIActionType.PICKUP_INVENTORY_ITEMS;
		
		m_bUniqueInActionQueue = true;		
		
		m_vPickupPosition.Init(this, pickupPosition);
		m_MagazineWellType.Init(this, magazineWellType);
		
		if (!utility)
			return;
		
		float movePriority = m_fPriority + 0.1;
		m_MoveBehavior = new SCR_AIMoveIndividuallyBehavior(utility, prioritize, groupActivity, pos: m_vPickupPosition.m_Value, priority : movePriority, radius: 0.3);
		utility.AddAction(m_MoveBehavior);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_MoveBehavior)
			m_MoveBehavior.Complete();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_MoveBehavior)
			m_MoveBehavior.Fail();
	}
}

class SCR_AIGetPickupInventoryItemsParameters: SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIPickupInventoryItemsBehavior(null, false, null, vector.Zero, BaseMagazineWell)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};