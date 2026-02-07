//------------------------------------------------------------------------------------------------

class ScenarioDetailsMenu: ChimeraMenuBase
{
	protected EGameFlags m_eFocusedGameflags;
	protected SCR_MissionHeader m_Header;
	protected TextWidget m_ScenarioNameText;
	protected TextWidget m_ScenarioDescription;
	protected ImageWidget m_ScenarioImage;
	protected Widget m_wRoot;
	protected Widget m_wPlayButton;
	protected SCR_ButtonTextComponent m_PlayButtonComponent;
	protected Widget m_wHostButton;
	
	protected SCR_ButtonTextComponent m_HostButtonComponent;
	protected SCR_NavigationButtonComponent m_BackButtonComponent;
	protected Widget m_wFindServersButton;
	protected SCR_ButtonTextComponent m_FindServersButtonComponent;
	private InputManager m_InputManager;
	
	protected ref array<ref SCR_MissionHeader> m_aGridHeaders = new array<ref SCR_MissionHeader>(); 
	protected ref array<string> m_aMissionFolderPaths = new ref array<string>;
	protected ref array<string> m_aMissionPaths = new ref array<string>;
	
	override void OnMenuOpen()
	{
		
		m_wRoot = GetRootWidget();
		if (!m_wRoot)
			return;
		
		m_InputManager = GetGame().GetInputManager();
		//m_InputManager.AddActionListener("MenuBack", EActionTrigger.PRESSED, Action_MenuBack);
		//m_InputManager.AddActionListener("MenuSelect", EActionTrigger.PRESSED, Action_OnMenuSelect);
		m_ScenarioNameText = TextWidget.Cast(m_wRoot.FindAnyWidget("Title"));
		m_ScenarioDescription = TextWidget.Cast(m_wRoot.FindAnyWidget("ModDescription"));
		m_ScenarioImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("ScenarioImage"));
		m_wPlayButton = m_wRoot.FindAnyWidget("PlayMainButton");
		if (m_wPlayButton)
		{
			m_PlayButtonComponent = SCR_ButtonTextComponent.Cast(m_wPlayButton.FindHandler(SCR_ButtonTextComponent));
			if (m_PlayButtonComponent)
				m_PlayButtonComponent.m_OnClicked.Insert(Play);
		}
		
		m_wHostButton = m_wRoot.FindAnyWidget("HostMainButton");
		if (m_wHostButton)
		{
			m_HostButtonComponent = SCR_ButtonTextComponent.Cast(m_wHostButton.FindHandler(SCR_ButtonTextComponent));
			if (m_HostButtonComponent)
				m_HostButtonComponent.m_OnClicked.Insert(Host);
		}
		
		m_wFindServersButton = m_wRoot.FindAnyWidget("FindServersButton");
		if (m_wFindServersButton)
		{
			m_FindServersButtonComponent = SCR_ButtonTextComponent.Cast(m_wFindServersButton.FindHandler(SCR_ButtonTextComponent));
			if (m_FindServersButtonComponent)
				m_FindServersButtonComponent.m_OnClicked.Insert(JoinScenarioServer);
		}
		
		
		m_BackButtonComponent = SCR_NavigationButtonComponent.GetNavigationButtonComponent("Back", m_wRoot);
		if (m_BackButtonComponent)
			m_BackButtonComponent.m_OnActivated.Insert(Action_MenuBack);
		
		//m_sSectionName = "CONFLICT";
		m_aMissionPaths = {"{ECC61978EDCC2B5A}Missions/23_Campaign.conf"};
		m_aMissionFolderPaths = {};
		//m_bListAddons = false;
		
		if (m_aGridHeaders)
		{
			m_aGridHeaders.Clear();
			
			foreach (string path : m_aMissionPaths)
			{
				LoadMissionFile(path);
			}
			
			foreach (string path : m_aMissionFolderPaths)
			{
				LoadMissionFolder(path);
			}
		}
		
		SetScenario(m_aGridHeaders[0]);
		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScenario(SCR_MissionHeader scenario)
	{
		if (!scenario)
			return;
		
		m_Header = scenario;
		if (m_ScenarioNameText)
			m_ScenarioNameText.SetText("Conflict");
		if (m_ScenarioDescription)
			m_ScenarioDescription.SetText(scenario.m_sDescription);
		if (m_ScenarioImage)
		{
			m_ScenarioImage.LoadImageTexture(0, scenario.m_sIcon, false, true);
			m_ScenarioImage.SetImage(0);
		}
						//m_wImagePlaceholder.SetVisible(false);
						//m_wImageDetail.SetVisible(true);
		
		
	}	
	
	//------------------------------------------------------------------------------------------------
	void JoinScenarioServer()
	{
		
		ServerBrowserMenuUI menu = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu, 0, true, true));
		//if (menu)
			//menu.params.FilterByScenraioId(m_FocusedMissionHeader.m_owner.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	bool LoadMissionFile(string missionPath)
	{
		ref SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(missionPath));
		if (header && m_aGridHeaders.Find(header) == -1)
		{
			m_aGridHeaders.Insert(header);
			return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadMissionFolder(string folderPath)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void Play()
	{
		if (m_Header)
		{
			GetGame().SetGameFlags(m_eFocusedGameflags, false);
			
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.CLICK);
			
			WorkshopApi workshopApi = GetGame().GetBackendApi().GetWorkshop();
			if (workshopApi)
				workshopApi.Cleanup();
			
			if (!GameStateTransitions.RequestMissionChangeTransition(m_Header))
			{
				// Show error dialog?
				MenuManager menuManager = GetGame().GetMenuManager();
				if (menuManager)
				{
					//ShowInvalidMissionDialog();
				}
			}
			else
			{
				GetGame().GetMenuManager().CloseAllMenus();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Host()
	{
		if (m_Header)
		{
			GetGame().SetGameFlags(m_eFocusedGameflags, false);

			WorkshopApi workshopApi = GetGame().GetBackendApi().GetWorkshop();
			if (workshopApi)
				workshopApi.Cleanup();

			bool res = GameStateTransitions.RequestPublicServerTransition(m_Header, new ArmaReforgerServerParams(m_Header));
			
			if (res)
				GetGame().GetMenuManager().CloseAllMenus();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_MenuBack()
	{
		
		if (IsFocused())
		{
			//m_ParentMenu.SetFocusedItem(m_WorkshopItem);
			Close();
		}
			
	}
	

};
