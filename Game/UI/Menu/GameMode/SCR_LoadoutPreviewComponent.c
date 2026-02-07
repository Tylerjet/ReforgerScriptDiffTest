class SCR_LoadoutPreviewComponent : ScriptedWidgetComponent
{
	[Attribute("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et")]
	protected ResourceName m_sPreviewManager;

	[Attribute("Preview")]
	protected string m_sPreviewWidgetName;

	protected ItemPreviewManagerEntity m_PreviewManager;
	protected ItemPreviewWidget m_wPreview;
	
	protected bool m_bReloadLoadout;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wPreview = ItemPreviewWidget.Cast(w.FindAnyWidget(m_sPreviewWidgetName));
		m_bReloadLoadout = true;
	}

	//------------------------------------------------------------------------------------------------
	IEntity SetPreviewedLoadout(notnull SCR_BasePlayerLoadout loadout, PreviewRenderAttributes attributes = null)
	{
		if (!m_bReloadLoadout)
			return null;
		
		ChimeraWorld world = GetGame().GetWorld();
		m_PreviewManager = world.GetItemPreviewManager();

		if (!m_PreviewManager)
		{
			Resource res = Resource.Load(m_sPreviewManager);
			if (res.IsValid())
				GetGame().SpawnEntityPrefabLocal(res, world);
			
			m_PreviewManager = world.GetItemPreviewManager();
			if (!m_PreviewManager)
			{
				return null;
			}
		}
		
		if (SCR_PlayerArsenalLoadout.Cast(loadout))
		{
			Resource resource = Resource.Load(loadout.GetLoadoutResource());
			if (!resource)
				return null;
			
			IEntity char = GetGame().SpawnEntityPrefabLocal(resource);
			if (!char)
				return null;
			
			m_PreviewManager.SetPreviewItem(m_wPreview, char, attributes);
			return char;
		}
		else
		{
			ResourceName resName = loadout.GetLoadoutResource();
			m_PreviewManager.SetPreviewItemFromPrefab(m_wPreview, resName, attributes);

			return m_PreviewManager.ResolvePreviewEntityForPrefab(resName);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ItemPreviewManagerEntity GetPreviewManagerEntity()
	{
		return m_PreviewManager;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPreviewManagerEntity(ItemPreviewManagerEntity instance)
	{
		m_PreviewManager = instance;
	}

	//------------------------------------------------------------------------------------------------
	ItemPreviewWidget GetItemPreviewWidget()
	{
		return m_wPreview;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItemPreviewWidget(ItemPreviewWidget instance)
	{
		m_wPreview = instance;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetReloadLoadout(bool flag)
	{
		m_bReloadLoadout = flag;
	}
};