/*
Component to be attached to addon lines.
*/

//----------------------------------------------------------------------------------------------
class SCR_WorkshopAddonLineComponent : ScriptedWidgetComponent
{
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	protected string m_sSoundHovered;

	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	protected string m_sSoundClicked;
	
	protected ref SCR_AddonLineWidgets m_Widgets = new SCR_AddonLineWidgets();
	protected Widget m_wRoot;	
	protected ref SCR_WorkshopItem m_Item;
	protected bool m_bMouseOver;
	protected bool m_bFocused;
	protected ref SCR_WorkshopUiCommon_DownloadSequence m_DownloadRequest;
	
	protected bool m_bCanUpdate = true;
	
	ref ScriptInvoker m_OnEnableButton = new ScriptInvoker();
	ref ScriptInvoker m_OnDisableButton = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseEnter = new ScriptInvoker();
	ref ScriptInvoker m_OnMouseLeave = new ScriptInvoker();
	ref ScriptInvoker m_OnFocus = new ScriptInvoker();
	ref ScriptInvoker m_OnFocusLost = new ScriptInvoker();
	
	protected SCR_MultiActionButtonComponent m_ActionButtons;
	
	//----------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem() {return m_Item;}
	
	//----------------------------------------------------------------------------------------------
	Widget GetRootWidget() { return m_wRoot; }
	
	//----------------------------------------------------------------------------------------------
	bool IsWorkshopItemEnabled()
	{
		if (!m_Item)
			return false;
		
		return m_Item.GetEnabled();
	}
	
	//----------------------------------------------------------------------------------------------
	void Init(SCR_WorkshopItem item)
	{
		m_Item = item;
		
		m_ActionButtons = SCR_MultiActionButtonComponent.Cast(m_wRoot.FindHandler(SCR_MultiActionButtonComponent));
		UpdateAllWidgets();
		
		// OnChanged is called whenever something happens with the workshop item.
		// Simplest approach is to refresh whole line associated with the workshop item on this event.
		item.m_OnChanged.Insert(UpdateAllWidgets);
	}
	
	
	//----------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{	
		if (!m_bCanUpdate)
			return;
		
		if (!m_Item)
			return;
	
		bool mouseOver = m_bMouseOver;
			
		// Update name
		m_Widgets.m_NameText.SetText(m_Item.GetName());
		
		
		bool enabled = m_Item.GetEnabled();
		
		m_Widgets.m_SizeMoveRight.SetVisible(!enabled);
		m_Widgets.m_MoveRightButton.SetVisible(!enabled && m_bMouseOver || m_bFocused);
		
		m_Widgets.m_SizeMoveLeft.SetVisible(enabled);
		m_Widgets.m_MoveLeftButton.SetVisible(enabled && m_bMouseOver || m_bFocused);
		
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
		/*else if (problem)
		{
			stateText = problemDescription;
			m_Widgets.m_StateText.SetVisible(mouseOver); // Those texts are too big, had to disable
		}*/
		else
			m_Widgets.m_HorizontalState.SetVisible(false);
		
		m_Widgets.m_StateText.SetText(stateText);

		
		// Update the action button text
		/*string actionText = SCR_WorkshopUiCommon.GetPrimaryActionName(m_Item);
		m_Widgets.m_ActionButton.SetVisible(!actionText.IsEmpty());
		if (!actionText.IsEmpty())
		{
			SCR_ButtonEffectText e = SCR_ButtonEffectText.Cast(m_Widgets.m_ActionButtonComponent.FindEffect("text"));
			e.m_sDefault = actionText;
			e.m_sHovered = actionText;
			e.OnStateDefault(true); // todo remake the fn to reapply effects
		}*/
		
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
	protected bool GetMouseOver()
	{
		// Check if mouse is over the line
		int xInt, yInt;
		WidgetManager.GetMousePos(xInt, yInt);
		float mx = xInt;
		float my = yInt;
		
		float posx, posy, sizex, sizey;
		m_wRoot.GetScreenPos(posx, posy);
		m_wRoot.GetScreenSize(sizex, sizey);
		
		bool mouseOver =	(mx >= posx) && (mx <= posx + sizex) &&
							(my >= posy) && (my <= posy + sizey);
		
		return mouseOver;
	}
	
	
	//----------------------------------------------------------------------------------------------
	void OnDeleteButton()
	{
		if (!m_Item)
			return;
		
		SCR_DeleteAddonDialog.CreateDeleteAddon(m_Item);
	}
	
	
	//----------------------------------------------------------------------------------------------
	void OnOpenDetailsButton()
	{
		if (!m_Item)
			return;
		
		ContentBrowserDetailsMenu.OpenForWorkshopItem(m_Item);
	}
	
	
	//----------------------------------------------------------------------------------------------
	void OnActionButton()
	{
		if (!m_Item)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}
	
	
	//----------------------------------------------------------------------------------------------
	void OnEnableButton()
	{
		if (!m_Item)
			return;
		
		m_OnEnableButton.Invoke(this);
		
		// Enabled dependencies 
		/*array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		foreach (SCR_WorkshopItem dep : dependencies)
			dep.SetEnabled(true);*/
	}
	
	
	//----------------------------------------------------------------------------------------------
	void OnDisableButton()
	{
		if (!m_Item)
			return;
		
		m_OnDisableButton.Invoke(this);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnUpdateButton()
	{
		if (!m_Item)
			return;
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}
	
	//----------------------------------------------------------------------------------------------
	void OnFixButton()
	{
		if (!m_Item)
			return;
		
		// All dependencies
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		if (dependencies.IsEmpty())
			return;
		
		// Missing dependencies
		dependencies = SCR_AddonManager.SelectItemsOr(dependencies, EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.UPDATE_AVAILABLE);
		if (dependencies.IsEmpty())
			return;
		
		// Create addon map and dialog 
		array<ref Tuple2<SCR_WorkshopItem, string>> addonsAndVersions = {};		
		foreach (SCR_WorkshopItem item : dependencies)
			addonsAndVersions.Insert(new Tuple2<SCR_WorkshopItem, string>(item, string.Empty));
		
		SCR_DownloadConfirmationDialog.CreateForAddons(addonsAndVersions, true);
	}
	
	//----------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		if (!SCR_Global.IsEditMode())
		{
			m_Widgets.Init(w);
			
			m_Widgets.m_DeleteButtonComponent.m_OnClicked.Insert(OnDeleteButton);
			m_Widgets.m_OpenDetailsButtonComponent.m_OnClicked.Insert(OnOpenDetailsButton);
			//m_Widgets.m_ActionButtonComponent.m_OnClicked.Insert(OnActionButton);
			m_Widgets.m_MoveRightButtonComponent.m_OnClicked.Insert(OnEnableButton);
			m_Widgets.m_MoveLeftButtonComponent.m_OnClicked.Insert(OnDisableButton);
			
			// Fix update 
			m_Widgets.m_UpdateButtonComponent.m_OnClicked.Insert(OnUpdateButton);
			m_Widgets.m_FixButtonComponent.m_OnClicked.Insert(OnFixButton);
		}
	}
	
	
	//----------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		if (m_Item)
			m_Item.m_OnChanged.Remove(UpdateAllWidgets); // Remember to unsubscribe from OnChanged event when this UI component is destroyed!
	}
	
	//----------------------------------------------------------------------------------------------
	void EnableUpdate(bool enable)
	{
		m_bCanUpdate = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string sound)
	{
		if (sound != string.Empty)
			SCR_UISoundEntity.SoundEvent(sound);
	}
}
