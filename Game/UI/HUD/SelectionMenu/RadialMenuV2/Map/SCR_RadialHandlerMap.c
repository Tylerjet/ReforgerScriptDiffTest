//------------------------------------------------------------------------------------------------
class SCR_RadialHandlerMap : SCR_RadialMenuHandler
{
	//------------------------------------------------------------------------------------------------
	void InitMapHandler()
	{
		m_pRadialMenuInteractions = new SCR_RadialMenuMapInteractions();
		Init(GetGame().GetPlayerController());
		m_pRadialMenuInteractions.SetHoldToggleToOpen(false);
		m_pRadialMenuInteractions.SetEntryPerformType(ERadialMenuPerformType.OnPressPerformInput);
		m_pRadialMenuInteractions.SetHandling(GetGame().GetPlayerController(), false, "MapContextualMenu");
		m_sBackAction = "RadialBack";
		m_pRadialMenuInteractions.SetHandlingPaging();
		SetEvenlyPlacedEntries(true);
		SetEntryDistance(45);
		SetEntryOffset(0, 0);
		SetShowEmptyEntries(true);
		SetUseCategories(true);
		SetRadialMenuSelectionBehavior(ERadialMenuSelectionBehavior.StickToLastSection);
		SetSelectFreeDelay(0.250);
	}
};
