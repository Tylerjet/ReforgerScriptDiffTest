class SCR_TurretWeaponMenuEntry : SCR_WeaponSwitchSelectionMenuEntry
{
	protected BaseControllerComponent m_pController; // Vehicle
	//protected WeaponSlotComponent m_pTargetSlot;
	//protected IEntity m_pOwner; // Character
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry is supposed to be performed
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		// Select tuuret weapon
		TurretControllerComponent turretController = TurretControllerComponent.Cast(m_pController);
		if (turretController)
		{
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();
			turretController.SelectWeapon(user, m_pTargetSlot);
		}
		
		super.OnPerform(user, sourceMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can this entry be shown?
	/*protected override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryIconPathScript(out string outIconPath)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override UIInfo GetUIInfoScript()
	{
		if (m_pTargetSlot)
			return m_pTargetSlot.GetUIInfo();
		
		return null;
	}*/
	
	//------------------------------------------------------------------------------------------------
	void SCR_TurretWeaponMenuEntry(IEntity owner, WeaponSlotComponent weaponSlot, BaseControllerComponent controller)
	{
		m_pOwner = owner;
		m_pTargetSlot = weaponSlot;
		m_pController = controller;
	}
};