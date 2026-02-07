/*
Selsection menu entry widget component for displaying 3d rendered objects
*/

//------------------------------------------------------------------------------------------------
class SCR_SelectionMenuEntryPreviewComponent : SCR_SelectionMenuEntryComponent
{
	[Attribute("ItemPreview")]
	protected string m_sPreviewItem;
	
	[Attribute("FallbackIcon")]
	protected string m_sFallbackIcon;
	
	protected ItemPreviewWidget m_wPreviewItem;
	protected ImageWidget m_wFallbackIcon;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wPreviewItem = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget(m_sPreviewItem));
		m_wFallbackIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sFallbackIcon));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepare preview item and render given entity 
	void SetPreviewItem(IEntity item)
	{
		if (!m_wPreviewItem || ! m_wFallbackIcon)
		{
			Print("[SCR_SelectionMenuEntryPreviewComponent] - missing widgets!", LogLevel.DEBUG);
			return;
		}
		
		// Visibility 
		m_wPreviewItem.SetVisible(item != null);
		m_wFallbackIcon.SetVisible(!item);
		
		if (!item)
			return;

		// Get manager and render preview 
		ItemPreviewManagerEntity manager = GetGame().GetItemPreviewManager();
		if (!manager)
			return;
		
		// Set rendering and preview properties 
		manager.SetPreviewItem(m_wPreviewItem, item);
		m_wPreviewItem.SetResolutionScale(1, 1);
	}
}