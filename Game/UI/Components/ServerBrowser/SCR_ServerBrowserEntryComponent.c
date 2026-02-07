// This component handles server entry and visiualization of server data
//
//------------------------------------------------------------------------------------------------

class SCR_ServerBrowserEntryComponent : SCR_ScriptedWidgetComponent
{
	// Widget names
	const string WIDGET_FAVORITE_BUTTON = "FavButton";
	const string WIDGET_CONTENT = "Content";
	const string WIDGET_HORIZONTAL_PROPERTIES = "HPropertyImages";
	const string WIDGET_PROPERTIES_MODS = "HPropertyImages_Mods";
	const string WIDGET_PROPERTIES_PASSWORD = "HPropertyImages_Password";
	const string WIDGET_IMAGE_PING = "ImgPing";

	const string WIDGET_BACKGROUND = "Background";
	const string WIDGET_BACKGROUND_EMPTY = "BackgroundEmpty";
	const string WIDGET_HORIZONTAL_CONTENT = "HorizontalLayout";
	const string WIDGET_LOADING = "Loading";

	// Strings
	const string STRING_COUNT_UNLIMITED = "#AR-ServerBrowser_CountUnlimited";

	// Numbers
	const int SERVER_NAME_LENGTH = 60;
	const int SIZE_PROPERTY_IMAGE = 32;
	const int PING_LIMIT = 999;

	const string SERVER_NAME_ENDING = "...";

	// Base
	protected Room m_RoomInfo;

	protected SCR_ModularButtonComponent m_ModularButton = null;

	// Server properties images
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Image - texture or imageset", "edds imageset")]
	protected ResourceName m_PropertiesImageSet;

	protected Widget m_wProperties;
	protected Widget m_wPropertiesMods;
	protected Widget m_wPropertiesPassword;

	// Properties
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref ServerBrowserEntryProperty> m_aPropertyImages;

	protected ref array<ImageWidget> m_aPropertyWidgets = {};

	protected SCR_EServerEntryProperty m_iProperties;

	// Ping
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref ServerBrowserEntryProperty> m_aPingStates;

	// Backrounds and wrappers
	protected Widget m_wBackground;
	protected Widget m_wBackgroundEmpty;
	protected Widget m_wHorizontalContent;
	protected Widget m_wLoading;

	// Favorite widges and behavior
	protected bool m_bFavoritingAnimationEnabled = true;
	protected bool m_bIsFavorite;
	protected Widget m_wFavorite;
	protected Widget m_wFavoriteImage;

	protected ImageWidget m_wImgPing;
	protected int m_iPingLimit = 0;

	protected bool m_bIsModded = false;

	protected ref SCR_ButtonComponent m_FavComponent;

	ref ScriptInvoker Event_OnFocusEnter = new ScriptInvoker();
	ref ScriptInvoker Event_OnFocusLeave = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseEnter = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();

	ref ScriptInvoker m_OnFavorite = new ScriptInvoker();

	// Text scrolling anims
	SCR_HorizontalScrollAnimationComponent m_NameScrollAnim;
	SCR_HorizontalScrollAnimationComponent m_ScenarioScrollAnim;

	protected bool m_bInnerButtonInteraction = false;

	//------------------------------------------------------------------------------------------------
	// Override API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;

		super.HandlerAttached(w);

		// Get wrappers
		m_wBackground = m_wRoot.FindAnyWidget(WIDGET_BACKGROUND);
		m_wBackgroundEmpty = m_wRoot.FindAnyWidget(WIDGET_BACKGROUND_EMPTY);
		m_wHorizontalContent = m_wRoot.FindAnyWidget(WIDGET_HORIZONTAL_CONTENT);
		m_wLoading = m_wRoot.FindAnyWidget(WIDGET_LOADING);

		// Call init
		Init(w);

		// Disabled opacity
		if (!w.IsEnabled())
			w.SetOpacity(UIConstants.DISABLED_WIDGET_OPACITY);

		// Find widgets
		m_wProperties = w.FindAnyWidget(WIDGET_HORIZONTAL_PROPERTIES);
		m_wPropertiesMods = w.FindAnyWidget(WIDGET_PROPERTIES_MODS);
		m_wPropertiesPassword = w.FindAnyWidget(WIDGET_PROPERTIES_PASSWORD);
		m_wImgPing = ImageWidget.Cast(w.FindAnyWidget(WIDGET_IMAGE_PING));

		// Name
		Widget frameName = m_wRoot.FindAnyWidget("FrameName");
		if (frameName)
		{
			m_NameScrollAnim = SCR_HorizontalScrollAnimationComponent.Cast(
				frameName.FindHandler(SCR_HorizontalScrollAnimationComponent)
			);
		}

		// Scenario
		Widget frameScenario = m_wRoot.FindAnyWidget("FrameScenario");
		if (frameScenario)
		{
			m_ScenarioScrollAnim = SCR_HorizontalScrollAnimationComponent.Cast(
				frameScenario.FindHandler(SCR_HorizontalScrollAnimationComponent)
			);
		}

		// Setup scroll animations
		EnableTextAnimations(false);

		m_wLoading.SetVisible(false);

		// Get highest ping
		for (int i = 0, count = m_aPingStates.Count(); i < count; i++)
		{
			int ping = m_aPingStates[i].m_sValue.ToInt();

			if (m_iPingLimit < ping)
				m_iPingLimit = ping;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		if (!m_ModularButton)
			m_ModularButton = SCR_ModularButtonComponent.Cast(m_wRoot.FindHandler(SCR_ModularButtonComponent));

		if (m_ModularButton && !m_ModularButton.GetIsFocusOnMouseEnter())
			m_OnMouseEnter.Invoke(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		if (m_ModularButton && !m_ModularButton.GetIsFocusOnMouseEnter())
			m_OnMouseLeave.Invoke(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// LMB check
		if (button != 0 || m_bInnerButtonInteraction)
			return false;

		return super.OnClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		// LMB check
		if (button != 0 || m_bInnerButtonInteraction)
			return false;

		return super.OnDoubleClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		Event_OnFocusEnter.Invoke(this);

		if (!m_NameScrollAnim)
			return false;

		// Set text animations
		EnableTextAnimations(true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		Event_OnFocusLeave.Invoke(this);

		// Stop anim
		EnableTextAnimations(false);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		Event_OnFocusEnter.Clear();
		Event_OnFocusLeave.Clear();
	}


	//------------------------------------------------------------------------------------------------
	// Public API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Call this on initialization to setup widget
	void Init(Widget w)
	{
		// Favourite reference
		Widget m_wFavoriteButton = w.FindAnyWidget(WIDGET_FAVORITE_BUTTON);

		// Setup favorite button
		if (m_wFavoriteButton)
		{
			m_FavComponent = SCR_ButtonComponent.Cast(m_wFavoriteButton.FindHandler(SCR_ButtonComponent));
			m_wFavoriteImage = m_wFavoriteButton.FindAnyWidget(WIDGET_CONTENT);

			m_wFavoriteButton.SetEnabled(true);
		}

		// Setup invoker actions
		if (!m_FavComponent)
			return;

		m_FavComponent.m_OnClicked.Insert(OnFavoriteClicked);
		m_FavComponent.m_OnHover.Insert(OnFavoriteHover);
		m_FavComponent.m_OnHoverLeave.Insert(OnFavoriteLeave);
	}

	//------------------------------------------------------------------------------------------------
	//! Set room and display room info in entry
	void SetRoomInfo(Room room)
	{
		m_RoomInfo = room;

		// Visualize as empty?
		if (!m_RoomInfo)
			return;

		// Disable if not joinable
		EnableEntry(room.Joinable());

		// Name
		SetCellText("Name", m_RoomInfo.Name());

		// Scenario
		SetCellText("Scenario", m_RoomInfo.ScenarioName());

		// Player count
		string playerCount = m_RoomInfo.PlayerCount().ToString();
		string playerCountMax = m_RoomInfo.PlayerLimit().ToString();

		SetCellText("Players", playerCount + "/" + playerCountMax);

		// Check mods
		m_bIsModded = room.IsModded();

		// Setup room properties
		CheckRoomProperties();
		DisplayServerProperties();

		// Favorite
		SetFavorite(m_RoomInfo.IsFavorite(), false);

		// Ping
		DisplayPing(room.GetPing());
	}

	//------------------------------------------------------------------------------------------------
	void SetFavorite(bool bFavorite, bool callback)
	{
		if (!m_wFavoriteImage)
			return;

		m_bIsFavorite = bFavorite;

		// Play star fade animation
		if (m_bFavoritingAnimationEnabled)
		{
			if (m_bIsFavorite)
				AnimateWidget.Color(m_wFavoriteImage, UIColors.CONTRAST_COLOR, UIConstants.FADE_RATE_FAST);
			else
				AnimateWidget.Color(m_wFavoriteImage, UIColors.LIGHT_GREY, UIConstants.FADE_RATE_FAST);
		}

		if (callback)
			m_OnFavorite.Invoke(this, bFavorite);
	}

	//------------------------------------------------------------------------------------------------
	void OnFavoriteClicked(SCR_ButtonBaseComponent button)
	{
		SetFavorite(!m_bIsFavorite, true);
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
	}

	//------------------------------------------------------------------------------------------------
	//! Animate whole widget opacity
	void AnimateOpacity(int delay, float animationTime, float opacityEnd, float opacityStart = -1)
	{
		if (opacityStart != -1)
			GetRootWidget().SetOpacity(opacityStart);

		GetGame().GetCallqueue().Remove(OpacityAnimation);
		GetGame().GetCallqueue().CallLater(OpacityAnimation, delay, false, animationTime, opacityEnd);
	}

	//------------------------------------------------------------------------------------------------
	// Protected API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void OnFavoriteHover()
	{
		m_bInnerButtonInteraction = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavoriteLeave(Widget w = null)
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() == m_wRoot && !w)
			GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);

		m_bInnerButtonInteraction = false
	}

	//------------------------------------------------------------------------------------------------
	//! Set text in cell by it's widget name
	protected void SetCellText(string cellName, string str)
	{
		Widget wCell = m_wRoot.FindAnyWidget(cellName);
		if (!wCell)
			return;

		TextWidget wText = TextWidget.Cast(wCell.FindAnyWidget(WIDGET_CONTENT));
		if (wText)
			wText.SetText(str);
	}

	//------------------------------------------------------------------------------------------------
	//! Display number of current ping and add icon and color
	protected void DisplayPing(int ping)
	{
		float lastHighest = m_iPingLimit + 1;
		ServerBrowserEntryProperty displayState;

		// No ping state
		if (ping == 0)
		{
			if (m_wImgPing)
				m_wImgPing.SetVisible(false);

			return;
		}

		// Go through available icons
		foreach (ServerBrowserEntryProperty pingState : m_aPingStates)
		{
			float pingFromStr = pingState.m_sValue.ToFloat();

			// Find smallest ping
			if (ping < pingFromStr && pingFromStr < lastHighest)
			{
				displayState = pingState;
				lastHighest = pingFromStr;
			}
		}

		// Set ping text
		string strPing = Math.Floor(ping).ToString();

		// Over limit
		if (ping > m_iPingLimit || ping < 0)
			displayState = m_aPingStates[m_aPingStates.Count() - 1];

		if (ping > PING_LIMIT)
			strPing = PING_LIMIT.ToString() + "#ENF-ComboModifier";

		SetCellText("Ping", strPing);

		// Set ping icon
		if (displayState && m_wImgPing)
		{
			m_wImgPing.SetVisible(true);
			m_wImgPing.LoadImageFromSet(0, m_PropertiesImageSet, displayState.m_sImageName);
			m_wImgPing.SetColor(displayState.m_Color);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set button visuals and behavior
	void EmptyVisuals(bool enable)
	{
		//m_wRoot.SetEnabled(!enable);

		// Set widgets
		//m_wBackground.SetVisible(!enable);
		//m_wBackgroundEmpty.SetVisible(enable);
		m_wHorizontalContent.SetVisible(!enable);
		m_wLoading.SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpacityAnimation(int time, float opacityEnd)
	{
		AnimateWidget.Opacity(GetRootWidget(), opacityEnd, time);
	}

	//------------------------------------------------------------------------------------------------
	//! Check properties and assign state of room
	protected void CheckRoomProperties()
	{
		m_iProperties = 0;

		// Client versions missmatch
		bool wrongVersion = m_RoomInfo.GameVersion() != GetGame().GetBuildVersion();
		bool restrictedUGC = m_bIsModded && !SCR_AddonManager.GetInstance().GetUgcPrivilege();

		if (wrongVersion || restrictedUGC)
			m_iProperties |= SCR_EServerEntryProperty.VERSION_MISMATCH;

		// Locked with password
		if (m_RoomInfo.PasswordProtected())
			m_iProperties |= SCR_EServerEntryProperty.PASSWORD_PROTECTED;

		// Crossplay
		if (m_RoomInfo.IsCrossPlatform())
			m_iProperties |= SCR_EServerEntryProperty.CROSS_PLATFORM;

		// LAN - TODO@wernerjak - add local network check
		// m_iProperties |= SCR_EServerEntryProperty.LAN;

		// MODDED
		if (m_bIsModded)
			m_iProperties |= SCR_EServerEntryProperty.MODDED;
	}

	//------------------------------------------------------------------------------------------------
	// Create property images based on room setup
	protected void DisplayServerProperties()
	{
		// Remove previous
		foreach (Widget w : m_aPropertyWidgets)
		{
			w.RemoveFromHierarchy();
		}

		m_aPropertyWidgets.Clear();

		// Check images
		if (!m_aPropertyImages || m_aPropertyImages.IsEmpty())
			return;

		// Check states and create images
		ImageWidget imgProperty;
		Widget container = m_wProperties;

		foreach (ServerBrowserEntryProperty property : m_aPropertyImages)
		{
			if (!(m_iProperties & property.m_iPropertyState))
				continue;

			if (property.m_iPropertyState == SCR_EServerEntryProperty.PASSWORD_PROTECTED) // Locked with password
				container = m_wPropertiesPassword;
			else if (property.m_iPropertyState == SCR_EServerEntryProperty.MODDED) // Modded
				container = m_wPropertiesMods;
			else // As part of the refactoring we only want Locked and Modded icons
				continue;

			// Create image
			imgProperty = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidget(
				WidgetType.ImageWidgetTypeID, WidgetFlags.BLEND | WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.NOWRAP, property.m_Color, 0, container
			));

			// Setup image
			imgProperty.LoadImageFromSet(0, m_PropertiesImageSet, property.m_sImageName);
			imgProperty.SetSize(SIZE_PROPERTY_IMAGE, SIZE_PROPERTY_IMAGE);
			imgProperty.SetFlags(WidgetFlags.IGNORE_CURSOR);

			// Cache in list
			m_aPropertyWidgets.Insert(imgProperty);
		}

		// Turn favorites button into warning icon
		if (m_iProperties & SCR_EServerEntryProperty.VERSION_MISMATCH)
		{
			EnableFavoritingAnimation(false);

			ImageWidget favImage = ImageWidget.Cast(m_wFavoriteImage);
			if (favImage)
			{
				favImage.LoadImageFromSet(0, m_PropertiesImageSet, "warning");
				favImage.SetColor(UIColors.WARNING);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Apply behavior on all potentially longer entry text
	protected void EnableTextAnimations(bool enable)
	{
		HandleTextAnimation(m_NameScrollAnim, enable);
		HandleTextAnimation(m_ScenarioScrollAnim, enable);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle animation enabled-disabling
	//! enable = false restarts positions
	protected void HandleTextAnimation(SCR_HorizontalScrollAnimationComponent anim, bool enable)
	{
		if (!anim)
			return;

		if (enable)
		{
			if (!anim.GetContentFit())
			{
				anim.AnimationStart();
			}
			else
			{
				anim.AnimationStop();
				anim.ResetPosition();
			}

			return;
		}

		// Disabled
		anim.AnimationStop();
		anim.ResetPosition();
	}

	//------------------------------------------------------------------------------------------------
	protected void EnableEntry(bool enable)
	{
		if (enable)
		{
			GetRootWidget().SetOpacity(1);
		}
		else
		{
			GetRootWidget().SetOpacity(0.5);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Get & Set API
	//------------------------------------------------------------------------------------------------
	Room GetRoomInfo()
	{
		return m_RoomInfo;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsModded()
	{
		return m_bIsModded;
	}

	//------------------------------------------------------------------------------------------------
	void EnableFavoritingAnimation(bool enabled)
	{
		m_bFavoritingAnimationEnabled = enabled;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class ServerBrowserEntryProperty
{
	[Attribute("1", UIWidgets.ComboBox, "", category:"Selector", ParamEnumArray.FromEnum(SCR_EServerEntryProperty))]
	SCR_EServerEntryProperty m_iPropertyState;

	[Attribute("", UIWidgets.EditBox, desc: "Image name to load")]
	string m_sImageName;

	[Attribute("", UIWidgets.EditBox, desc: "String value of property")]
	string m_sValue;

	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_Color;
};

//------------------------------------------------------------------------------------------------
enum SCR_EServerEntryProperty
{
	VERSION_MISMATCH = 1<<0,
	PASSWORD_PROTECTED = 1<<1,
	CROSS_PLATFORM = 1<<2,
	LAN = 1<<3,
	MODDED = 1<<4,
};
