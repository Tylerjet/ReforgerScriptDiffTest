/*
Component for a line in scenario browser.
*/
class SCR_ContentBrowser_ScenarioLineComponent : ScriptedWidgetComponent
{	
	protected ref SCR_ContentBrowser_ScenarioLineWidgets m_Widgets = new SCR_ContentBrowser_ScenarioLineWidgets;

	protected MissionWorkshopItem m_Mission;
	
	protected Widget m_wRoot;
	
	// Called when scenario state is changed through this component
	ref ScriptInvoker m_OnScenarioStateChanged = new ScriptInvoker; // (SCR_ContentBrowser_ScenarioLineComponent comp)
	
	//-----------------------------------------------------------------------------------
	static SCR_ContentBrowser_ScenarioLineComponent FindComponent(Widget w)
	{
		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}
	
	//-----------------------------------------------------------------------------------
	//! Must be called when the scenario properties have been updated (set/clear favourite or similar)
	void NotifyScenarioUpdate()
	{
		UpdateAllWidgets();
	}
	
	//-----------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem mission)
	{
		m_Mission = mission;
		
		UpdateAllWidgets();
	}
	
	
	//-----------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Mission;
	}
	
	//-----------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//-----------------------------------------------------------------------------------
	void ShowFavouriteButton(bool show)
	{
		m_Widgets.m_FavouriteButtonOverlay.SetVisible(show);
	}
	
	//-----------------------------------------------------------------------------------
	void ShowName(bool show)
	{
		m_Widgets.m_NameText.SetVisible(show);
	}
	
	//-----------------------------------------------------------------------------------
	void ShowPlayerCount(bool show)
	{
		m_Widgets.m_PlayerCount.SetVisible(show);
	}
	
	//-----------------------------------------------------------------------------------
	void ShowLastPlayedText(bool show)
	{
		m_Widgets.m_LastPlayedText.SetVisible(show);
	}
	
	//-----------------------------------------------------------------------------------
	void ShowSource(bool show)
	{
		m_Widgets.m_Source.SetVisible(show);
	}
	
	
	// ----------------------------- Protected / Private ---------------------------------
		
	
	//-----------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!m_Mission)
			return;
		
		// Fav button
		if (m_Mission)
			m_Widgets.m_FavouriteButtonComponent.SetToggled(m_Mission.IsFavorite(), false);
		
		// Name
		m_Widgets.m_NameText.SetText(m_Mission.Name());
		
		// Game mode
		/*
		// Not supported yet - todo
		*/
		
		// Type/player count
		{
			int playerCount = m_Mission.GetPlayerCount();
			bool mp = playerCount > 1;
			bool sp = !mp;
			
			m_Widgets.m_SinglePlayerImage.SetVisible(sp);
			m_Widgets.m_MultiPlayerImage.SetVisible(mp);
			m_Widgets.m_PlayerCountText.SetVisible(mp);
			m_Widgets.m_PlayerCountSeparator.SetVisible(mp && sp); // Visible only for both SP and SP scenario - it's a separator between images
			if (mp)
				m_Widgets.m_PlayerCountText.SetText(playerCount.ToString());
		}
		
		// Last played time text
		{
			int timeSinceLastPlayedSeconds = m_Mission.GetTimeSinceLastPlay();
			string timeSinceLastPlayText;
			if (timeSinceLastPlayedSeconds > 0 )
				timeSinceLastPlayText = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);
			else
				timeSinceLastPlayText = "-";
			m_Widgets.m_LastPlayedText.SetText(timeSinceLastPlayText);
		}
		
		// Source addon
		{
			WorkshopItem sourceAddon = m_Mission.GetOwner();
			m_Widgets.m_SourceImageOfficial.SetVisible(sourceAddon == null);
			m_Widgets.m_SourceImageCommunity.SetVisible(sourceAddon != null);
			m_Widgets.m_SourceNameTextOfficial.SetVisible(sourceAddon == null);
			m_Widgets.m_SourceNameTextCommunity.SetVisible(sourceAddon != null);
			string sourceAddonText = string.Empty;
			if (sourceAddon)
			{
				m_Widgets.m_SourceNameTextCommunity.SetText(sourceAddon.Name());
			}
			else
			{
				m_Widgets.m_SourceNameTextOfficial.SetText("#AR-Editor_Attribute_OverlayLogo_Reforger");				
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);
		
		// Init event handlers
		m_Widgets.m_FavouriteButtonComponent.m_OnToggled.Insert(OnFavouriteButtonMouseToggled);
	}
	
	
	//-----------------------------------------------------------------------------------
	void OnFavouriteButtonMouseToggled(SCR_ModularButtonComponent comp, bool newToggled)
	{
		if (!m_Mission)
			return;
		
		m_Mission.SetFavorite(newToggled);
		
		m_OnScenarioStateChanged.Invoke(this);
	}	
};