class SCR_PatientCompartmentSlot : CargoCompartmentSlot
{
	protected ref array<HitZone> m_aRegeneratingHitZones;
	
	[Attribute("1", UIWidgets.Auto, "")]
	protected float m_fBleedingRateMultiplier;

	[Attribute("1", UIWidgets.Auto, "")]
	protected float m_fCompartmentRegenerationRateMultiplier;

	//-----------------------------------------------------------------------------------------------------------
	float GetCompartmentRegenRateMultiplier()
	{
		return m_fCompartmentRegenerationRateMultiplier;
	}	
};