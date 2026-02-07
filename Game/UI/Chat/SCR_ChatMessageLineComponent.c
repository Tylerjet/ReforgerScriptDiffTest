class SCR_ChatMessageLineComponent : ScriptedWidgetComponent
{
	protected ref SCR_ChatMessageLineWidgets m_Widgets = new SCR_ChatMessageLineWidgets();
	
	protected ResourceName CHAT_IMAGESET = "{1872FFA1133724A2}UI/Textures/Chat/chat.imageset";
	
	protected Widget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);
		SetEmptyMessage();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] msg
	//! \param[in] style
	void SetMessage(notnull SCR_ChatMessage msg, SCR_ChatMessageStyle style)
	{		
		// Enable elements which can be disabled for empty message lines
		m_Widgets.m_Badge.SetVisible(true);
		m_Widgets.m_TypeImage.SetVisible(true);
		
		SCR_ChatMessageGeneral messageGeneral = SCR_ChatMessageGeneral.Cast(msg);
		SCR_ChatMessagePrivate messagePrivate = SCR_ChatMessagePrivate.Cast(msg);
		SCR_ChatMessageRadioProtocol messageRadio = SCR_ChatMessageRadioProtocol.Cast(msg);
		
		Color channelColor = style.m_Color;
		string chatTypeImageName = style.m_sIconName;
		
		//---------------------------------------------------------------------------------
		// Common properties derived from the resolved style, regardless of message type
		
		// Message color
		m_Widgets.m_MessageText.SetColor(Color.FromInt(Color.WHITE)); // Message is always white now
		
		// Badge color
		Color badgeColor = Color.FromInt(Color.WHITE);
		if (style.m_bColoredBadge)
			badgeColor = channelColor;

		m_Widgets.m_Badge.SetColor(badgeColor);
		
		// Image color
		Color imageColor = Color.FromInt(Color.WHITE);
		if (style.m_bColoredIcon)
			imageColor = channelColor;

		m_Widgets.m_TypeImage.SetColor(imageColor);
		
		// Background color
		float bgAlphaOld = m_Widgets.m_BackgroundImage.GetColor().A();
		Color bgColorNew = Color.FromInt(Color.BLACK);
		if (style.m_bColoredBackground)
			bgColorNew = Color.FromInt(style.m_Color.PackToInt());

		bgColorNew.SetA(bgAlphaOld); // Keep the old alpha value
		m_Widgets.m_BackgroundImage.SetColor(bgColorNew);
		m_Widgets.m_BackgroundImage.SetVisible(true);
		
		//---------------------------------------------------------------------------------
		// Properties specific to message type
		
		// Set properties depending on channel class
		string senderNameText;
		if (messagePrivate)
			senderNameText = messagePrivate.m_sSenderName + ": ";
		else if (messageGeneral)
			senderNameText = messageGeneral.m_sSenderName + ": ";
		
		Color textColor = Color.FromInt(Color.WHITE);		
		
		// Player name color
		if (!senderNameText.IsEmpty() && style.m_bColoredPlayerName)
			textColor = style.m_Color;

		m_Widgets.m_TypeImage.LoadImageFromSet(0, CHAT_IMAGESET, chatTypeImageName);

		m_Widgets.m_MessageText.SetColor(textColor);		
		m_Widgets.m_MessageText.SetText(senderNameText + msg.m_sMessage);	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEmptyMessage()
	{
		// Hide elements, leave only text so that it constraints the height of this strip
		m_Widgets.m_Badge.SetVisible(false);
		m_Widgets.m_BackgroundImage.SetVisible(false);
		m_Widgets.m_TypeImage.SetVisible(false);
		m_Widgets.m_MessageText.SetText(" ");
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] visible
	void SetVisible(bool visible)
	{
		m_wRoot.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
}
