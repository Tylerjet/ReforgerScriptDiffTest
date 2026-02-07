/*!
This is a Chat Info Display / Chat HUD.
There is a Chat Panel prefab Inside the layout.
The Chat HUD opens the chat on a button press, and doesn't do anything more interesting.
*/

class SCR_ChatHud : SCR_InfoDisplay
{
	protected SCR_ChatPanel m_ChatPanel;	
	
	//------------------------------------------------------------------------------------------------
	override void UpdateValues(IEntity owner, float timeSlice)
	{
		if (m_ChatPanel)
		{
			m_ChatPanel.OnUpdateChat(timeSlice);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.AddActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnToggleAction);
		
		if (!m_wRoot)
			return;
		
		Widget wChatPanel = m_wRoot.FindAnyWidget("ChatPanel");
		
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnStopDraw(IEntity owner)
	{
		super.OnStopDraw(owner);
		
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.RemoveActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnToggleAction);
		
		// Widget does not get destroyed by HUD manager when switching characters, so we unregister the chat panel manually.
		SCR_ChatPanelManager.GetInstance().Unregister(m_ChatPanel);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called when Enter key is pressed
	protected void Callback_OnToggleAction()
	{	
		if (!m_ChatPanel)
			return;
		
		if (!m_ChatPanel.IsOpen())
		{
			SCR_ChatPanelManager.GetInstance().OpenChatPanel(m_ChatPanel);
		}
	}
};