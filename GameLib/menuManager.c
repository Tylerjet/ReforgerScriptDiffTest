typedef func MenuItemCallback;
void MenuItemCallback(MenuBase menu = null, string menuItem = "", bool changed = false, bool finished = false); 

enum DialogPriority
{
	INFORMATIVE,
	WARNING,
	CRITICAL	
}

enum DialogResult
{
	PENDING,
	OK,
	YES,
	NO,
	CANCEL,
}

enum ScriptMenuPresetEnum
{
	
}

enum GameLibMenusEnum : ScriptMenuPresetEnum
{
	
}

class MenuManager
{
	proto native MenuBase OpenMenu(ScriptMenuPresetEnum preset, int userId = 0, bool unique = false, bool hideParentMenu = true);
	proto native MenuBase OpenDialog(ScriptMenuPresetEnum preset, int priority = DialogPriority.INFORMATIVE, int iUserData = 0, bool unique = false);
	proto native MenuBase FindMenuByPreset(ScriptMenuPresetEnum preset);
	proto native MenuBase FindMenuByUserData(int userId);
	proto native MenuBase GetTopMenu();
	proto native MenuBase GetOwnerMenu(Widget w);
	
	proto native bool IsAnyMenuOpen();	
	proto native bool IsAnyDialogOpen();

	proto native bool CloseMenuByPreset(ScriptMenuPresetEnum preset);
	proto native bool CloseMenuByUserData(int userId);
	proto native bool CloseMenu(MenuBase menu);
	proto native int CloseAllMenus();
	
	protected void MenuManager();
	protected void ~MenuManager();
}

class MenuBindAttribute
{
	string m_MenuItemName;
	
	void MenuBindAttribute(string menuItemName = "")
	{
		m_MenuItemName = menuItemName;
	}
}

/*! 
Base class for menus in menu manager
order callbacks call:
opening:
1) OnMenuInit - called during load, when MenuManager loading menus config
2) OnMenuOpen - called when menu is beeing open
3) OnMenuShow
4) OnMenuFocusGained
5) OnMenuOpened - called after menu is opened and is ready

loop:
OnMenuUpdate
OnMenuItem

closing:
1) OnMenuFocusLost - called when menu is closed or other menu/dialog going to overlap the menu
2) OnMenuHide
3) OnMenuClose
*/
class MenuBase: ScriptedWidgetEventHandler
{
	proto native int GetUserData();
	proto native Widget GetRootWidget();
	proto external MenuBase BindItem(string menuItemName, MenuItemCallback callback);
	proto native MenuBase SetLabel(string menuItemName, string text);
	proto native Widget GetItemWidget(string menuItemName);
	proto native bool AddMenuItem(string menuItemName, Widget w, string actionName);
	proto native bool RemoveMenuItem(string menuItemName);
	proto native MenuManager GetManager();
	proto native void SetActionContext(string actionContextName);
	proto native bool IsFocused();
	proto native bool IsOpen();
	
	[CallbackMethod()]
	proto native void Close();
	
	[CallbackMethod()]
	void CloseMe() {}
	
	void	OnMenuFocusGained() {}
	void	OnMenuFocusLost() {}
	void	OnMenuShow() {}
	void	OnMenuHide() {}
	void	OnMenuOpen() {}
	void	OnMenuOpened() {}
	void	OnMenuClose() {}
	void	OnMenuInit() {}
	void	OnMenuUpdate(float tDelta) {}
	void	OnMenuItem(string menuItemName, bool changed, bool finished) {}
	
	protected void MenuBase();
	protected void ~MenuBase();
}

class MessageBox: MenuBase
{
	[MenuBindAttribute()]
	void Ok()
	{
		Close();
	}
}
