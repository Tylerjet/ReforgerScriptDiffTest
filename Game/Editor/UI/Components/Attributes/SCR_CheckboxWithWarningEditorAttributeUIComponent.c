class SCR_CheckboxWithWarningEditorAttributeUIComponent: SCR_CheckboxEditorAttributeUIComponent
{
	[Attribute("0")]
	protected bool m_bShowWarningNoteOnTrue;
	
	[Attribute("1")]
	protected bool m_bOnlyShowWarningOnChanged;
	
	[Attribute()]
	protected LocalizedString m_sWarningNote;
	
	protected bool m_bStartingValue;
	
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{	
		if (!var)
			return;
		
		m_bStartingValue = var.GetBool();
		
		if (!m_bOnlyShowWarningOnChanged)
			OverrideDescription(m_bStartingValue == m_bShowWarningNoteOnTrue, m_sWarningNote);
		super.SetFromVar(var);
	}
	
	protected override void OnChangeCheckbox(SCR_SelectionWidgetComponent selectionBox, bool value)
	{
		super.OnChangeCheckbox(selectionBox, value);
		OverrideDescription(((value != m_bStartingValue || !m_bOnlyShowWarningOnChanged) && value == m_bShowWarningNoteOnTrue), m_sWarningNote);
	}
};