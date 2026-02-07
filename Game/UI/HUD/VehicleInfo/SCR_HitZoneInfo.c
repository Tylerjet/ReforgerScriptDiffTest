class SCR_HitZoneInfo: SCR_BaseVehicleInfo
{
	[Attribute("", uiwidget: UIWidgets.EditBox)]
	protected string m_sHitZoneName;
	
	protected HitZone m_pHitZone;
	
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	override bool GetState()
	{
		// Hitzones should not have listeners - it will be costful for memory
		return m_pHitZone && m_pHitZone.GetDamageState() >= EDamageState.DESTROYED;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if there is no hitzone
		if (!m_pHitZone)
			return false;
		
		return super.DisplayStartDrawInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return;
		
		if (m_sHitZoneName.IsEmpty())
			m_pHitZone = damageManager.GetDefaultHitZone();
		else
			m_pHitZone = damageManager.GetHitZoneByName(m_sHitZoneName);
	}
};