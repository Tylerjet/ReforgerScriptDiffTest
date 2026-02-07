class SCR_BlockedUsersDialogEntryUIComponent : SCR_ScriptedWidgetComponent
{	
	[Attribute("m_PlayerName")]
	protected string m_sPlayerNameWidgetName;
	
	[Attribute("m_PlatformIcon")]
	protected string m_sPlatformIconWidgetName;
	
	protected RichTextWidget m_wPlayerName;
	protected ImageWidget m_wPlatfromIcon;
	
	protected string m_sPlayerName;
	protected PlatformKind m_ePlayerPlatform;
	
	protected SCR_ModularButtonComponent m_Button;
	
	//Debug variable
	bool m_bDebugIsPlaystation;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wPlayerName = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sPlayerNameWidgetName));
		m_wPlatfromIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sPlatformIconWidgetName));			
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show the name of the player
	void SetPlayerName(string name)
	{
		if (!m_wPlayerName)
			return;
		
		m_wPlayerName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show the players platform
	void SetPlatfrom(PlatformKind platform)
	{
		if (!m_wPlatfromIcon)
			return;
		
		if (platform == PlatformKind.PSN)
			m_wPlatfromIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME);
		else
			m_wPlatfromIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME);
	}
}