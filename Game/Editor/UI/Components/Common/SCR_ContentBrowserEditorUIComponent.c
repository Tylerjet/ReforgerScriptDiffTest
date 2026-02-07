/** @ingroup Editor_UI Editor_UI_Components
The Content Browser Window in the Content Browser Dialog
*/
class SCR_ContentBrowserEditorUIComponent: SCR_BasePaginationUIComponent//MenuRootSubComponent
{
	[Attribute()]
	protected ref array<ref SCR_ContentBrowserEditorCard> m_aCardPrefabs;
	[Attribute("4", desc: "Speed value passed to WidgetAnimator, 1 = 1sec, higher = faster")]
	private float m_iCardFadeInSpeed;
	
	[Attribute(params: "layout")]
	private ResourceName m_sCardPrefab;
	
	[Attribute(params: "layout")]
	private ResourceName m_sEmptyCardPrefab;
	
	[Attribute(params: "layout")]
	private ResourceName m_sUndefinedCardPrefab;
	
	[Attribute("Filters")]
	private string m_sLabelComponentWidget;
	
	[Attribute("NoFilterResults")]
	protected string m_sNoFilterResultsWidgetName;
	
	[Attribute(defvalue: "ResetFilterButton")]
	protected string m_sResetFiltersButtonName;
	
	[Attribute("EditBoxSearch")]
	private string m_sSearchEditBoxName;
	
	[Attribute("Scroll")]
	private string m_sScrollLayoutName;
	
	[Attribute("LoadingCircle")]
	private string m_sContentLoadingWidgetName;
	
	[Attribute(defvalue: "ToggleBetweenCardAndFilter")]
	protected string m_sToggleCardFilterButton;
	
	[Attribute(defvalue: "ToggleSearch")]
	protected string m_sToggleSearchButton;
	
	[Attribute(defvalue: "ActiveFilters")]
	protected string m_sActiveFiltersName;
	
	[Attribute(UISounds.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSfxOnOpenDialog;
	
	[Attribute(UISounds.CLICK_CANCEL, UIWidgets.EditBox, "Played when no entity card is selected on close dialog")]
	protected string m_sSfxOnCloseDialog;
	
	private ref array<int> m_aFilteredPrefabIDs = {};
	private int m_iFilteredPrefabIDsCount;
	private SCR_PrefabsCacheEditorComponent m_PrefabsCache;
	private SCR_EditableEntityCore m_EntityCore;
	private SCR_BudgetEditorComponent m_BudgetManager;
	private SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	private SCR_PlacingEditorComponent m_PlacingManager;
	private EditorBrowserDialogUI m_ContentBrowserDialog;
	
	private SCR_ContentBrowserFiltersEditorUIComponent m_LabelsUIComponent;
	
	protected Widget m_wFocusedCard;
	protected int m_iCardFocusIndex;
	protected bool m_bBudgetPreviewUpdateEnabled;
	protected bool m_bUsingGamePad;
	protected bool m_bFocusingAssetCard;
	protected bool m_bSearchIsToggled;
	protected bool m_bContentLoadingWidgetActive;
	protected bool m_bAnimateEntries;
	protected SCR_NavigationButtonComponent m_wToggleCardFilterButton;
	
	//Widget refs
	protected Widget m_wToggleSearchButton;
	protected Widget m_LastSelectedOnSearch;
	protected Widget m_NoFilterResultsWidget;
	
	protected Widget m_ContentLoadingVisual;
	protected SCR_EditBoxComponent m_SearchEditBox;
	protected ScrollLayoutWidget m_ScrollLayout;
	protected string m_sLastSearch;
	protected ref array<string> m_aLocalizationKeys = {};
	
	private ref ScriptInvoker m_LabelApplyCallback = new ScriptInvoker();
	private ref ScriptInvoker m_ShowLoadingCircleCallback = new ScriptInvoker();
	
	protected bool m_bCardSelected; //~ If card selected then no close Sfx is played
	
	/*!
	On Card click
	*/
	void OnCardLMB(Widget assetWidget)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent));
		if (!placingManager) return;
		
		int entryIndex = GetEntryIndex(assetWidget);
		int prefabID = m_aFilteredPrefabIDs[entryIndex];
		
		ResourceName prefab = GetPrefab(prefabID);
		
		if (placingManager.SetSelectedPrefab(prefab, showBudgetMaxNotification: true))
		{
			// Only close window if prefab can be selected
			m_bBudgetPreviewUpdateEnabled = false;
			CloseContentBrowser();
			m_bCardSelected = true;
		}
	}
	
	/*!
	Close content browser menu from handler, helper function for e.g. autotests
	*/
	void CloseContentBrowser()
	{
		if (m_ContentBrowserDialog)
			m_ContentBrowserDialog.Close();
	}
	
	override void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		Widget firstCard;
		
		//Show no result indicator and hide grid		
		bool showNoResults = indexEnd == 0 && !m_bContentLoadingWidgetActive;
		if (m_NoFilterResultsWidget)
			m_NoFilterResultsWidget.SetVisible(showNoResults);
		contentWidget.SetVisible(!showNoResults);

		SCR_PlacingEditorComponentClass placingPrefabData = SCR_PlacingEditorComponentClass.Cast(SCR_PlacingEditorComponentClass.GetInstance(SCR_PlacingEditorComponent, true));
		if (!placingPrefabData) return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return;
		
		//ResourceName prefab;
		Resource prefabResource;
		IEntityComponentSource componentSource;
		SCR_UIInfo info;
		ResourceName cardPrefab, defaultCardPrefab;
		foreach (SCR_ContentBrowserEditorCard cardPrefabCandidate: m_aCardPrefabs)
		{
			if (cardPrefabCandidate.m_EntityType == EEditableEntityType.GENERIC)
			{
				defaultCardPrefab = cardPrefabCandidate.m_sPrefab;
				break;
			}
		}
		
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		
		for (int i = indexStart; i < indexEnd; i++)
		{
			int prefabID = m_aFilteredPrefabIDs[i];
			info = m_ContentBrowserManager.GetInfo(prefabID);
			
			cardPrefab = m_sUndefinedCardPrefab;
			if (info)
			{
				SCR_EditableEntityUIInfo editableEntityInfo = SCR_EditableEntityUIInfo.Cast(info);
				cardPrefab = defaultCardPrefab;
				foreach (SCR_ContentBrowserEditorCard cardPrefabCandidate: m_aCardPrefabs)
				{
					if (cardPrefabCandidate.m_EntityType == editableEntityInfo.GetEntityType())
					{
						cardPrefab = cardPrefabCandidate.m_sPrefab;
						break;
					}
				}
			}
			
			//--- Create card
			Widget assetWidget = workspace.CreateWidgets(cardPrefab, contentWidget);
			if (!assetWidget) continue;
			
			SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(assetWidget.FindHandler(SCR_AssetCardFrontUIComponent));
			if (!assetCard)
			{
				Print("Asset widget is missing SCR_AssetCardFrontUIComponent!", LogLevel.ERROR);
				assetWidget.RemoveFromHierarchy();
				continue;
			}
			
			SCR_EditableEntityCoreBudgetSetting blockingBudgetSettings;
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (GetEntityPreviewBudgetCosts(prefabID, budgetCosts))
			{
				EEditableEntityBudget blockingBudget;
				if (!m_BudgetManager.CanPlace(budgetCosts, blockingBudget, false))
				{
					 m_EntityCore.GetBudgetSettingsForBudgetType(blockingBudget, blockingBudgetSettings);
				}
			}
			
			assetCard.SetPrefabIndex(prefabID);
			
			//Set card focus
			if (firstCard == null)
				firstCard = assetCard.GetButtonWidget();
			if (prefabID == m_iCardFocusIndex)
				m_wFocusedCard = assetCard.GetButtonWidget();
			
			assetCard.GetOnHover().Insert(OnCardHover);
			assetCard.GetOnFocus().Insert(OnCardFocus);
			assetCard.InitCard(prefabID, info, GetPrefab(prefabID), blockingBudgetSettings);
			ButtonActionComponent.GetOnAction(assetWidget, true, 0).Insert(OnCardLMB);
			
			if (m_bAnimateEntries)
			{
				assetWidget.SetOpacity(0);
				int cardIndexOnPage = (i - indexStart);
				callQueue.CallLater(AnimateCardFadeIn, cardIndexOnPage * 20, false, assetWidget);
			}
		}
		
		//No focused card
		if (m_wFocusedCard == null)
			m_wFocusedCard = firstCard;
		
		m_bAnimateEntries = false;
	}
	
	override int GetEntryCount()
	{
		return m_iFilteredPrefabIDsCount;
	}
	
	protected void AnimateCardFadeIn(Widget cardWidget)
	{
		WidgetAnimator.PlayAnimation(new WidgetAnimationOpacity(cardWidget, m_iCardFadeInSpeed, 1.0));
	}
	
	protected ResourceName GetPrefab(int prefabID)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent));
		if (!placingManager) return ResourceName.Empty;
		
		SCR_PlacingEditorComponentClass placingPrefabData = SCR_PlacingEditorComponentClass.Cast(placingManager.GetEditorComponentData());
		if (!placingPrefabData) return ResourceName.Empty;
		
		return placingPrefabData.GetPrefab(prefabID);
	}
	
	protected void ToggleBetweenCardsAndFilters()
	{
		if (m_bFocusingAssetCard)
			SwitchToFilters();
		else
			SwitchToAssetCards();
	}
	
	protected void SwitchToAssetCards()
	{
		if (!m_wFocusedCard)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) 
			return;

		workspace.SetFocusedWidget(m_wFocusedCard);
	}
	
	protected void SwitchToFilters()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) 
			return;
			
		if (m_LabelsUIComponent)
			workspace.SetFocusedWidget(m_LabelsUIComponent.GetLastFocusedLabel());
	}
	
	protected void GamePadSwitchToggleSearch()
	{
		if (m_bSearchIsToggled)
			GamePadToggleSearch(false, false);
		else
			GamePadToggleSearch(true, false);
	}
	
	protected void GamePadToggleSearch(bool toggle, bool forceToggle = true)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) 
			return;
		
		Widget focusedWidget = workspace.GetFocusedWidget();
		if (!forceToggle && (focusedWidget == m_SearchEditBox.GetRootWidget() || focusedWidget == m_SearchEditBox.m_wEditBox))
		{
			m_SearchEditBox.m_wEditBox.ActivateWriteMode();
		}
		else if (m_bSearchIsToggled != toggle) 
		{
			m_bSearchIsToggled = toggle;
			
			if (m_bSearchIsToggled)
			{			
				workspace.SetFocusedWidget(m_SearchEditBox.GetRootWidget());
				m_SearchEditBox.m_wEditBox.ActivateWriteMode();
			}
			else 
			{
				workspace.SetFocusedWidget(m_wFocusedCard);
			}
		}
		
	}
	
	protected void ResetSearch()
	{
		if (m_SearchEditBox)
		{
			m_SearchEditBox.SetValue(string.Empty);	
		}
		m_ContentBrowserManager.SetLastSearch(m_sLastSearch);
		m_sLastSearch = string.Empty;
		
		if (m_bUsingGamePad)
			GamePadToggleSearch(false);
	}
	
	protected bool GetEntityPreviewBudgetCosts(int prefabID, out notnull array<ref SCR_EntityBudgetValue> budgetCosts)
	{		
		SCR_EditableEntityUIInfo editableUIInfo = m_ContentBrowserManager.GetInfo(prefabID);
		if (!editableUIInfo || !m_BudgetManager)
		{
			return false;
		}
		
		return m_BudgetManager.GetEntityPreviewBudgetCosts(editableUIInfo, budgetCosts));
	}
	
	protected void RefreshBudgetPreviewCost(int prefabID = -1)
	{
		if (m_PlacingManager.HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER) || prefabID == -1)
		{
			m_BudgetManager.ResetPreviewCost();
			return;
		}
		
		array<ref SCR_EntityBudgetValue> budgetCosts = {};
		if (GetEntityPreviewBudgetCosts(prefabID, budgetCosts))
		{
			m_BudgetManager.UpdatePreviewCost(budgetCosts);
		}
	}
	
	protected void FilterEntries(bool resetPage = true, bool resetSearch = true)
	{
		SCR_EditableEntityComponent extendedEntity = m_ContentBrowserManager.GetExtendedEntity();
		bool isExtending = extendedEntity != null;
		ResourceName extendedPrefab;
		bool isInheritedSlot;
		if (isExtending)
		{
			extendedPrefab = extendedEntity.GetPrefab();
			isInheritedSlot = extendedEntity.HasEntityFlag(EEditableEntityFlag.SLOT);
		}
		
		m_aFilteredPrefabIDs.Clear();
		m_aLocalizationKeys.Clear();
		array<EEditableEntityLabel> entityLabels = {};
		SCR_EditableEntityUIInfo info;
		int count = m_ContentBrowserManager.GetInfoCount();
		for (int i = 0; i < count; i++)
		{
			entityLabels.Clear();
			info = m_ContentBrowserManager.GetInfo(i);
			
			if (!info)
			{
				continue;
			}
			
			if (isExtending)
			{
				if (m_PrefabsCache && isInheritedSlot)
				{
					if (!m_PrefabsCache.IsPrefabInherited(info.GetSlotPrefab(), extendedPrefab))
						continue;
				}
				else
				{
					if (info.GetSlotPrefab() != extendedPrefab)
						continue;
				}
			}
			else
			{
				//--- Browser opened generically - ignore assets on fixed position
				if (info.HasEntityFlag(EEditableEntityFlag.STATIC_POSITION) && extendedPrefab != extendedPrefab)
					continue;
			}
			
			info.GetEntityLabels(entityLabels);
			if (!m_ContentBrowserManager.IsMatchingToggledLabels(entityLabels))
				continue;
			
 			m_aFilteredPrefabIDs.Insert(i);
			
			m_aLocalizationKeys.Insert(info.GetName());
		}
		m_iFilteredPrefabIDsCount = m_aFilteredPrefabIDs.Count();
		
		if (resetSearch)
		{
			ResetSearch();
		}
		
		if (resetPage)
		{
			SetPage(0);	
		}
		else
		{
			SetPage(m_ContentBrowserManager.GetPageIndex());
		}
		m_bBudgetPreviewUpdateEnabled = true;
	}
	
	protected void OnInputDeviceChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		m_bUsingGamePad = (newDevice == EInputDeviceType.GAMEPAD || newDevice == EInputDeviceType.JOYSTICK);
		
		if (m_bFocusingAssetCard)
			OnCardHover(null, -1, true);
		else
			OnCardHover(null, -1, false);
		
		
		if (!m_SearchEditBox)
			return;
		
		if (!m_bUsingGamePad)
		{
			GamePadToggleSearch(false);
		}
		else 
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (!workspace) 
				return;
			
			workspace.SetFocusedWidget(m_SearchEditBox.GetRootWidget());
		}
	}
	
	protected void OnCardHover(Widget card, int prefabIndex, bool hover)
	{
		if (!hover)
		{
			RefreshBudgetPreviewCost();
			return;
		}
		
		if (!m_bBudgetPreviewUpdateEnabled || m_bUsingGamePad) return;
		
		RefreshBudgetPreviewCost(prefabIndex);
	}
	
	protected void OnCardFocus(Widget card, int prefabIndex, bool focus)
	{	
		if (!m_bUsingGamePad) return;
		
		m_bFocusingAssetCard = focus;
		
		if (m_bFocusingAssetCard)
		{
			m_wToggleCardFilterButton.SetLabel("#AR-Editor_ContentBrowser_SwitchToFilters");
		}
		else
		{
			m_wToggleCardFilterButton.SetLabel("#AR-Editor_ContentBrowser_SwitchToCards");
			RefreshBudgetPreviewCost();
			return;
		}
		
		if (card)
		{
			m_wFocusedCard = card;
			m_iCardFocusIndex = prefabIndex;
			RefreshBudgetPreviewCost(prefabIndex);
		}
	}
	
	protected void OnSearchFocusChanged(SCR_EditBoxComponent editBoxComponent, EditBoxWidget editBoxWidget, bool focus)
	{
		if (focus != m_bSearchIsToggled)
			m_bSearchIsToggled = focus	
	}
	
	protected void OnSearchConfirmed(SCR_EditBoxComponent editBox, string text)
	{		
		string search = text.Trim();
		if (search == m_sLastSearch)
		{
			return;
		}
		m_sLastSearch = search;
		m_bAnimateEntries = true;
		
		FilterEntries(false, false);
		
		if (!m_sLastSearch.IsEmpty())
		{
			array<int> searchResultPrefabID = {};
			array<int> searchResultIndices = {};
			WidgetManager.SearchLocalized(m_sLastSearch, m_aLocalizationKeys, searchResultIndices);
			
			foreach (int searchResultIndex : searchResultIndices)
			{
				int prefabID = m_aFilteredPrefabIDs.Get(searchResultIndex);
				searchResultPrefabID.Insert(prefabID);
			}
			
			m_aFilteredPrefabIDs.Copy(searchResultPrefabID);
			m_iFilteredPrefabIDsCount = m_aFilteredPrefabIDs.Count();
			m_bAnimateEntries = true;
			
			int pageIndex = 0;
			if (m_ContentBrowserManager)
			{
				pageIndex = m_ContentBrowserManager.GetPageIndex();
			}
			SetPage(pageIndex);
		}
		
		//If using gamepad make sure search box is no longer selected
		if (m_bUsingGamePad && m_wFocusedCard)
			GamePadToggleSearch(false);
	}
	
	protected void OnMenuClosed()
	{
		if (m_BudgetManager && m_bBudgetPreviewUpdateEnabled)
		{
			m_BudgetManager.ResetPreviewCost();
		}
		ResetSearch();
	}
	
	protected void OnResetClicked()
	{
		m_LabelsUIComponent.OnResetClicked();
		ResetSearch();
	}
	
	protected void OnShowLoadingCircle(bool state = true)
	{
		if (m_bContentLoadingWidgetActive == state)
		{
			return;
		}
		m_ContentLoadingVisual.SetVisible(state);
		m_aFilteredPrefabIDs.Clear();
		m_iFilteredPrefabIDsCount = 0;
		m_bContentLoadingWidgetActive = state;
		SetPage(0);
	}
	
	protected void OnApplyLabelChanges()
	{
		OnShowLoadingCircle(false);
		m_bAnimateEntries = true;
		FilterEntries(true, false);
	}
	
	protected void InitializeLabels()
	{
		Widget labelsWidget = GetWidget().FindAnyWidget(m_sLabelComponentWidget);
		if (!labelsWidget) return;
		
		m_LabelsUIComponent = SCR_ContentBrowserFiltersEditorUIComponent.Cast(labelsWidget.FindHandler(SCR_ContentBrowserFiltersEditorUIComponent));		
		if (!m_LabelsUIComponent) return;
		
		m_LabelsUIComponent.SetContentBrowserCallbacks(m_ShowLoadingCircleCallback, m_LabelApplyCallback);
		
		Widget activeFilters = GetWidget().FindAnyWidget(m_sActiveFiltersName);
		if (activeFilters)
		{
			SCR_ContentBrowserActiveFiltersEditorUIComponent activeFilterComponent = SCR_ContentBrowserActiveFiltersEditorUIComponent.Cast(activeFilters.FindHandler(SCR_ContentBrowserActiveFiltersEditorUIComponent));
			
			if (activeFilterComponent)
				activeFilterComponent.Init(m_LabelsUIComponent);
		}
	}
	
	override protected bool IsUnique()
	{
		return true;
	}
	
	protected override void OnButtonPrev()
	{
		if (m_bPlayAudioOnPageChange)
			SCR_UISoundEntity.SoundEvent(m_sOnePrevPageSfx, true);
			
			SetPage(m_iCurrentPage - 1);			
	}
	
	protected override void OnButtonNext()
	{
		if (m_bPlayAudioOnPageChange)
			SCR_UISoundEntity.SoundEvent(m_sOnePrevPageSfx, true);
			
		SetPage(m_iCurrentPage + 1);
	}
	
	override protected void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		InputManager inputManager =  GetGame().GetInputManager();
		
		Widget toggleBetweenCardAndFilter = w.FindAnyWidget(m_sToggleCardFilterButton);
		
		if (toggleBetweenCardAndFilter)
		{
			ButtonActionComponent.GetOnAction(toggleBetweenCardAndFilter, false, 0).Insert(ToggleBetweenCardsAndFilters);
			m_wToggleCardFilterButton = SCR_NavigationButtonComponent.Cast(toggleBetweenCardAndFilter.FindHandler(SCR_NavigationButtonComponent));
		}
		
		// Disable budget preview update during loading, prevent OnCardHover callbacks during init
		m_bBudgetPreviewUpdateEnabled = false;
		
		if (SCR_Global.IsEditMode()) return;
		
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!m_EntityCore)
			return;
		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (!m_ContentBrowserManager)
			return;
		
		m_PlacingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true));
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent));
		m_PrefabsCache = SCR_PrefabsCacheEditorComponent.Cast(SCR_PrefabsCacheEditorComponent.GetInstance(SCR_PrefabsCacheEditorComponent));	
		m_ContentBrowserDialog = EditorBrowserDialogUI.Cast(GetMenu());
		m_ContentBrowserDialog.GetOnMenuHide().Insert(OnMenuClosed);
		
		m_NoFilterResultsWidget = w.FindAnyWidget(m_sNoFilterResultsWidgetName);
				
		Widget resetFiltersButton = w.FindAnyWidget(m_sResetFiltersButtonName);
		if (resetFiltersButton)
			ButtonActionComponent.GetOnAction(resetFiltersButton, false, 0).Insert(OnResetClicked);		
		
		m_SearchEditBox = SCR_EditBoxComponent.GetEditBoxComponent(m_sSearchEditBoxName, w, true);
		if (m_SearchEditBox)
		{
			m_SearchEditBox.m_OnConfirm.Insert(OnSearchConfirmed);
			m_SearchEditBox.m_OnFocusChangedEditBox.Insert(OnSearchFocusChanged);
		}
		
		m_ContentLoadingVisual = w.FindAnyWidget(m_sContentLoadingWidgetName);
		if (!m_ContentLoadingVisual)
			Print("'SCR_ContentBrowserEditorUIComponent' cannot find 'm_ContentLoadingVisual'!", LogLevel.WARNING);
		
		m_ShowLoadingCircleCallback.Insert(OnShowLoadingCircle);
		m_LabelApplyCallback.Insert(OnApplyLabelChanges);
		
		m_wToggleSearchButton = w.FindAnyWidget(m_sToggleSearchButton);
		if (m_wToggleSearchButton)
			ButtonActionComponent.GetOnAction(m_wToggleSearchButton, false, 0).Insert(GamePadSwitchToggleSearch);
		
		//Input device changed
		if (inputManager)
		{
			OnInputDeviceChanged(inputManager.GetLastUsedInputDevice(), inputManager.GetLastUsedInputDevice());
		}
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChanged);
		
		InitializeLabels();
		
		string lastSearch = m_ContentBrowserManager.GetLastSearch();
		if (!lastSearch.IsEmpty())
		{
			m_SearchEditBox.SetValue(lastSearch);
			OnSearchConfirmed(m_SearchEditBox, lastSearch);
		}
		else
		{
			m_bAnimateEntries = true;
			FilterEntries(false, true);
		}
		
		SCR_UISoundEntity.SoundEvent(m_sSfxOnOpenDialog, true);
		RefreshBudgetPreviewCost();
	}
	
	override protected void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		//Input device changed
		GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputDeviceChanged);
		
		if (SCR_Global.IsEditMode()) return;
		
		m_ShowLoadingCircleCallback.Remove(OnShowLoadingCircle);
		m_LabelApplyCallback.Remove(OnApplyLabelChanges);
		
		m_ContentBrowserDialog.GetOnMenuHide().Remove(OnMenuClosed);
		
		if (m_ContentBrowserManager)
		{
			m_ContentBrowserManager.SetPageIndex(GetCurrentPage());
			m_ContentBrowserManager.SetExtendedEntity(null);
		}
		
		if (m_SearchEditBox)
		{
			m_SearchEditBox.m_OnConfirm.Remove(OnSearchConfirmed);
		}
		
		if (!m_bCardSelected)
			SCR_UISoundEntity.SoundEvent(m_sSfxOnCloseDialog, true);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_EntityType")]
class SCR_ContentBrowserEditorCard
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	EEditableEntityType m_EntityType;
	
	[Attribute(params: "layout")]
	ResourceName m_sPrefab;
};