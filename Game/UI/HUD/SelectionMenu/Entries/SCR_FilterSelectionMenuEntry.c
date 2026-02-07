//------------------------------------------------------------------------------------------------
class SCR_FilterSelectionMenuEntry : ScriptedSelectionMenuEntry
{
	int m_iFilter;

	//------------------------------------------------------------------------------------------------
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
		if (!gadgetManager)
			return;
		
		SCR_FlashlightComponent flashlightComp = SCR_FlashlightComponent.Cast( gadgetManager.GetHeldGadgetComponent() );
		if (flashlightComp)
			flashlightComp.SwitchLenses(m_iFilter);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetEntryNameScript(out string outName)
	{		
		outName = m_sName;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetEntryDescriptionScript(out string outDescription)
	{		
		outDescription = "Select lense filter";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_FilterSelectionMenuEntry(int id, string name)
	{
		m_iFilter = id;
		m_sName = name;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_FilterSelectionMenuEntry()
	{
	}
	
};
