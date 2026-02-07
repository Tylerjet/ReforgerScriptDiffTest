class SCR_LoadoutPreviewComponent : ScriptedWidgetComponent
{
	[Attribute("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et")]
	protected ResourceName m_sPreviewManager;

	[Attribute("Preview")]
	protected string m_sPreviewWidgetName;

	protected ItemPreviewManagerEntity m_PreviewManager;
	protected ItemPreviewWidget m_wPreview;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wPreview = ItemPreviewWidget.Cast(w.FindAnyWidget(m_sPreviewWidgetName));
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
		m_PreviewManager.SetPreviewItemFromPrefab(m_wPreview, resName);

		return m_PreviewManager.ResolvePreviewEntityForPrefab(resName);
	}
};