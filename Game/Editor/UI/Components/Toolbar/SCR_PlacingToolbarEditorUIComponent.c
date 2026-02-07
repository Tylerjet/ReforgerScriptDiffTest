/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_PlacingToolbarEditorUIComponent: SCR_BaseToolbarEditorUIComponent
{
	[Attribute()]
	protected ref array<ref SCR_ContentBrowserEditorCard> m_aCardPrefabs;
	
	protected SCR_PlacingEditorComponent m_PlacingManager;
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected ResourceName m_RepeatPrefab;
	protected ResourceName m_DefaultLayout;
	
	//--- ToDo: Move to sandbox
	ResourceName GetPrefab(int prefabID)
	{
		SCR_PlacingEditorComponentClass placingPrefabData = SCR_PlacingEditorComponentClass.Cast(m_PlacingManager.GetEditorComponentData());
		if (placingPrefabData)
			return placingPrefabData.GetPrefab(prefabID);
		else
			return ResourceName.Empty;
	}
	void OnCardHover(Widget itemWidget, int prefabIndex, bool enter)
	{
		if (!enter)
			prefabIndex = -1;
		m_ContentBrowserManager.RefreshPreviewCost(prefabIndex);
	}
	
	void OnCardLMB(Widget itemWidget)
	{
		SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(itemWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		if (!assetCard)
			return;
		
		int prefabID = assetCard.GetPrefabIndex();
		ResourceName prefab = GetPrefab(prefabID);
		
		m_PlacingManager.SetSelectedPrefab(prefab, showBudgetMaxNotification: true);
			
		SCR_PlacingToolbarEditorUIComponent linkedComponent = SCR_PlacingToolbarEditorUIComponent.Cast(m_LinkedComponent);
		if (linkedComponent)
			linkedComponent.m_RepeatPrefab = prefab;
		
		if (m_bIsInDialog)
		{
			EditorMenuBase menu = EditorMenuBase.Cast(GetMenu());
			if (menu)
				menu.CloseSelf();
		}
	}
	
	protected Widget CreateItem(int index)
	{
		int prefabID = m_ContentBrowserManager.GetFilteredPrefabID(index);
		if (prefabID < 0)
			return null;
		
		SCR_EditableEntityUIInfo info = m_ContentBrowserManager.GetInfo(prefabID);
		
		//--- Select layout
		m_ItemLayout = m_DefaultLayout;
		if (info)
		{
			foreach (SCR_ContentBrowserEditorCard itemLayoutCandidate: m_aCardPrefabs)
			{
				if (itemLayoutCandidate.m_EntityType == info.GetEntityType())
				{
					m_ItemLayout = itemLayoutCandidate.m_sPrefab;
					break;
				}
			}
		}
		
		//--- Create layout
		Widget itemWidget;
		SCR_BaseToolbarItemEditorUIComponent item;
		if (!CreateItem(itemWidget, item))
			return null;
		
		SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(itemWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		assetCard.SetPrefabIndex(prefabID);
		assetCard.GetOnHover().Insert(OnCardHover);
		assetCard.InitCard(prefabID, info, GetPrefab(prefabID));
		ButtonActionComponent.GetOnAction(itemWidget, true, 0).Insert(OnCardLMB);
		
		SCR_ButtonBaseComponent buttonComponent = SCR_ButtonBaseComponent.Cast(itemWidget.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComponent)
			buttonComponent.SetMouseOverToFocus(false);
		
		return itemWidget;
	}
	
	override protected void OnPageChanged(int page)
	{
		if (m_ContentBrowserManager.GetPageEntryCount() > 0)
		{
			m_iFirstShownIndex = page * m_Pagination.GetColumns();
			int contentBrowserPageIndex = m_iFirstShownIndex / m_ContentBrowserManager.GetPageEntryCount();
			m_ContentBrowserManager.SetPageIndex(contentBrowserPageIndex);
		}
		super.OnPageChanged(page);
	}
	
	override protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		for (int i = indexStart; i < indexEnd; i++)
		{
			CreateItem(i);
		}
	}
	
	protected void OnBrowserEntriesFiltered()
	{
		if (!m_ContentBrowserManager || !m_Pagination || m_ContentBrowserManager.GetContentBrowserDisplayConfig())
			return;
		
		Refresh();
	}
	
	protected void OnBrowserStatesSaved()
	{
		if (!m_ContentBrowserManager || !m_Pagination || m_ContentBrowserManager.GetContentBrowserDisplayConfig())
			return;
		
		Refresh();
	}
	
	override protected void Refresh()
	{		
		m_Pagination.SetEntryCount(m_ContentBrowserManager.GetFilteredPrefabCount());
		
		int contentBrowserPage = m_ContentBrowserManager.GetPageIndex();
		int contentBrowserPageEntryCount = m_ContentBrowserManager.GetPageEntryCount();
		int toolbarPageIndex = (contentBrowserPage * contentBrowserPageEntryCount) / m_Pagination.GetColumns();
		
		m_Pagination.SetPage(toolbarPageIndex);
		
		super.Refresh();
	}
	override void OnRepeat()
	{
		if (m_RepeatPrefab)
			m_PlacingManager.SetSelectedPrefab(m_RepeatPrefab, showBudgetMaxNotification: true);
	}
	override void HandlerAttachedScripted(Widget w)
	{
		m_PlacingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true));
		if (!m_PlacingManager)
			return;
		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (!m_ContentBrowserManager)
			return;
		
		m_ContentBrowserManager.GetOnBrowserStatesSaved().Insert(OnBrowserStatesSaved);
		m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Insert(OnBrowserEntriesFiltered);
		
		//--- Find default card layout
		foreach (SCR_ContentBrowserEditorCard defaultLayoutCandidate: m_aCardPrefabs)
		{
			if (defaultLayoutCandidate.m_EntityType == EEditableEntityType.GENERIC)
			{
				m_DefaultLayout = defaultLayoutCandidate.m_sPrefab;
				break;
			}
		}
		
		super.HandlerAttachedScripted(w);
	}
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_ContentBrowserManager)
		{
			m_ContentBrowserManager.GetOnBrowserStatesSaved().Remove(OnBrowserStatesSaved);
			m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Remove(OnBrowserEntriesFiltered);
		}
	}
}