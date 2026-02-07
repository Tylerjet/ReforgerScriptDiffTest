//------------------------------------------------------------------------------------------------
class SCR_RadialMenuMapInteractions : SCR_RadialMenuInteractions
{
	//------------------------------------------------------------------------------------------------
	override protected bool CanBeOpened()
	{
		SCR_MapCursorModule mapCursor = SCR_MapCursorModule.Cast(SCR_MapEntity.GetMapInstance().GetMapModule(SCR_MapCursorModule));
		if (mapCursor)
		{
			if (mapCursor.GetCursorState() & mapCursor.STATE_CTXMENU_RESTRICTED)
				return false;
		}
		
		SCR_MapContextualMenuUI mapRadial = SCR_MapContextualMenuUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapContextualMenuUI));
		if (mapRadial)
		{
			if (mapRadial.GetEntryCount() != 0)
				return true;	
		}
		
		return false;
	}
};
