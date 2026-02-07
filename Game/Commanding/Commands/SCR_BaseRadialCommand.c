[BaseContainerProps("", "Entry of one command"), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_BaseRadialCommand
{
	[Attribute("", UIWidgets.EditBox, "Unique name of the command")]
	protected string m_sCommandName;
	
	[Attribute("", UIWidgets.EditBox, "Display name of the command, in future will replace the one in commanding menu conf")]
	protected string m_sCommandDisplayName;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourceNamePicker, "Imageset source for display icon" )]
	protected ResourceName m_sImageset;
	
	[Attribute("", UIWidgets.EditBox, "Name of the icon associated to the command, taken from appropriate imageset set in the radial menu" )]
	protected string m_sIconName;
	
	//------------------------------------------------------------------------------------------------
	//! method that will be executed when the command is selected in the menu
	//! This method is right now broadcaasted to all clients, so if the command 
	//! is intended only for specific clients, it needs to be filtered inside 
	// Returns true if the command was executed succesfully, false otherwise.
	bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void VisualizeCommand(vector targetPosition);
	
	//------------------------------------------------------------------------------------------------
	void VisualizeCommandPreview(vector targetPosition);
	
	//------------------------------------------------------------------------------------------------
	//!
	string GetIconName()
	{
		return m_sIconName;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	string GetCommandDisplayName()
	{
		return m_sCommandDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	ResourceName GetIconImageset()
	{
		return m_sImageset;
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	string GetCommandName()
	{
		return m_sCommandName;
	}
			
	//------------------------------------------------------------------------------------------------
	//!
	bool CanRoleShow()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanBeShownInCurrentLifeState()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanBeShown()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanShowOnMap()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	bool CanBePerformed()
	{
		return true;
	}
}