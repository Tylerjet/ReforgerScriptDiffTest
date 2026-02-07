/*!
Class for addon line button base. 
This component just vizualize state and invoke interaction. 
Items are not manipulated in this component.
*/

class SCR_AddonLineBaseComponent : ScriptedWidgetComponent
{
	// Attributes
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	protected string m_sSoundHovered;

	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSoundClicked;
	
	// Fields  
	protected ref SCR_AddonLineWidgets m_Widgets = new SCR_AddonLineWidgets();
	protected Widget m_wRoot;
	
	protected ref SCR_WorkshopItem m_Item;
	
	protected bool m_bMouseOver;
	protected bool m_bFocused;
	protected bool m_bCanUpdate = true;
	
	ref ScriptInvoker m_OnEnableButton = new ScriptInvoker();
	ref ScriptInvoker m_OnDisableButton = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseEnter = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();
	ref ScriptInvoker m_OnFocus = new ScriptInvoker();
	ref ScriptInvoker m_OnFocusLost = new ScriptInvoker();
	
	protected SCR_MultiActionButtonComponent m_ActionButtons;
	
	//----------------------------------------------------------------------------------------------
	// Override
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		if (!SCR_Global.IsEditMode())
		{
			m_Widgets.Init(w);
			
			// Button callbacks
			m_Widgets.m_DeleteButtonComponent.m_OnClicked.Insert(OnDeleteButton);
			m_Widgets.m_OpenDetailsButtonComponent.m_OnClicked.Insert(OnOpenDetailsButton);
			m_Widgets.m_MoveRightButtonComponent.m_OnClicked.Insert(OnEnableButton);
			m_Widgets.m_MoveLeftButtonComponent.m_OnClicked.Insert(OnDisableButton);
			
			// Fix update 
			m_Widgets.m_UpdateButtonComponent.m_OnClicked.Insert(OnUpdateButton);
			m_Widgets.m_FixButtonComponent.m_OnClicked.Insert(OnFixButton);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	// Custom mouse enter/leave events, because when mouse enters one of buttons inside
	// the native OnMouseLeave event is fired
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bMouseOver = true;
		UpdateAllWidgets();
		m_OnMouseEnter.Invoke(this);
		
		PlaySound(m_sSoundHovered);
		
		return true;
	}
	

	//----------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bMouseOver = false;
		UpdateAllWidgets();
		m_OnMouseLeave.Invoke(this);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// Accept only LMB as valid click
		if (button != 0)
			return false;

		PlaySound(m_sSoundClicked);
		return false;
	}
	
	//----------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_bFocused = true;
		UpdateAllWidgets();
		m_OnFocus.Invoke(this);
		
		PlaySound(m_sSoundHovered);
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bFocused = false;
		UpdateAllWidgets();
		m_OnFocusLost.Invoke(this);
		return false;
	}
	
	//----------------------------------------------------------------------------------------------
	// Protected 
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	//! Update vizual state of line
	protected void UpdateAllWidgets()
	{	
		if (!m_bCanUpdate)
			return;
		
		if (!m_Item)
			return;
	
		bool mouseOver = m_bMouseOver;
			
		// Update name
		m_Widgets.m_NameText.SetText(m_Item.GetName());
		
		//bool enabled = m_Item.GetEnabled();
		
		/*m_Widgets.m_SizeMoveRight.SetVisible(!enabled);
		m_Widgets.m_MoveRightButton.SetVisible(!enabled && m_bMouseOver || m_bFocused);
		
		m_Widgets.m_SizeMoveLeft.SetVisible(enabled);
		*/
		
		m_Widgets.m_MoveRightButton.SetVisible(m_bMouseOver || m_bFocused);
		m_Widgets.m_MoveLeftButton.SetVisible(m_bMouseOver || m_bFocused);
		
		//HandleEnableButtons(false);
		
		// Update state text
		string stateText;
		bool downloading = m_Item.GetDownloadAction() || m_Item.GetDependencyCompositeAction();
		bool problemCritical;
		string problemDescription;
		//bool problem = SCR_WorkshopUiCommon.GetHighestPriorityProblemDescription(m_Item, problemDescription, problemCritical);
		if (downloading)
		{
			float progress = SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);
			stateText = string.Format("%1%%", Math.Round(100.0*progress));
			m_Widgets.m_HorizontalState.SetVisible(true);
			m_Widgets.m_UpdateButton.SetVisible(false);
		}
		else
		{
			m_Widgets.m_HorizontalState.SetVisible(false);
		}
		
		m_Widgets.m_StateText.SetText(stateText);

		EWorkshopItemProblem problem = m_Item.GetHighestPriorityProblem();
		
		
		m_Widgets.m_FixButton.SetVisible(
			problem == EWorkshopItemProblem.BROKEN ||
			problem == EWorkshopItemProblem.DEPENDENCY_MISSING ||
			problem == EWorkshopItemProblem.DEPENDENCY_OUTDATED
		);
		
		m_Widgets.m_UpdateButton.SetVisible(problem == EWorkshopItemProblem.UPDATE_AVAILABLE);
		
		// Update buttons 
		if (m_ActionButtons) 
			m_ActionButtons.ShowAllButtons(m_bMouseOver);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string sound)
	{
		if (sound != string.Empty)
			SCR_UISoundEntity.SoundEvent(sound);
	}
	
	//----------------------------------------------------------------------------------------------
	void HandleEnableButtons(bool enabled)
	{
		m_Widgets.m_SizeMoveRight.SetVisible(!enabled);
		m_Widgets.m_MoveRightButton.SetVisible(!enabled && m_bMouseOver || m_bFocused);
		
		m_Widgets.m_SizeMoveLeft.SetVisible(enabled);
		m_Widgets.m_MoveLeftButton.SetVisible(enabled && m_bMouseOver || m_bFocused);
	}
	
	//----------------------------------------------------------------------------------------------
	// Callbacks 
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	void OnDeleteButton(){}
	
	
	//----------------------------------------------------------------------------------------------
	void OnOpenDetailsButton()
	{
		if (!m_Item)
			return;
		
		ContentBrowserDetailsMenu.OpenForWorkshopItem(m_Item);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnActionButton(){}
	
	//----------------------------------------------------------------------------------------------
	void OnEnableButton()
	{
		if (!m_Item)
			return;
		
		// Check dependencies
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		
		if (!dependencies.IsEmpty())
		{	
			// Check missing dependencies
			dependencies = SCR_AddonManager.SelectItemsOr(dependencies, EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.UPDATE_AVAILABLE);
			
			// Show download dialog 
			if (!dependencies.IsEmpty())
			{
				OnFixButton();
				return;
			}
		}
		
		// Enable
		m_OnEnableButton.Invoke(this);
		HandleEnableButtons(true);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnDisableButton()
	{
		if (!m_Item)
			return;
		
		m_OnDisableButton.Invoke(this);
		HandleEnableButtons(false);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnUpdateButton(){}
	
	//----------------------------------------------------------------------------------------------
	//! Call on fix button click, will provide solution to missing mods
	void OnFixButton()
	{
		if (!m_Item)
			return;
		
		// All dependencies
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		if (dependencies.IsEmpty())
			return;
		
		// Reported and blocked dependencies 
		array<ref SCR_WorkshopItem> issues = SCR_AddonManager.SelectItemsOr(dependencies,
		EWorkshopItemQuery.BLOCKED | EWorkshopItemQuery.AUTHOR_BLOCKED | EWorkshopItemQuery.REPORTED_BY_ME);
		
		if (!issues.IsEmpty())
		{
			// Show dialog of issues 
			SCR_AddonListDialog.CreateRestrictedAddonsDownload(issues);
			return;
		}
		
		// Missing dependencies
		dependencies = SCR_AddonManager.SelectItemsOr(dependencies, EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.UPDATE_AVAILABLE);
		if (dependencies.IsEmpty())
			return;
		
		array<ref SCR_DownloadManager_Entry> downloads = new array<ref SCR_DownloadManager_Entry>;
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		// Remove mods in downloading 
		for (int i = dependencies.Count() - 1; i >= 0; i--)
		{
			foreach (SCR_DownloadManager_Entry download : downloads)
			{
				if (download.m_Item.GetId() == dependencies[i].GetId())
				{
					if (download.m_Action.IsActive())
					{
						dependencies.RemoveItem(dependencies[i]);
						break;
					}
				}
			}
		}
		
		// Open download dialog if all are being downloaded
		if (dependencies.IsEmpty())
		{
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.DownloadManagerDialog);
			return;
		}
		
		// Create addon list and dialog 
		array<ref Tuple2<SCR_WorkshopItem, string>> addonsAndVersions = {};		
		foreach (SCR_WorkshopItem item : dependencies)
			addonsAndVersions.Insert(new Tuple2<SCR_WorkshopItem, string>(item, string.Empty));
		
		SCR_DownloadConfirmationDialog.CreateForAddons(addonsAndVersions, true);
	}
	
	//----------------------------------------------------------------------------------------------
	// API
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	//! Setup line
	void Init(SCR_WorkshopItem item)
	{
		m_Item = item;
		
		m_ActionButtons = SCR_MultiActionButtonComponent.Cast(m_wRoot.FindHandler(SCR_MultiActionButtonComponent));
		UpdateAllWidgets();
	}
	
	//----------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}
	
	//----------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//----------------------------------------------------------------------------------------------
	void EnableUpdate(bool enable)
	{
		m_bCanUpdate = enable;
	}	
}