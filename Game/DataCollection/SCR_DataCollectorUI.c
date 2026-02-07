[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_DataCollectorUIClass : ScriptComponentClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class SCR_DataCollectorUI : ScriptComponent
{
	[Attribute("pathtolayout")]
	protected ResourceName m_sBaseLayout;
	
	[Attribute("pathtolayout")]
	protected ResourceName m_sEntryLayout;
	
	protected Widget m_wParentWidget;
	
	//------------------------------------------------------------------------------------------------
	Widget CreateEntry()
	{
		if (!GetGame() || !GetGame().GetWorkspace())
			return null;
		
		if (!m_wParentWidget)
		{
			m_wParentWidget = GetGame().GetWorkspace().CreateWidgets(m_sBaseLayout);
			if (!m_wParentWidget)
				return null;
		}
		
		Widget entriesWidget = m_wParentWidget.FindAnyWidget("Entries");
		if (!entriesWidget)
			return null;
		
		return GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, entriesWidget);
	}
	
	Widget GetParentWidget()
	{
		return m_wParentWidget;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_wParentWidget = GetGame().GetWorkspace().CreateWidgets(m_sBaseLayout);
	}
}
