//------------------------------------------------------------------------------------------------
class GameMasterMenuUI : PlayMenuUI
{
	protected string m_sSectionTitle = "#AR-MainMenu_Editor_Name";
	protected string m_sTitleWidgetName = "Title";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		TextWidget title = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sTitleWidgetName));
		if (title)
			title.SetText(m_sSectionTitle);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PrepareHeaders()
	{
		Resource resource = BaseContainerTools.LoadContainer(m_Config);
		if (!resource)
			return;
		
		BaseContainer cont = resource.GetResource().ToBaseContainer();
		if (!cont)
			return;
		
		array<ResourceName> missions = {};
		cont.Get("m_aGameMasterMissions", missions);
		
		if (!missions)
			return;

		foreach (ResourceName str : missions)
		{
			MissionWorkshopItem item = GetMission(str);
			if (!item || !IsUnique(item, m_aEntries))
				continue;
			
			m_aEntries.Insert(new SCR_PlayEntry(item, false, false, false));
		}
	}
};