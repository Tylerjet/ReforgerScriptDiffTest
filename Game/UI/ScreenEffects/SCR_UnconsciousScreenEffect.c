class SCR_UnconsciousScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of UnconsciousnessEffect()
	protected const float UNCONSCIOUS_FADEIN_OPACITY_DURATION = 0.8;
	protected const float UNCONSCIOUS_FADEIN_PROGRESSION_DURACTION = 0.5;
	protected const float UNCONSCIOUS_FADEIN_OPACITY_TARGET = 0.98;
	protected const float UNCONSCIOUS_FADEIN_PROGRESSION_TARGET = 0.5;
	
	// Widgets
	protected ImageWidget m_wDeath;
	
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
		//Print(string.Format(">> %1 [DisplayControlledEntityChanged] %2 -> %3", this, from, to));
		
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DisplayConsciousnessChanged(bool conscious, bool init = false)
	{
		//Print(string.Format(">> %1 [DisplayConsciousnessChanged] conscious: %2 | init: %3", this, conscious, init));
		
		UnconsciousnessEffect(conscious, init);
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO@FAC scale durations based on progression of existing death effect
	void UnconsciousnessEffect(bool conscious, bool init = false)
	{
		//Print(string.Format(">> %1 [UnconsciousnessEffect] conscious: %2", this, conscious));
		
		if (!m_wDeath)
			return;
		
		if (init)
		{
			AnimateWidget.StopAllAnimations(m_wDeath);
			
			if (!conscious)
			{
				m_wDeath.SetOpacity(UNCONSCIOUS_FADEIN_OPACITY_TARGET);
				m_wDeath.SetMaskProgress(UNCONSCIOUS_FADEIN_PROGRESSION_TARGET);	
			}
			else
			{
				m_wDeath.SetOpacity(0);
				m_wDeath.SetMaskProgress(0);	
			}		
		}
		else
		{
			if (!conscious)
			{
				AnimateWidget.Opacity(m_wDeath, UNCONSCIOUS_FADEIN_OPACITY_TARGET, UNCONSCIOUS_FADEIN_OPACITY_DURATION);
				AnimateWidget.AlphaMask(m_wDeath, UNCONSCIOUS_FADEIN_PROGRESSION_TARGET, UNCONSCIOUS_FADEIN_PROGRESSION_DURACTION);			
			}
			else
			{
				AnimateWidget.Opacity(m_wDeath, 0, UNCONSCIOUS_FADEIN_OPACITY_DURATION);
				AnimateWidget.AlphaMask(m_wDeath, 0, UNCONSCIOUS_FADEIN_PROGRESSION_DURACTION);
			}		
		}
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
	}
};