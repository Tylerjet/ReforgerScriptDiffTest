class SCR_DummyMenuEntry : ScriptedSelectionMenuEntry
{	
	[Attribute("", UIWidgets.ResourceNamePicker, "")]
	protected ResourceName m_sIcon;
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry is supposed to be performed
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		super.OnPerform(user, sourceMenu);
		
		Print("this dummy is performable!");
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be shown?
	protected override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryIconPathScript(out string outIconPath)
	{
		if(m_sIcon)
		{
			outIconPath = m_sIcon;
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override UIInfo GetUIInfoScript()
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DummyMenuEntry()
	{

	}
};