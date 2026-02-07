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
		
		// In case of unconsciousness starting outside of character, apply effect now
		if (m_pCharacterEntity.GetCharacterController() && m_pCharacterEntity.GetCharacterController().IsUnconscious())
			LowPassFilterEffect();	
	}

	//------------------------------------------------------------------------------------------------
	// Check whether to apply or unapply the filter effect in case of unconsciousness
	override void DisplayConsciousnessChanged(bool conscious, bool init = false)
	{
		LowPassFilterEffect();
	}	
		
	//------------------------------------------------------------------------------------------------
	override void UpdateEffect(float timeSlice, bool playerOutsideCharacter)
	{
		if (m_bLocalPlayerOutsideCharacter == playerOutsideCharacter)
			return;
		
		m_bLocalPlayerOutsideCharacter = playerOutsideCharacter;
		
		// Check whether to apply or unapply the filter effect in case of the controlled camera no longer being the player camera
		LowPassFilterEffect();
	}

	//------------------------------------------------------------------------------------------------
	// Check whether to apply or unapply the filter effect in case of death
	void OnDamageStateChanged()
	{
		LowPassFilterEffect();
	}	
	
	//------------------------------------------------------------------------------------------------
	private void LowPassFilterEffect()
	{
		ECharacterLifeState state = ECharacterLifeState.ALIVE;
		
		if (!m_bLocalPlayerOutsideCharacter && m_pCharacterEntity)
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
		//Run when clearing effect to ensure starting signal state is ECharacterLifeState.ALIVE
		LowPassFilterEffect();
		
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
	}
};