class SCR_LoadoutGallery : SCR_GalleryComponent
{
	[Attribute("{39D815C843414C76}UI/layouts/Menus/DeployMenu/LoadoutButton.layout")]
	protected ResourceName m_sLoadoutButton;
	protected ref set<SCR_LoadoutButton> m_aLoadoutButtons = new set<SCR_LoadoutButton>();	

	protected ref ScriptInvoker<SCR_LoadoutButton> m_OnLoadoutClicked;
	protected ref ScriptInvoker<SCR_BasePlayerLoadout> m_OnLoadoutHovered;

	int AddItem(SCR_BasePlayerLoadout loadout, bool enabled = true)
	{
		Widget loadoutEntry = GetGame().GetWorkspace().CreateWidgets(m_sLoadoutButton);
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(loadoutEntry.FindHandler(SCR_LoadoutButton));
		if (loadoutBtn)
		{
			loadoutBtn.SetLoadout(loadout);
			loadoutBtn.SetEnabled(enabled);

			loadoutBtn.m_OnClicked.Insert(OnLoadoutClicked);
			loadoutBtn.m_OnFocus.Insert(OnLoadoutHovered);
			loadoutBtn.m_OnMouseEnter.Insert(OnLoadoutHovered);

			m_aLoadoutButtons.Insert(loadoutBtn);
		}

		int itm = AddItem(loadoutEntry);
		ShowPagingButtons(); // hack to go around hajkovinas

		return itm;
	}
	
	override void ShowPagingButtons(bool animate = true)
	{
		bool show = (m_aWidgets.Count() > m_iCountShownItems);
		if (m_PagingLeft)
			m_PagingLeft.SetVisible(show, false);
		
		if (m_PagingRight)
			m_PagingRight.SetVisible(show, false);
	}

	protected void OnLoadoutClicked(notnull SCR_LoadoutButton loadoutBtn)
	{
		GetOnLoadoutClicked().Invoke(loadoutBtn);
	}
	
	protected void OnLoadoutHovered(Widget btn)
	{
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn.FindHandler(SCR_LoadoutButton));
		if (loadoutBtn)
			GetOnLoadoutHovered().Invoke(loadoutBtn.GetLoadout());
	}
	
	Widget GetContentRoot()
	{
		return m_wContentRoot;
	}

	void SetSelected(SCR_BasePlayerLoadout loadout)
	{
		foreach (SCR_LoadoutButton loadoutBtn : m_aLoadoutButtons)
		{
			loadoutBtn.SetSelected(loadoutBtn.GetLoadout() == loadout)
		}
	}

	override void ClearAll()
	{
		super.ClearAll();
		m_aLoadoutButtons.Clear();
	}

	SCR_LoadoutButton GetButtonForLoadout(SCR_BasePlayerLoadout loadout)
	{
		foreach (SCR_LoadoutButton btn : m_aLoadoutButtons)
		{
			if (btn.GetLoadout() == loadout)
				return btn;
		}

		return null;
	}
	
	ScriptInvoker GetOnLoadoutClicked()
	{
		if (!m_OnLoadoutClicked)
			m_OnLoadoutClicked = new ScriptInvoker();

		return m_OnLoadoutClicked;
	}
	
	ScriptInvoker GetOnLoadoutHovered()
	{
		if (!m_OnLoadoutHovered)
			m_OnLoadoutHovered = new ScriptInvoker();

		return m_OnLoadoutHovered;
	}	
};