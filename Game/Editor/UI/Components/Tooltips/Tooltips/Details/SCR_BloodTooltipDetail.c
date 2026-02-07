[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_BloodTooltipDetail: SCR_DpsConditionBarBaseTooltipDetail
{	
	protected SCR_CharacterDamageManagerComponent m_CharacterDamageManager;
	
	protected bool m_bCharacterIsBleeding;
	
	override bool NeedUpdate()
	{
		return m_CharacterDamageManager != null;
	}
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		super.UpdateDetail(entity);
		
		HitZone bloodHitzone = m_CharacterDamageManager.GetBloodHitZone();
		if (!bloodHitzone)
			return;
		
		//~ If character dead set blood 0
		if (m_CharacterDamageManager.GetState() == EDamageState.DESTROYED)
		{
			SetBarAndPercentageValue(0);
			return;
		}
		
		//~ Update blood visuals
		float bloodLevelLoseConsciousness = bloodHitzone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS);
		float bloodLevel = Math.InverseLerp(bloodLevelLoseConsciousness, 1, bloodHitzone.GetHealthScaled());
		Math.Clamp(bloodLevel, 0, 1);
		SetBarAndPercentageValue(bloodLevel);
	}
		

	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		if (!super.InitDetail(entity, widget))
			return false;
		
		m_CharacterDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_DamageManager);
		if (!m_CharacterDamageManager)
			return false;
		
		HitZone bleedingHitZone = m_CharacterDamageManager.GetBloodHitZone();
		if (!bleedingHitZone)
			return false;
		
		if (m_CharacterDamageManager.GetState() == EDamageState.DESTROYED)
			return false; 
		
		return true;
	}
};