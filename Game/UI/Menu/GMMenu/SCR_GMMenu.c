
class SCR_GMMenu : ChimeraMenuBase
{
	protected const ResourceName TILES_LAYOUT = "{02155A85F2DC521F}UI/layouts/Menus/GMMenu/GMMenuTile.layout";
	protected const ResourceName CONFIG = "{CA59D3A983A1BBAB}Configs/GMMenu/GMMenuEntries.conf";
	
	protected SCR_GalleryComponent m_Gallery;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_GMMenuTileComponent m_CurrentTile;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_Gallery = SCR_GalleryComponent.GetGalleryComponent("Gallery", GetRootWidget());
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, GetRootWidget());
		if (back)
			back.m_OnActivated.Insert(OnBack);

		PrepareTiles();

		if (m_Gallery)
			m_Gallery.SetFocusedItem(0, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		if (m_CurrentTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_CurrentTile.GetRootWidget());

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareTiles()
	{
		Resource resource = BaseContainerTools.LoadContainer(CONFIG);
		if (!resource)
			return;

		BaseContainer container = resource.GetResource().ToBaseContainer();
		SCR_GMMenuConfiguration menuConf = SCR_GMMenuConfiguration.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!menuConf)
			return;

		array<ResourceName> missions = menuConf.GetScenarios();

		foreach (ResourceName mission : missions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(TILES_LAYOUT, GetRootWidget());
			if (!w)
				continue;

			SCR_GMMenuTileComponent tile = SCR_GMMenuTileComponent.Cast(w.FindHandler(SCR_GMMenuTileComponent));
			if (!tile)
				continue;

			tile.ShowMission(SCR_ScenarioUICommon.GetInGameScenario(mission));

			if (m_Gallery)
				m_Gallery.AddItem(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_GMMenuTileComponent tile)
	{
		m_CurrentTile = tile;
	}
}
