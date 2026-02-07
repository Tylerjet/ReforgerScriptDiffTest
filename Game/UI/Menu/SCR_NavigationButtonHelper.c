class SCR_NavigationButtonHelper
{
	static ref ScriptInvoker<bool> m_OnActiveWidgetInteraction = new ScriptInvoker();
	static ref ScriptInvoker<DialogUI> m_OnDialogOpen = new ScriptInvoker();
	static ref ScriptInvoker<DialogUI> m_OnDialogClose = new ScriptInvoker();
		
	//------------------------------------------------------------------------------------------------
	//! Navigation buttons are bound to this delegate to disable themselves during edit/combo boxes interaction and prevent reenabling
	//! Edit boxes and dropdown menus call this method as a substitute for the lack of events when starting and finishing txt editing / dropdown interaction
	static void SetActiveWidgetInteractionState(bool isActive, int delay = 0)
	{			
		m_OnActiveWidgetInteraction.Invoke(isActive, delay);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Methods to keep track of current dialogs. 
	//! The MenuManager does not keep track of dialogs: GetTopMenu() only returns the last menu opened with OpenMenu(), not those opened with OpenDialog()
	//! Navigation buttons bind themselves to m_OnDialogOpen and m_OnDialogClose
	//! Called by DialogUI
	static void OnDialogOpen(DialogUI dialog)
	{
		m_OnDialogOpen.Invoke(dialog);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called by DialogUI.
	static void OnDialogClose(DialogUI dialog)
	{	
		m_OnDialogClose.Invoke(dialog);
	}
};