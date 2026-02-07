[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalAction
{
	SCR_CharacterDamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	void Init(SCR_ChimeraCharacter character)
	{
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!m_DamageManager)
		{
			Print(string.Format("ScenarioFramework Action: Character Damage Manager Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate();
}