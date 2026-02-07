enum EIconType
{
	NONE,
	BASE,
	ENEMY_BASE,
	RELAY,
	TASK,
	SERVICE,
	SPAWNPOINT
};

class SCR_MapUIElement : ScriptedWidgetComponent
{
	protected static SCR_MapUIElement s_SelectedElement;
	protected SCR_MapCampaignUI m_Par;
	protected MapItem m_MapItem;
	protected Widget m_wRoot;
	protected Widget m_wBaseFrame;
	protected ImageWidget m_wImage;
	protected Widget m_wBaseIcon;
	protected ImageWidget m_wGradient;
	protected ImageWidget m_wSelectImg;
	protected ImageWidget m_wHighlightImg;
	protected Widget m_wAntennaImg;
	protected bool m_bIsSelected;
	protected bool m_bIsHovering;
	protected const float ANIM_SPEED = 20;
	protected EIconType m_eIconType;
	protected bool m_bVisible = false;

	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset")]
	protected ResourceName m_sImageSet;
	
	[Attribute("{8479B3B5347DF5CF}UI/Imagesets/MilitarySymbol/ID_D.imageset")]
	protected ResourceName m_sImageSetARO;
	
	[Attribute("{27F2439D610D02B3}UI/Imagesets/MilitarySymbol/ICO_Land.imageset")]
	protected ResourceName m_sImageSetSpecial;

	[Attribute("1")]
	protected bool m_bUseBackgroundGradient;

	[Attribute(SCR_SoundEvent.SOUND_MAP_HOVER_BASE)]
	protected string m_sSoundBase;
	[Attribute(SCR_SoundEvent.SOUND_MAP_HOVER_ENEMY)]
	protected string m_sSoundEnemyBase;
	[Attribute(SCR_SoundEvent.SOUND_MAP_HOVER_TRANS_TOWER)]
	protected string m_sSoundRelay;
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER)]
	protected string m_sSoundTask;	
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER)]
	protected string m_sSoundService;	
	[Attribute()]
	ref Color m_UnknownFactionColor;
	protected string m_sName;

	static ref ScriptInvoker Event_OnPointSelected = new ScriptInvoker();

	//------------------------------------------------------------------------------
	void SetParent(SCR_MapCampaignUI parent)
	{
		m_Par = parent;
	}

	//------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------
	bool GetIconVisible()
	{
		return m_bVisible;
	}


	//------------------------------------------------------------------------------
	void ShowName(bool visible)
	{
	}

	//------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		m_wRoot.SetVisible(visible);
	}

	//------------------------------------------------------------------------------
	void SetOpacity(float val)
	{
		m_wRoot.SetOpacity(val);
	}

	//------------------------------------------------------------------------------
	void SetIconSize(float val)
	{
		FrameSlot.SetSize(m_wRoot, val, val);
	}

	//------------------------------------------------------------------------------
	Widget GetRoot()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------
	vector GetIconSize()
	{
		return FrameSlot.GetSize(m_wBaseFrame);
	}

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_wBaseFrame = w.FindAnyWidget("BaseFrame");
		m_wBaseIcon = Widget.Cast(w.FindAnyWidget("SideSymbol"));
		m_wImage = ImageWidget.Cast(w.FindAnyWidget("Image"));
		m_wSelectImg = ImageWidget.Cast(w.FindAnyWidget("Corners"));
		m_wHighlightImg = ImageWidget.Cast(w.FindAnyWidget("Highlight"));
		m_wAntennaImg = w.FindAnyWidget("AntenaOff");
		if (m_bUseBackgroundGradient)
			m_wGradient = ImageWidget.Cast(w.FindAnyWidget("BackgroundGradient"));
	}

	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bIsHovering = true;

		return false;
	}

	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bIsHovering = false;

		return false;
	}

	// ------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if(m_bVisible)
			SelectIcon();

		return false;
	}

	// ------------------------------------------------------------------------------
	sealed void SelectIcon()
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
			Event_OnPointSelected.Invoke(m_MapItem);
	}

	// ------------------------------------------------------------------------------
	void Select(bool select = true)
	{
		s_SelectedElement = this;
		m_bIsSelected = select;
		if (m_wSelectImg)
			m_wSelectImg.SetVisible(select);
		if (select)
		{
			s_SelectedElement = this;
			AnimExpand();
		}
		else
		{
			AnimCollapse();
		}
		if (m_wGradient)
			m_wGradient.SetVisible(select);
	}

	//------------------------------------------------------------------------------
	protected void AnimExpand()
	{
	}

	//------------------------------------------------------------------------------
	protected void AnimCollapse()
	{
	}

	//------------------------------------------------------------------------------
	protected void PlayHoverSound(string sound)
	{
		if (sound != string.Empty)
			SCR_UISoundEntity.SoundEvent(sound);
	}

	//------------------------------------------------------------------------------
	vector GetPos()
	{
		if (!m_MapItem)
			return vector.Zero;

		return m_MapItem.GetPos();
	}

	//------------------------------------------------------------------------------
	void SetImage(string image)
	{
		if (m_wImage)
			m_bVisible = m_wImage.LoadImageFromSet(0, m_sImageSet, image);
	}

	//------------------------------------------------------------------------------
	Color GetColorForFaction(string factionKey)
	{
		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return null;

		Faction faction = fm.GetFactionByKey(factionKey);
		if (!faction)
			return m_UnknownFactionColor;

		return faction.GetFactionColor();
	}
};
