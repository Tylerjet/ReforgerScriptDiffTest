//------------------------------------------------------------------------------------------------
//! Quickmenu element data class for display in QuickMenu
class SCR_QuickMenu_DisplayElement : Managed
{
	const int DEFAULT_PADDING = 4; 		// px, padding between elements (from each side)
	const ResourceName ELEMENT_LAYOUT = "{82443F96355D1C70}UI/layouts/HUD/QuickMenu/QuickMenu_ElementVON.layout";
		
	protected Widget m_wElement;
	protected Widget m_wElementRoot;
	protected ImageWidget m_wBackground;
	protected ImageWidget m_wFrame;
	protected ImageWidget m_wIcon;
	protected TextWidget m_wText;

	//------------------------------------------------------------------------------------------------
	//! Set text 
	//! \param text is the subject
	void SetText(string text)
	{
		m_wText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Set visibility
	//! \param state is the target state
	void SetVisible(bool state)
	{
		m_wElementRoot.SetVisible(state);
	}
		
	//------------------------------------------------------------------------------------------------
	//! Initial element setup
	//! \param entry is VONEntry which is being displayed
	void Init(SCR_VONEntry entry)
	{					
		m_wIcon.SetVisible(true);
		m_wIcon.LoadImageTexture(0, entry.GetIconResource());
		m_wIcon.SetImage(0);
		m_wText.SetText(entry.GetDisplayText());	
		
		if (entry.GetVONMethod() == ECommMethod.DIRECT)
			m_wIcon.SetColor(Color.White);
		else 
			m_wIcon.SetColor(Color.Gray75);
	}

	//------------------------------------------------------------------------------------------------
	//! Update visual state which may change during the widget lifetime
	//! \param isSelected is the current selection state
	void Update(bool isSelected)
	{
		if (isSelected)
			m_wFrame.SetVisible(true);
		else 
			m_wFrame.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_QuickMenu_DisplayElement(Widget displayRoot)
	{
		m_wElementRoot = GetGame().GetWorkspace().CreateWidgets(ELEMENT_LAYOUT, displayRoot.FindAnyWidget("MenuElements"));
		LayoutSlot.SetPadding(m_wElementRoot, DEFAULT_PADDING, 0, DEFAULT_PADDING, 0);

		m_wElement = m_wElementRoot.FindAnyWidget("Element");
		m_wBackground = ImageWidget.Cast(m_wElement.FindAnyWidget("Background"));
		m_wFrame = ImageWidget.Cast(m_wElement.FindAnyWidget("Frame"));
		m_wIcon = ImageWidget.Cast(m_wElement.FindAnyWidget("Icon"));
		m_wText = TextWidget.Cast(m_wElement.FindAnyWidget("Text"));
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_QuickMenu_DisplayElement()
	{
		if (m_wElementRoot)
		{
			m_wElementRoot.RemoveFromHierarchy();
			m_wElementRoot = null;
		}
	}
};