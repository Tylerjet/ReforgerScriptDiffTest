class SCR_ServerSaveRequestCallback: SCR_BackendCallback
{
	static const string SESSION_SAVE_NAME = "SCR_SaveFileManager_SessionSave";
	
	protected string m_sFileName;
	protected ref SCR_UploadSaveCallback_PageParams m_PageParams;
	protected ref SCR_ServerSaveUploadCallback m_UploadCallback;
	
	protected bool m_bHasData = false;
	
	//----------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		super.OnSuccess(code);	
		Print(string.Format("[SCR_ServerSaveRequestCallback] OnSuccess(): code=%1", code), LogLevel.NORMAL);
		
		if (code == 0)
			return;
		
		// Get saves 
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop(); 
		
		array<WorkshopItem> items = {};
		workshop.GetPageItems(items);
		
		m_bHasData = code == EBackendRequest.EBREQ_WORKSHOP_GetAsset;
		
		WorldSaveItem save;
		WorldSaveManifest manifest = new WorldSaveManifest();
		
		bool isNew = items.IsEmpty();
		if (!isNew)
		{
			// Clear  
			save = WorldSaveItem.Cast(items[0]);
			
			if (m_bHasData)
			{
				save.FillManifest(manifest);
			}
			else
			{
				save.AskDetail(this);
				return;
			}		
		}
		
		//--- Define manifest params
		manifest.m_sName = SESSION_SAVE_NAME;
		manifest.m_sSummary = m_sFileName;
		manifest.m_aFileNames = {m_sFileName};
		
		//--- Create new save from manifest
		if (isNew)
		{
			save = WorldSaveItem.CreateLocalWorldSave(manifest);
		}
		
		//--- Upload file
		m_UploadCallback = new SCR_ServerSaveUploadCallback(save.Id());
		save.UploadWorldSave(manifest, m_UploadCallback);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode)
	{
		super.OnError(code, restCode, apiCode);
		Print(string.Format("[SCR_ServerSaveRequestCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code)), LogLevel.NORMAL);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		super.OnTimeout();
		Print("[SCR_ServerSaveRequestCallback] OnTimeout", LogLevel.NORMAL);
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_ServerSaveRequestCallback(string fileName)
	{
		m_sFileName = fileName;
		
		m_PageParams = new SCR_UploadSaveCallback_PageParams();
		m_PageParams.limit = 1;
		m_PageParams.type = "world-save";
		GetGame().GetBackendApi().GetWorkshop().RequestPage(this, m_PageParams, false);
	}
};

//----------------------------------------------------------------------------------------
class SCR_UploadSaveCallback_PageParams: PageParams
{
	override void OnPack()
	{
		StoreBoolean("owned", true);
		StoreString("search", SCR_ServerSaveRequestCallback.SESSION_SAVE_NAME);
	}
};

//----------------------------------------------------------------------------------------
class SCR_ServerSaveUploadCallback: SCR_BackendCallback
{
	protected string m_sId;
	
	//----------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		Print(string.Format("[SCR_ServerSaveRequestCallback] OnSuccess(): code=%1", code), LogLevel.NORMAL);
		
		BaseChatComponent chatComponent = BaseChatComponent.Cast(GetGame().GetPlayerController().FindComponent(BaseChatComponent));
		if (chatComponent)
			chatComponent.SendMessage(string.Format("#load %1", m_sId), 0);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnError(int code, int restCode, int apiCode)
	{
		Print(string.Format("[SCR_ServerSaveUploadCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code)), LogLevel.NORMAL);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		Print("[SCR_ServerSaveUploadCallback] OnTimeout", LogLevel.NORMAL);
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_ServerSaveUploadCallback(string id)
	{
		m_sId = id;
	}
}