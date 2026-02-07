//------------------------------------------------------------------------------------------------
class SCR_AccountWidgetComponent : ScriptedWidgetComponent 
{
	[Attribute("0 0 1 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorOnline;

	[Attribute("1 1 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorLocal;

	[Attribute("1 0 0 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorOffline;

	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker)]
	protected ref Color m_ColorWorking;

	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sTextureConnect;
	
	[Attribute("down")]
	protected string m_sImageConnect;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sTextureDisconnect;
	
	[Attribute("up")]
	protected string m_sImageDisconnect;
	
	[Attribute("HoverOverlay")]
	protected string m_sHoverOverlayName;
	
	[Attribute("Icon")]
	protected string m_sHoverIconName;

	protected Widget m_wRoot;
	protected Widget m_wStatusDot;
	protected Widget m_wConnectImage;
	protected TextWidget m_wLoginText;
	protected SCR_ButtonImageComponent m_News;
	protected SCR_ButtonImageComponent m_Profile;
	protected SCR_ButtonImageComponent m_Career;

	protected bool m_bLinked = true;
	protected bool m_bLoggedIn;
	protected bool m_bActive;
	protected BackendApi m_BackendApi;
	
	protected static ref array<SCR_AccountWidgetComponent> s_Instances;
	static const int AUTH_CHECK_PERIOD = 1000;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!s_Instances)
			s_Instances = {};
		s_Instances.Insert(this);

		m_wRoot = w;
		m_wLoginText = RichTextWidget.Cast(w.FindAnyWidget("PlatformPlayerName"));
		m_News = SCR_ButtonImageComponent.GetButtonImage("News", w);
		if (m_News)
			m_News.m_OnClicked.Insert(OnNews);
		
		/*m_Career = SCR_ButtonImageComponent.GetButtonImage("Career", w);
		if (m_Career)
			m_Career.m_OnClicked.Insert(OnCareer);*/

		m_Profile = SCR_ButtonImageComponent.GetButtonImage("Profile", w);
		if (m_Profile)
		{
			m_wConnectImage = m_Profile.GetRootWidget().FindAnyWidget(m_sHoverOverlayName);
			if (m_wConnectImage)
				m_wConnectImage.SetOpacity(0);
			
			m_Profile.m_OnClicked.Insert(OnProfile);
			m_wStatusDot = m_Profile.GetRootWidget().FindAnyWidget("Dot");
			SCR_EventHandlerComponent comp = SCR_EventHandlerComponent.Cast(m_Profile.GetRootWidget().FindHandler(SCR_EventHandlerComponent));
			if (comp)
			{
				comp.GetOnMouseEnter().Insert(OnProfileMouseOver);
				comp.GetOnMouseLeave().Insert(OnProfileMouseLeave);
			}
		}
		
		m_BackendApi = GetGame().GetBackendApi();
		UpdateAuthentication(true);
		UpdateNotifications(false);
		GetGame().GetCallqueue().CallLater(UpdateNotifications, 0, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (s_Instances)
			s_Instances.RemoveItem(this);
	}
	
	//------------------------------------------------------------------------------------------------
	static array<SCR_AccountWidgetComponent> GetInstances()
	{
		return s_Instances;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateNotifications(bool animate = true)
	{
		if (!m_News)
			return;
		
		// Updating notifications is disabled until the ability to set read articles is done
		SetNotificationNumber(m_News.GetRootWidget(), 0, animate);
		//SetNotificationNumber(m_News.GetRootWidget(), MainMenuUI.GetUnreadNewsCount(), animate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetConnectionIcon(bool connect)
	{
		if (!m_wConnectImage)
			return;
		
		ImageWidget image = ImageWidget.Cast(m_wConnectImage.FindAnyWidget(m_sHoverIconName));
		if (!image)
			return;
		
		if (connect)
			SCR_WLibComponentBase.SetTexture(image, m_sTextureConnect, m_sImageConnect);
		else
			SCR_WLibComponentBase.SetTexture(image, m_sTextureDisconnect, m_sImageDisconnect);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAuthentication(bool periodicUpdate)
	{
		if (!m_BackendApi)
		{
			m_BackendApi = GetGame().GetBackendApi();
			if (!m_BackendApi)
				return;
		}

		if (!m_wStatusDot)
			return;
		
		Color color;
		if (m_BackendApi.IsAuthInProgress())
		{
			color = m_ColorWorking;
			m_bLoggedIn = false;
		}
		else if (m_BackendApi.IsAuthenticated())
		{
			color = m_ColorOnline;
			m_bLoggedIn = true;
		}
		else
		{
			color = m_ColorOffline;
			m_bLoggedIn = false;
		}
		
		SetLinked(m_BackendApi.IsBIAccountLinked());
		
		m_wStatusDot.SetColor(color);
		if (periodicUpdate)
			GetGame().GetCallqueue().CallLater(UpdateAuthentication, AUTH_CHECK_PERIOD, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNews()
	{
		OpenProfileMenu(0);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnCareer()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CareerProfileMenu);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommunity()
	{
		OpenProfileMenu(1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnProfile()
	{
		// Show logout screen
		if (m_BackendApi.IsBIAccountLinked())
		{
			LogoutDialogUI dialog = LogoutDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.LogoutDialog));
			if (dialog)
				dialog.m_OnConfirm.Insert(OnLogout);
			return;
		}
		
#ifdef PLATFORM_CONSOLE	
		LoginDialogUI dialog = LoginDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.LoginDialogConsole));
#else
		LoginDialogUI dialog = LoginDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.LoginDialog));
#endif
		if (!dialog)
			return;
		
		dialog.m_OnDialogClosed.Insert(OnAccountDialogClosed);
		dialog.m_OnLogin.Insert(OnLogin);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAccountDialogClosed()
	{
		if (!m_BackendApi)
			return;
	}

	//------------------------------------------------------------------------------------------------
	//! Open profile menu on page: 0 - news, 1 - community, 2 - profile setup
	protected void OpenProfileMenu(int page)
	{
		SCR_ProfileSuperMenu menu = SCR_ProfileSuperMenu.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ProfileSuperMenu, 0, true, false));
		if (menu)
			menu.SetPage(page);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetUnreadCount(array<ref SCR_NewsEntry> entries)
	{
		int count;
		if (!entries)
			return count;
		
		foreach (SCR_NewsEntry entry : entries)
		{
			if (!entry.m_bRead)
				count++;
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetNotificationNumber(Widget parent, int number, bool animate = true)
	{
		if (!parent)
			return;

		TextWidget text = TextWidget.Cast(parent.FindAnyWidget("Number"));
		if (!text)
			return;

		text.SetText(number.ToString());
		if (animate)
			AnimateWidget.Opacity(text.GetParent(), number > 0, UIConstants.FADE_RATE_FAST);
		else
			text.GetParent().SetVisible(number > 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLogin()
	{
		SetLinked(m_BackendApi.IsBIAccountLinked());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLogout()
	{
		SetLinked(m_BackendApi.IsBIAccountLinked());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnProfileMouseOver(Widget button)
	{
		if (m_BackendApi.IsBIAccountLinked())
			AnimateWidget.Opacity(m_wConnectImage, 1, UIConstants.FADE_RATE_FAST);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnProfileMouseLeave(Widget button)
	{
		if (m_BackendApi.IsBIAccountLinked())
			AnimateWidget.Opacity(m_wConnectImage, 0, UIConstants.FADE_RATE_FAST);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetLinked(bool linked)
	{
		if (linked == m_bLinked)
			return;
		m_bLinked = linked;
		
		SetConnectionIcon(!linked);
		
		if (!linked || GetGame().GetWorkspace().GetFocusedWidget() != m_Profile.GetRootWidget())
			AnimateWidget.Opacity(m_wConnectImage, !linked, UIConstants.FADE_RATE_FAST);
	}
};