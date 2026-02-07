class SCR_MapUISpawnPoint : SCR_MapUIElement
{
	[Attribute("Select")]
	protected string m_sSelection;
	[Attribute("Base_Main")]
	protected string m_sSpawnPoint;

	protected SCR_SpawnPoint m_SpawnPoint;
	protected string m_sFactionKey;
	protected SCR_MapCampaignUI m_Parent;
	protected TextWidget m_wSpawnPointName;
	protected Widget m_wImageOverlay;
	protected OverlayWidget m_wSymbolOverlay;
	protected SCR_MilitarySymbolUIComponent m_MilitarySymbolComponent;
	protected ButtonWidget m_wButton;
	protected SizeLayoutWidget m_wSizeLayout;
	
	//------------------------------------------------------------------------------
	override void SetParent(SCR_MapCampaignUI parent)
	{
		m_Parent = parent;
	}

	//------------------------------------------------------------------------------
	void Init(SCR_SpawnPoint sp)
	{
		m_SpawnPoint = sp;
		m_sFactionKey = sp.GetFactionKey();
		UpdateIcon();
		SCR_UIInfo info = sp.GetInfo();
		if (info)
			info.SetNameTo(m_wSpawnPointName);

		SCR_MapDescriptorComponent descr = SCR_MapDescriptorComponent.Cast(m_SpawnPoint.FindComponent(SCR_MapDescriptorComponent));
		
		if(!descr)
			return;
		
		m_MapItem = descr.Item();
		MapDescriptorProps props = m_MapItem.GetProps();
		props.SetIconVisible(false);
		props.SetTextVisible(false);
		props.Activate(true);
	}

	//------------------------------------------------------------------------------
	void UpdateIcon()
	{
		SCR_MilitarySymbol symbol = new SCR_MilitarySymbol;
		
		SCR_GroupIdentityCore core = SCR_GroupIdentityCore.Cast(SCR_GroupIdentityCore.GetInstance(SCR_GroupIdentityCore));
		if (!core)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey(m_sFactionKey));
		if (faction)
		{
			SCR_MilitarySymbolRuleSet ruleSet = core.GetSymbolRuleSet();
			ruleSet.UpdateSymbol(symbol, faction);
		}
		else
		{
			symbol.SetIdentity(EMilitarySymbolIdentity.UNKNOWN);
		}
		
		//Selection visuals 
		string selection;
		string highlight;
		switch (symbol.GetIdentity())
		{
			case EMilitarySymbolIdentity.INDFOR:
			{
				selection = "Neutral_Select";
				highlight = "Neutral_Focus";
				break;
			}
			case EMilitarySymbolIdentity.OPFOR:
			{
				selection = "Hostile_Select";
				highlight = "Hostile_Focus";
				break;
			}
			case EMilitarySymbolIdentity.BLUFOR:
			{
				selection = "Friend_Select";
				highlight = "Friend_Focus";
				break;
			}
			case EMilitarySymbolIdentity.UNKNOWN:
			{
				selection = "Unknown_Select";
				highlight = "Unknown_Focus";
				break;
			}
		}
		m_bVisible = true;
		
		m_wHighlightImg.LoadImageFromSet(0, m_sImageSetARO, highlight);
		m_wSelectImg.LoadImageFromSet(0, m_sImageSetARO, selection);
		
		if (SCR_PlayerSpawnPoint.Cast(m_SpawnPoint))
			symbol.SetIcons(EMilitarySymbolIcon.RELAY);
		else
			symbol.SetIcons(EMilitarySymbolIcon.RESPAWN);
		
		m_wSymbolOverlay.SetColor(GetColorForFaction(m_sFactionKey));
		if (m_wGradient)
			m_wGradient.SetColor(GetColorForFaction(m_sFactionKey));
				
		m_MilitarySymbolComponent.Update(symbol);
	}

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wImageOverlay = w.FindAnyWidget("IconOverlay");
		m_wSizeLayout = SizeLayoutWidget.Cast(w.FindAnyWidget("SizeLayout"));
		m_wSpawnPointName = TextWidget.Cast(w.FindAnyWidget("Name"));
		SCR_SelectSpawnPointSubMenu.Event_OnSpawnPointChanged.Insert(OnSelected);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(Remove);
		m_wSymbolOverlay = OverlayWidget.Cast(m_wImageOverlay.FindWidget("Symbol"));
		if (!m_wSymbolOverlay)
			return;
		
		m_MilitarySymbolComponent = SCR_MilitarySymbolUIComponent.Cast(m_wSymbolOverlay.FindHandler(SCR_MilitarySymbolUIComponent));
		
		m_wButton = ButtonWidget.Cast(w.FindAnyWidget("Button"));
		if (m_wButton)
			m_wButton.SetEnabled(false);
	}

	//------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_SelectSpawnPointSubMenu.Event_OnSpawnPointChanged.Remove(OnSelected);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(Remove);
	}

	//------------------------------------------------------------------------------
	void Remove(SCR_SpawnPoint sp)
	{
		if (sp == m_SpawnPoint)
			m_Parent.RemoveIcon(this);
	}
	
	//------------------------------------------------------------------------------
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}

	//------------------------------------------------------------------------------
	override void SelectIcon()
	{
		if (!m_wSelectImg)
			return;

		if (s_SelectedElement && s_SelectedElement != this)
			s_SelectedElement.Select(false);

		Select();

		m_wSelectImg.SetVisible(true);
		if (m_wGradient)
			m_wGradient.SetVisible(true);

		if (m_bIsSelected)
			Event_OnPointSelected.Invoke(m_SpawnPoint);
	}

	//------------------------------------------------------------------------------
	protected void OnSelected(SCR_SpawnPoint sp)
	{
		if (!sp)
			return;
		if (sp == m_SpawnPoint)
		{
			AnimExpand();
			m_wRoot.SetZOrder(1);
			m_wSelectImg.SetVisible(true);
			if (m_wGradient)
				m_wGradient.SetVisible(true);
		}
		else
		{
			AnimCollapse();
			m_wRoot.SetZOrder(0);
			m_wSelectImg.SetVisible(false);
			if (m_wGradient)
				m_wGradient.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_wSizeLayout && w == m_wSizeLayout && m_wButton)
			m_wButton.SetEnabled(true);
		
		GetGame().GetWorkspace().SetFocusedWidget(w);
		super.OnMouseEnter(w, x, y);
		m_wRoot.SetZOrder(1);
		m_wHighlightImg.SetVisible(true);
		if (m_wGradient)
			m_wGradient.SetVisible(true);
		if (!m_bIsSelected)
		{
			AnimExpand();
		}
		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		m_wRoot.SetZOrder(0);
		m_wHighlightImg.SetVisible(false);
		if (!m_bIsSelected && m_wGradient)
			m_wGradient.SetVisible(false);
		if (!m_bIsSelected)
		{
			AnimCollapse();
		}
		
		if (RenderTargetWidget.Cast(enterW) && m_wButton.IsEnabled())
			m_wButton.SetEnabled(false);
		
		return false;
	}
};