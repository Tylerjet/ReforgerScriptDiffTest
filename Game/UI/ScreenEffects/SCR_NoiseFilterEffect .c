class SCR_NoiseFilterEffect : SCR_BaseScreenEffect
{
	//Character
	protected ChimeraCharacter 											m_pCharacterEntity;
	protected SCR_CharacterDamageManagerComponent 						m_pDamageManager;
	private bool m_bLocalPlayerOutsideCharacter							= true;

	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();
		
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		
		if (!m_pCharacterEntity)
			return;
		
		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateEffect(float timeSlice, bool playerOutsideCharacter)
	{
		m_bLocalPlayerOutsideCharacter = playerOutsideCharacter;
		LowPassFilterEffect();
	}

	//------------------------------------------------------------------------------------------------
	void OnDamageStateChanged()
	{
		LowPassFilterEffect();
	}
	
	//------------------------------------------------------------------------------------------------
	private void LowPassFilterEffect()
	{
		ECharacterLifeState state = ECharacterLifeState.ALIVE;
		
		if (!m_bLocalPlayerOutsideCharacter)
		{
			SCR_CharacterControllerComponent scrCharController = SCR_CharacterControllerComponent.Cast(m_pCharacterEntity.GetCharacterController());
			state = scrCharController.GetLifeState();
		}
			
		AudioSystem.SetOutputStateSignal(
			AudioSystem.DefaultOutputState,
			AudioSystem.GetOutpuStateSignalIdx(AudioSystem.DefaultOutputState, "CharacterLifeState"),
			state);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (!m_pDamageManager)
			return;

		m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
	}
};