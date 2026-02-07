class SCR_WeaponInfoVehicle : SCR_WeaponInfo
{
	protected TurretComponent m_Turret;
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		if(!owner)
			return false;
		
		m_Turret = TurretComponent.Cast(owner.FindComponent(TurretComponent));
		if(!m_Turret)
			return false;	

		TurretControllerComponent turretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));
		if(!turretController)
			return false;
		
		BaseCompartmentSlot slot = turretController.GetCompartmentSlot();
		if(!slot)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(slot.GetOccupant());
		if(!character)
			return false;
		
		// Detect and store weapon manager
		m_WeaponManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent));
		if (!m_WeaponManager)
			return false;	

		m_MenuManager = GetGame().GetMenuManager();
		if (!m_MenuManager)
			return false;
				
		m_EventHandlerManager = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManager)
		{
			m_EventHandlerManager.RegisterScriptHandler("OnWeaponChanged", this, OnWeaponChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnMuzzleChanged", this, OnMuzzleChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnMagazineChanged", this, OnMagazineChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnMagazineCountChanged", this, OnMagazineCountChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnAmmoCountChanged", this, OnAmmoCountChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnFiremodeChanged", this, OnFiremodeChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnZeroingChanged", this, OnZeroingChanged, true);
			m_EventHandlerManager.RegisterScriptHandler("OnADSChanged", this, OnADSChanged, true);
		}
		
		OnWeaponChanged(m_WeaponManager.GetCurrentWeapon(), null);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_2DSightsComponent GetSights()
	{
		if (!m_Turret)
			return null;
		
		SCR_2DSightsComponent sights = SCR_2DSightsComponent.Cast(m_Turret.FindComponent(SCR_2DSightsComponent));
		
		return sights;
	}
};
