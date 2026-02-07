class SCR_OpenVehicleStorageAction : SCR_InventoryAction
{
#ifndef DISABLE_INVENTORY
	[Attribute("1")]
	protected bool m_bShowFromOutside;

	[Attribute("1")]
	protected bool m_bShowInside;

	protected IEntity m_Vehicle;

	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.SetStorageToOpen(pOwnerEntity);
		manager.OpenInventory();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!user || !m_Vehicle)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!m_bShowFromOutside && !character.IsInVehicle())
			return false;

		if (!m_bShowInside && character.IsInVehicle())
			return false;

		FactionAffiliationComponent vehicleFaction = FactionAffiliationComponent.Cast(m_Vehicle.FindComponent(FactionAffiliationComponent));
		FactionAffiliationComponent userFaction = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
		if (!vehicleFaction || !userFaction)
			return false;

		if (!vehicleFaction.GetAffiliatedFaction())
			return true;
		
		if (!userFaction.GetAffiliatedFaction())
			return false;

		return (vehicleFaction.GetAffiliatedFaction() == userFaction.GetAffiliatedFaction());
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!Vehicle.Cast(pOwnerEntity))
		{
			m_Vehicle = pOwnerEntity.GetParent();
		}
		else
		{
			m_Vehicle = pOwnerEntity;
		}
	}
#endif
};