/*
Component to be attached to addon lines.
*/

//----------------------------------------------------------------------------------------------
class SCR_WorkshopAddonLineComponent : SCR_AddonLineBaseComponent
{
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		bool result = super.OnClick(w, x, y, button);
		OnOpenDetailsButton();

		return result;
	}
	
	//----------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		super.HandlerDeattached(w);
		
		if (m_Item)
			m_Item.m_OnChanged.Remove(UpdateAllWidgets);
	}
	
	//----------------------------------------------------------------------------------------------
	override void Init(SCR_WorkshopItem item)
	{
		super.Init(item);
		
		// OnChanged is called whenever something happens with the workshop item.
		// Simplest approach is to refresh whole line associated with the workshop item on this event.
		item.m_OnChanged.Insert(UpdateAllWidgets);
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnDeleteButton()
	{
		super.OnDeleteButton();
		
		SCR_DeleteAddonDialog.CreateDeleteAddon(m_Item);
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnActionButton()
	{
		super.OnActionButton();
		
		if (!m_Item)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnUpdateButton()
	{
		super.OnUpdateButton();
		
		if (!m_Item)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//----------------------------------------------------------------------------------------------
	override void UpdateAllWidgets()
	{	
		if (m_bCanUpdate && m_Item)
			HandleEnableButtons(m_Item.GetEnabled());
		
		super.UpdateAllWidgets();
	}
	
	//----------------------------------------------------------------------------------------------
	bool IsWorkshopItemEnabled()
	{
		if (!m_Item)
			return false;
		
		return m_Item.GetEnabled();
	}
}