class SCR_UnconsciousScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of UnconsciousnessEffect()
	protected const float UNCONSCIOUS_FADEIN_OPACITY_DURATION 			= 0.8;
	protected const float UNCONSCIOUS_FADEIN_PROGRESSION_DURACTION		= 0.5;
	protected const float UNCONSCIOUS_FADEIN_OPACITY_TARGET				= 0.98;
	protected const float UNCONSCIOUS_FADEIN_PROGRESSION_TARGET 		= 0.5;
	
	// Widgets
	protected ImageWidget							m_wDeath;
	
	//Character
	protected ChimeraCharacter m_pCharacterEntity;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
	 	m_pCharacterEntity = ChimeraCharacter.Cast(owner);
		m_wDeath = ImageWidget.Cast(m_wRoot.FindAnyWidget("DeathOverlay"));

		GetGame().OnUserSettingsChangedInvoker().Insert(SettingsChanged);
	}

	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();

		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;
				
		m_EventHandlerManager = EventHandlerManagerComponent.Cast(m_pCharacterEntity.FindComponent(EventHandlerManagerComponent));
		if (m_EventHandlerManager)
			m_EventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);

		// In case of unconsciousness starting outside of character, apply effect now
		if (m_pCharacterEntity.GetCharacterController() && m_pCharacterEntity.GetCharacterController().IsUnconscious())
			UnconsciousnessEffect(false);	
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		UnconsciousnessEffect(conscious);
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO@FAC scale durations based on progression of existing death effect
	void UnconsciousnessEffect(bool conscious)
	{
		if (!m_wDeath)
			return;
		
		if (!conscious)
		{
			m_wDeath.SetOpacity(m_wDeath.GetOpacity());
			m_wDeath.SetMaskProgress(0);
			AnimateWidget.Opacity(m_wDeath, UNCONSCIOUS_FADEIN_OPACITY_TARGET, UNCONSCIOUS_FADEIN_OPACITY_DURATION);
			AnimateWidget.AlphaMask(m_wDeath, UNCONSCIOUS_FADEIN_PROGRESSION_TARGET, UNCONSCIOUS_FADEIN_PROGRESSION_DURACTION);
		}
		else
		{
			AnimateWidget.Opacity(m_wDeath, 0, UNCONSCIOUS_FADEIN_OPACITY_DURATION);
			AnimateWidget.AlphaMask(m_wDeath, 0, UNCONSCIOUS_FADEIN_PROGRESSION_DURACTION);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterEffects()
	{
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_wDeath)
		{
			AnimateWidget.StopAllAnimations(m_wDeath);
			m_wDeath.SetOpacity(0);
			m_wDeath.SetMaskProgress(0);
		}
		
		UnregisterEffects();
	}
};