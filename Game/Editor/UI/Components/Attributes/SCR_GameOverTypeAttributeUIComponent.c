class SCR_GameOverTypeAttributeUIComponent: SCR_DropdownEditorAttributeUIComponent
{	
	protected ref SCR_GameOverScreenConfig m_GameOverScreenConfig;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{
		//Makes sure that var is considered edited
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		var.SetInt(0);
		    
		super.Init(w, attribute);
		
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			GetGame().GetCallqueue().CallLater(HideAttributeDescription, 1);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		UpdateVictoryTooltip(var.GetInt());
	}
	
	protected override void OnChangeComboBox(SCR_ComboBoxComponent comboBox, int value)
	{
		super.OnChangeComboBox(comboBox, value);
		UpdateVictoryTooltip(value);
	}
	
	protected bool SetGameOverConfig()
	{
		if (m_GameOverScreenConfig)
			return true;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return false;

		SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(gameMode.FindComponent(SCR_GameOverScreenManagerComponent));
		if (!gameOverManager)
			return false;
		
		m_GameOverScreenConfig = gameOverManager.GetGameOverConfig();
		
		return true;
	}
	
	protected void UpdateVictoryTooltip(int index)
	{
		if (!SetGameOverConfig())
			return;
		
		int victoryScreenId = (int)m_comboboxData.GetEntryFloatValue(index);
		
		SCR_BaseGameOverScreenInfo gameOverInfo = m_GameOverScreenConfig.GetGameOverInfo(victoryScreenId);
			
		if (!gameOverInfo || gameOverInfo.GetEditorOptionalParams().m_sDescription == string.Empty)
		{
			OverrideDescription(false);
			return;
		}
		
		OverrideDescription(true, gameOverInfo.GetEditorOptionalParams().m_sDescription,  gameOverInfo.GetEditorOptionalParams().m_sDescriptionParam1,  gameOverInfo.GetEditorOptionalParams().m_sDescriptionParam2);
	}
};