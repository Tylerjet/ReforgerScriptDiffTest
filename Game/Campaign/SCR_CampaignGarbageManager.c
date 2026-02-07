class SCR_CampaignGarbageManagerClass: SCR_GarbageManagerClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignGarbageManager: SCR_GarbageManager
{
	static const int PARKED_SUPPLY_TRUCK_LIFETIME = 3600;	// Lifetime of supply trucks parked near a base
	static const int MAX_BASE_DISTANCE = 300;				// Maximum distance of a base to be consider near
	
	//------------------------------------------------------------------------------------------------
	//! Don't process Mobile HQs
	protected override bool CanInsert(IEntity ent)
	{
		bool defaultReturn = super.CanInsert(ent);
		Vehicle veh = Vehicle.Cast(ent);
		
		if (!veh)
			return defaultReturn;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(veh.FindComponent(SlotManagerComponent));
		
		if (!slotManager)
			return defaultReturn;
		
		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);
			
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			IEntity truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (mobileAssemblyComponent)
				return false;
		}
		
		return defaultReturn;
	}
};