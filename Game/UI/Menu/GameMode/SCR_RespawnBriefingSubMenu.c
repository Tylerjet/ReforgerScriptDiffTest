class SCR_RespawnBriefingSubMenu : SCR_RespawnSubMenuBase
{
	[Attribute("TileSelection")]
	protected string m_sFullBriefingTop;
	
	[Attribute("Bottom")]
	protected string m_sFullBriefingButtom;

	[Attribute("DescriptionText")]
	protected string m_sSimpleBriefingDescriptionText;
	
	[Attribute("SimpleBriefingTop")]
	protected string m_sSimpleBriefingTop;
	
	[Attribute("SimpleBriefingBottom")]
	protected string m_sSimpleBriefingBottom;
	
	[Attribute("SimpleBriefingBackGroundImage")]
	protected string m_sSimpleBriefingBackGroundImage;
	
	[Attribute("SimpleBriefingIcon")]
	protected string m_sSimpleBriefingBackIconImage;
	
	[Attribute("#AR-PauseMenu_Continue", UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContinueButtonText;
	
	protected static const string FACTION_LAYOUT = "{2E728ED4D2D7875D}UI/layouts/Menus/RoleSelection/BriefingFaction.layout";
	protected static const string VICTORY_CONDITION_LAYOUT = "{B8E6EEFD3EDEEC1C}UI/layouts/Menus/RoleSelection/BriefingVictoryCondition.layout";
	protected SCR_RespawnBriefingComponent m_BriefingComponent;
	
	//------------------------------------------------------------------------------------------------
	override void GetWidgets()
	{
		super.GetWidgets();
		Widget tileSelection = GetRootWidget().FindAnyWidget("TileSelection");
		if (tileSelection)
			m_TileSelection = SCR_DeployMenuTileSelection.Cast(tileSelection.FindHandler(SCR_DeployMenuTileSelection));
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshBriefing()
	{
	}

	//------------------------------------------------------------------------------------------------
	override protected bool ConfirmSelection()
	{
		SCR_RespawnSuperMenu menu = SCR_RespawnSuperMenu.GetInstance();
		if (menu)
			menu.UpdateTabs();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		RefreshBriefing();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);

		m_ConfirmButton.SetEnabled(true, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		
		CreateConfirmButton();
		m_sConfirmButtonText = m_sContinueButtonText;
		SCR_BriefingMenuConfig briefConf;
		array<ref SCR_UIInfo> hints = {};
		array<ref SCR_BriefingVictoryCondition> conditions = {};
		int conditionCount;
		int hintCount;

		m_BriefingComponent = SCR_RespawnBriefingComponent.GetInstance();
		if (!m_BriefingComponent)
		{
			Print("SCR_RespawnBriefingSubMenu: Could not find SCR_RespawnBriefingComponent!", LogLevel.WARNING);
			return;
		}
		
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (missionHeader && !missionHeader.m_sBriefingConfig.IsEmpty())
		{
			Resource holder = BaseContainerTools.LoadContainer(missionHeader.m_sBriefingConfig);
			BaseContainer container = holder.GetResource().ToBaseContainer();
			briefConf = SCR_BriefingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			if (briefConf)
			{
				hints = briefConf.GetHints();
				hintCount = hints.Count();
				conditions = briefConf.GetWinConditions();
				conditionCount = conditions.Count();
			}
		} 
		else
		{
			hintCount = m_BriefingComponent.GetGameModeHints(hints);
			conditionCount = m_BriefingComponent.GetWinConditions(conditions);
		}
		
		//~ Default hint and condition briefing screen
		for (int i = 0; i < hintCount; ++i)
		{
			SCR_BriefingMenuTile tile = SCR_BriefingMenuTile.InitializeTile(m_TileSelection);
			tile.SetImageAndDescription(hints[i], i);
		}
		
		Widget victoryConditionContent = GetRootWidget().FindAnyWidget("VictoryConditionsContent");
		if (!victoryConditionContent)
			return;

		int x,y;
		string iconName;
		for (int i = 0; i < conditionCount; ++i)
		{
			Widget cond = GetGame().GetWorkspace().CreateWidgets(VICTORY_CONDITION_LAYOUT, victoryConditionContent); 

			TextWidget name = TextWidget.Cast(cond.FindAnyWidget("VictoryConditionText"));
			name.SetText(conditions[i].GetDescription());

			Widget icon = Widget.Cast(cond.FindAnyWidget("TaskIcon"));
			if (!icon)
				return;
			SCR_TaskIconComponent taskIconComp = SCR_TaskIconComponent.Cast(icon.FindHandler(SCR_TaskIconComponent));
			if (!taskIconComp)
				return;
			taskIconComp.SetIconType(conditions[i].GetConditionType() );
		}
		
		
		SCR_UIInfo uiInfo;
		
		if (hintCount <= 0 || conditionCount <= 0)
		{
			uiInfo = m_BriefingComponent.GetInfo();
			if (!uiInfo)
			{
				Print("SCR_RespawnBriefingSubMenu: Has no hints and/or conditions but m_BriefingComponent is misses UI info!", LogLevel.WARNING);
				return;
			}
		}
		
		
		//~ No hints show simple top
		if (hintCount <= 0)
		{
			Widget simpleBriefingTop = GetRootWidget().FindAnyWidget(m_sSimpleBriefingTop);
					
			if (!simpleBriefingTop)
			{
				Print("SCR_RespawnBriefingSubMenu: SCR_MissionHeader did not have hints but is also missing simpleBriefingTop", LogLevel.WARNING);
				return;
			}
			
			
			Widget fullTop = GetRootWidget().FindAnyWidget(m_sFullBriefingTop);
			if (fullTop)
				fullTop.SetVisible(false);
			
			ImageWidget icon = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSimpleBriefingBackIconImage));
			if (uiInfo.HasIcon())
				uiInfo.SetIconTo(icon);
			else if (icon)
				icon.SetVisible(false);
			
			string backgroundImage = m_BriefingComponent.GetSimpleBriefingBackground();
			
			ImageWidget background = ImageWidget.Cast(GetRootWidget().FindAnyWidget(m_sSimpleBriefingBackGroundImage));
			if (background)
			{
				if (!backgroundImage.IsEmpty())
					background.LoadImageTexture(0, backgroundImage);
				else 
					background.SetVisible(false);
			}
			
			simpleBriefingTop.SetVisible(true);
		}
		
		//~ No conditions show simple bottom
		if (conditionCount <= 0)
		{
			Widget simpleBriefingBottom = GetRootWidget().FindAnyWidget(m_sSimpleBriefingBottom);
			TextWidget descriptionText = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sSimpleBriefingDescriptionText));
			
						
			if (!simpleBriefingBottom || !descriptionText)
			{
				Print("SCR_RespawnBriefingSubMenu: SCR_MissionHeader did not have conditions but is also missing simpleBriefingBottom or Description Text", LogLevel.WARNING);
				return;
			}
			
			Widget fullBottom = GetRootWidget().FindAnyWidget(m_sFullBriefingButtom);
			if (fullBottom)
				fullBottom.SetVisible(false);
			
			uiInfo.SetDescriptionTo(descriptionText);
			
			simpleBriefingBottom.SetVisible(true);
		
		}
		
		
		RefreshFaction();
	}	

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		m_BriefingComponent.GetOnBriefingChanged().Insert(RefreshBriefing);

		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);

	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);

		if (m_BriefingComponent)
			m_BriefingComponent.GetOnBriefingChanged().Remove(RefreshBriefing);

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshFaction()
	{
		array<Faction> factions = {};
		Widget factionTile;
		RichTextWidget factionName;
		ImageWidget factionFlag;
		int x,y;
		Widget factionsContent = GetRootWidget().FindAnyWidget("Factions");
		if (!factionsContent)
			return;
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;			
		factionManager.GetFactionsList(factions);
		if (factions.IsEmpty())
			return;
		SCR_Faction scrFaction;
		foreach (Faction faction : factions)
		{			
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction || !scrFaction.IsPlayable())
				continue;
			factionTile = GetGame().GetWorkspace().CreateWidgets(FACTION_LAYOUT, factionsContent);
			factionName = RichTextWidget.Cast(factionTile.FindAnyWidget("FactionName"));
			if (!factionName)
				continue;
			
			factionName.SetText(scrFaction.GetFactionName());
			
			factionFlag = ImageWidget.Cast(factionTile.FindAnyWidget("FactionFlag"));
			if (!factionFlag)
				continue;
			factionFlag.LoadImageTexture(0, scrFaction.GetFactionFlag());
			
			factionFlag.GetImageSize(0, x, y);
			factionFlag.SetSize(x, y);
			
		}
	}
};