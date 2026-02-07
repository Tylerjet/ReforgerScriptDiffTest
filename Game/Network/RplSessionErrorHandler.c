//-----------------------------------------------------------------------------
class RplSessionErrorHandler : RplSessionCallbacks
{
	//-----------------------------------------------------------------------------
	void RplSessionErrorHandler()
	{
		RplSession.RegisterCallbacks(this);
	}
	
	//-----------------------------------------------------------------------------
	override void EOnFailed(string msg)
	{
		ref ServerBrowserMenuUI serverBrowser = ServerBrowserMenuUI.Cast(
		GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.ServerBrowserMenu));
		
		if (serverBrowser)
		{
			serverBrowser.SetErrorMessage(msg);
		}
	}

};
