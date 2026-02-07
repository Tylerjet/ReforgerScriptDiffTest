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
		// todo: temp
		string img = string.Format("%1_%2", m_sFactionKey, m_sSpawnPoint);
		string selectionImg = string.Format("%1_%2", img, m_sSelection);
		m_wImage.SetColor(GetColorForFaction(m_sFactionKey));
		SetImage(img);
		m_wSelectImg.LoadImageFromSet(0, m_sImageSet, selectionImg);
	}

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wImageOverlay = w.FindAnyWidget("Overlay");
		m_wSpawnPointName = TextWidget.Cast(w.FindAnyWidget("Name"));
		SCR_SelectSpawnPointSubMenu.Event_OnSpawnPointChanged.Insert(OnSelected);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(Remove);
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
	protected void OnSelected(SCR_SpawnPoint sp)
	{
		if (!sp)
			return;
		if (sp == m_SpawnPoint)
			AnimExpand();
		else
			AnimCollapse();
	}

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
		m_wRoot.SetZOrder(1);
		if (!m_bIsSelected)
		{
			AnimExpand();
		}
		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_wRoot.SetZOrder(0);
		if (!m_bIsSelected)
		{
			AnimCollapse();
		}
		return false;
	}

	//------------------------------------------------------------------------------
	override void AnimExpand()
	{
		int expand = -10;
		AlignableSlot.SetPadding(m_wImageOverlay, expand, expand, expand, expand);
	}

	//------------------------------------------------------------------------------
	override void AnimCollapse()
	{
		AlignableSlot.SetPadding(m_wImageOverlay, 0, 0, 0, 0);
	}
};