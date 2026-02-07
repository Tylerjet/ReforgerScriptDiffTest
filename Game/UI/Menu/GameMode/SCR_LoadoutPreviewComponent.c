class SCR_LoadoutPreviewComponent : ScriptedWidgetComponent
{
	[Attribute("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et")]
	protected ResourceName m_sPreviewManager;
	protected ItemPreviewManagerEntity m_PreviewManager;
	protected ItemPreviewWidget m_wPreview;

	protected IEntity m_PreviewedLoadout;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wPreview = ItemPreviewWidget.Cast(w.FindAnyWidget("Preview"));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_PreviewedLoadout)
			delete m_PreviewedLoadout;
	}

	//------------------------------------------------------------------------------------------------
	IEntity SetPreviewedLoadout(notnull SCR_BasePlayerLoadout loadout)
	{
		m_PreviewManager = GetGame().GetItemPreviewManager();

		if (!m_PreviewManager)
		{
			Resource res = Resource.Load(m_sPreviewManager);
			if (res.IsValid())
				GetGame().SpawnEntityPrefabLocal(res);
			m_PreviewManager = GetGame().GetItemPreviewManager();
			if (!m_PreviewManager)
			{
				return null;
			}
		}
		
		ResourceName resName = loadout.GetLoadoutResource();
		Resource res = Resource.Load(resName);

		if (m_PreviewedLoadout)
			delete m_PreviewedLoadout;

		m_PreviewedLoadout = GetGame().SpawnEntityPrefabLocal(res);

		if (m_PreviewedLoadout && m_wPreview)
		{
			m_PreviewManager.SetPreviewItem(m_wPreview, m_PreviewedLoadout);
			m_PreviewedLoadout.ClearFlags(EntityFlags.ACTIVE | EntityFlags.TRACEABLE, true);
			m_PreviewedLoadout.SetFlags(EntityFlags.NO_LINK, true);
		}

		return m_PreviewedLoadout;
	}
};