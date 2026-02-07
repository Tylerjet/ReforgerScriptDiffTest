//------------------------------------------------------------------------------------------------
//! Radial menu for handling vehicle weapons on turret positions 
class SCR_RadialMenuVehicleWeapons : SCR_RadialMenuWeapons
{
	//------------------------------------------------------------------------------------------------
	protected override void OpenMenu(IEntity owner, bool isOpen)
	{
		// Call menu opening
		OnMenutoggle(owner, true);
		
		// Prepare all entries
		SetEntriesToDisplay(owner);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Get weapon manager of vehicle turret for reading entries data
	override protected void FindWeaponManager(IEntity owner)
	{
		GenericEntity ownerGeneric = GenericEntity.Cast(owner);
		
		if(!ownerGeneric)
			return;
		
		// Get weapon of vehicle
		BaseVehicleNodeComponent nodeComponent = BaseVehicleNodeComponent.Cast(ownerGeneric.FindComponent(BaseVehicleNodeComponent));
		
		if (!nodeComponent)
			return;

		m_WeaponManager = BaseWeaponManagerComponent.Cast(nodeComponent.FindComponent(BaseWeaponManagerComponent));
	}
	
	// Is turret occupied

};