//------------------------------------------------------------------------------------------------
class SCR_SelectionMenuEntryIconComponent : SCR_SelectionMenuEntryComponent
{
	[Attribute("Size")]
	protected string m_sSizeLayout;
	
	[Attribute("Icon")]
	protected string m_sIcon;
	
	protected Widget m_wSizeLayout;
	protected ImageWidget m_wIcon;
	
	protected ResourceName m_sTexture;
	protected string m_sImage;
	
	protected float m_fOriginalSize;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wSizeLayout = m_wRoot.FindAnyWidget(m_sSizeLayout);
		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sIcon));
		
		m_fOriginalSize = GetLayoutSize();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry holding data driving this visuals
	override void SetEntry(SCR_SelectionMenuEntry entry)
	{
		// Clear previous referenced entry
		if (m_Entry)
		{
			m_Entry.GetOnIconChange().Remove(OnEntryIconChange);
		}
		
		// Setup new entry
		super.SetEntry(entry);
		
		if (!entry)
			return;
		
		m_Entry.GetOnIconChange().Insert(OnEntryIconChange);
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName texture, string image)
	{
		m_sTexture = texture;
		m_sImage = image;
		
		Print("m_sTexture: " + m_sTexture + ", m_sImage: " + m_sImage);
		SCR_WLibComponentBase.SetTexture(m_wIcon, m_sTexture, m_sImage);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLayoutSize(float size)
	{
		if (m_wSizeLayout)
			FrameSlot.SetSize(m_wSizeLayout, size, size);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLayoutSize()
	{
		if (!m_wSizeLayout)
			return -1;
		
		return FrameSlot.GetSize(m_wSizeLayout)[0];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetOriginalSize()
	{
		return m_fOriginalSize;
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryIconChange(SCR_SelectionMenuEntry entry, ResourceName texture, string image)
	{
		SetImage(texture, image);
	}
}