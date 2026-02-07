//------------------------------------------------------------------------------------------------	
class SCR_ScriptPlatformRequestCallback : ScriptPlatformRequestCallback
{
	ref ScriptInvoker m_OnResult = new ScriptInvoker; // (UserPrivilege privilege, UserPrivilegeResult result)
	
	
	//------------------------------------------------------------------------------------------------
	override void OnPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		m_OnResult.Invoke(privilege, result);
	}	
};

////////////////////////////////////////////////////////////
// Callbacks for specific workshop requests
////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------
// WorkshopApi.RequestPage
class SCR_WorkshopApiCallback_RequestPage : SCR_BackendCallback
{
	int m_iPageId;

	void SCR_WorkshopApiCallback_RequestPage(int pageId)
	{
		m_iPageId = pageId;
		
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 New: page: %2", this, this.m_iPageId), LogLevel.DEBUG);
		#endif
	}
	
	void ~SCR_WorkshopApiCallback_RequestPage(ContentBrowserUI contentBrowser, int tabId, int pageId)
	{
		#ifdef WORKSHOP_DEBUG
		Print(string.Format("%1 Delete: page: %2", this, this.m_iPageId), LogLevel.DEBUG);
		#endif
	}
};

//-----------------------------------------------------------------------------------------------
// WorkshopItem.AskDetails
class SCR_WorkshopItemCallback_AskDetails : SCR_BackendCallback
{
	WorkshopItem m_Item;
	
	//-----------------------------------------------------------------------------------------------
	void SCR_WorkshopItemCallback_AskDetails(WorkshopItem item)
	{
		m_Item = item;
	}
};

//-----------------------------------------------------------------------------------------------
// ImageScale.Download
class SCR_WorkshopItemCallback_DownloadImage : SCR_BackendCallback
{
	ImageScale m_Scale;
};