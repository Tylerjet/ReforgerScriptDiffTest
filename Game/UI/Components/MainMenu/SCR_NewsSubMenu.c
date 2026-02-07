[BaseContainerProps()]
class SCR_NewsEntry
{
	NewsFeedItem m_Item;
	SCR_NewsTileComponent m_Tile;
	Widget m_wRoot;
	bool m_bRead;
	
	void SCR_NewsEntry(NewsFeedItem item)
	{
		m_Item = item;
	}
	
	void SetTile(SCR_NewsTileComponent tile, Widget w)
	{
		m_Tile = tile;
		m_wRoot = w;
	}
};

class SCR_NewsSubMenu : SCR_SubMenuBase
{
	[Attribute("{02155A85F2DC521F}UI/layouts/Menus/PlayMenu/PlayMenuTile.layout", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_Layout;

	protected ref array<ref SCR_NewsEntry> m_aEntries = {};

	protected SCR_GalleryComponent m_Gallery;
	protected int m_iCurrentIndex;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_Gallery = SCR_GalleryComponent.GetGalleryComponent("Gallery", m_wRoot);
		MainMenuUI.GetNewsEntries(m_aEntries);
		SetNewsEntries();
	}

	//------------------------------------------------------------------------------------------------
	void SetNewsEntries()
	{
		m_Gallery.ClearAll();
		
		foreach (SCR_NewsEntry entry : m_aEntries)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_Layout);
			if (!w)
				continue;

			SCR_NewsTileComponent tile = SCR_NewsTileComponent.Cast(w.FindHandler(SCR_NewsTileComponent));
			if (!tile)
				continue;

			entry.SetTile(tile, tile.m_wRoot);
			tile.m_OnRead.Insert(OnRead);
			tile.ShowTile(entry);

			m_Gallery.AddItem(w);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRead(SCR_NewsTileComponent comp)
	{
		
		string url = comp.m_Entry.m_Item.URL();
		if (url.IsEmpty())
			return;
		
		GetGame().GetPlatformService().OpenBrowser(url);
		
		/*
		if (!m_Gallery)
			return;
		
		array<Widget> widgets = {};
		m_Gallery.GetWidgets(widgets);
		if (!widgets)
			return;
		
		m_iCurrentIndex = widgets.Find(comp.m_wRoot);
		
		if (m_iCurrentIndex < -1 || m_iCurrentIndex >= m_aEntries.Count())
			return;
		
		NewsDialogUI dialog = NewsDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.NewsDialog));
		if (!dialog)
			return;
		
		SCR_NewsEntry entry = m_aEntries[m_iCurrentIndex];
		if (!entry)
			return;
		
		dialog.SetTitle(entry.m_sTitle);
		dialog.SetMessage(entry.m_sFullText);
		dialog.SetAuthor(entry.m_sAuthor);
		dialog.SetDate(entry.m_sDate);
		dialog.SetImage(entry.m_sImage);
		*/
	}
};