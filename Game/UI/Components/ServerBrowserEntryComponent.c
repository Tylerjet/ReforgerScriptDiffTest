// This component handles server entry and visiualization of server data
// 
//------------------------------------------------------------------------------------------------

class ServerBrowserEntryComponent : ScriptedWidgetComponent
{	
	protected Room m_RoomInfo;
	protected Widget m_wRoot;
	
	ref ScriptInvoker Event_OnFocusEnter = new ref ScriptInvoker;
	ref ScriptInvoker Event_OnFocusLeave = new ref ScriptInvoker;
	ref ScriptInvoker m_OnClick = new ref ScriptInvoker();
	ref ScriptInvoker m_OnFavorite = new ref ScriptInvoker;
	
	//------------------------------------------------------------------------------------------------
	// Override API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_OnClick.Invoke(this);		
		return super.OnClick( w, x, y, button ); 
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		Event_OnFocusEnter.Invoke(this);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if ( Event_OnFocusLeave != null )
		{
			Event_OnFocusLeave.Invoke(this);
		}
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		//m_oServerInfo = null;
		Event_OnFocusEnter.Clear();
		Event_OnFocusLeave.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// Public API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void Init(Widget w) { m_wRoot = w; }
	
	//------------------------------------------------------------------------------------------------
	void EnableEntry(bool enable)
	{
		if (enable)
		{
			GetRootWidget().SetOpacity(1);
		}
		else 
		{
			GetRootWidget().SetOpacity(0.75);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup opacity animation 
	void AnimateOpacity(int delay, float animationTime, float opacityEnd, float opacityStart = -1)
	{
		if (opacityStart != -1)
			GetRootWidget().SetOpacity(opacityStart);
		
		GetGame().GetCallqueue().Remove(OpacityAnimation);
		GetGame().GetCallqueue().CallLater(OpacityAnimation, delay, false, animationTime, opacityEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpacityAnimation(int time, float opacityEnd) 
	{
		AnimateWidget.Opacity(GetRootWidget(), opacityEnd, time);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowWidget(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFavorite(bool bFavorite, bool callback){}
	
	//------------------------------------------------------------------------------------------------
	void OnFavoriteClicked(SCR_ButtonBaseComponent button) {}
	
	//------------------------------------------------------------------------------------------------
	// Get & Set API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	Room GetRoomInfo() { return m_RoomInfo; }
	
	//------------------------------------------------------------------------------------------------
	void SetRoomInfo(Room room)
	{
		m_RoomInfo = room;

		string target = room.HostAddress();
		int colon = target.IndexOf(":");	
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget() { return m_wRoot; }
	
};