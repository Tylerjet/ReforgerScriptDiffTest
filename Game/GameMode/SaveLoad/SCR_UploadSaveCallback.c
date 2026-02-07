class SCR_ServerSaveSessionCallback: DSSessionCallback
{
	protected SCR_MissionStruct m_Struct;
	protected ref SCR_ServerSaveRequestCallback m_RequestCallback;
	
	override void OnLoaded(string fileName)
	{
		m_RequestCallback = new SCR_ServerSaveRequestCallback(fileName, m_Struct);
	}
	override void OnLoadFailed(string fileName)
	{
	}
	void SCR_ServerSaveSessionCallback(string fileName, SCR_MissionStruct struct)
	{
		m_Struct = struct;
		
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		storage.LocalLoad(fileName);
	}
};
class SCR_ServerSaveRequestCallback: BackendCallback
{
	static const string SESSION_SAVE_NAME = "SCR_SaveFileManager_SessionSave";
	
	protected string m_sFileName;
	protected SCR_MissionStruct m_Struct;
	protected ref SCR_UploadSaveCallback_PageParams m_PageParams;
	protected ref SCR_ServerSaveUploadCallback m_UploadCallback;
	
	override void OnSuccess(int code)
	{
		PrintFormat("[SCR_ServerSaveRequestCallback] OnSuccess(): code=%1", code);
		
		if (code == 0)
			return;
		
		WorldSaveItem item;
		WorldSaveManifest manifest = new WorldSaveManifest();
		
		array<WorldSaveItem> items = {};
		if (GetGame().GetBackendApi().GetWorldSaveApi().GetTotalItemCount() > 0)
		{
			//--- Use existing Workshop item
			GetGame().GetBackendApi().GetWorldSaveApi().GetPageItems(items);
			item = items[0];
			
			item.FillManifest(manifest);
			
			manifest.m_aFiles = {m_Struct};
			manifest.m_aFileNames = {m_sFileName};
		}
		else
		{
			//--- Create new Workshop item
			manifest.m_sName = SESSION_SAVE_NAME;
			manifest.m_sSummary = m_sFileName;
			manifest.m_sPreview = "UI/Textures/ScalingTest/Sources/pattern_900x900.jpg";
		
			//manifest.m_aFiles = {m_Struct}; //--- Why though?
			manifest.m_aFileNames = {m_sFileName};
			
			GetGame().GetBackendApi().GetWorldSaveApi().CreateLocalWorldSave(manifest);
		}
		
		m_UploadCallback = new SCR_ServerSaveUploadCallback();
		
		Print(item, LogLevel.VERBOSE);
		item = GetGame().GetBackendApi().GetWorldSaveApi().UploadWorldSave(manifest, m_UploadCallback, item);
		Print(item, LogLevel.DEBUG);
	}
	override void OnError(int code, int restCode, int apiCode)
	{
		PrintFormat("[SCR_ServerSaveRequestCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code));
	}
	override void OnTimeout()
	{
		Print("[SCR_ServerSaveRequestCallback] OnTimeout");
	}
	void SCR_ServerSaveRequestCallback(string fileName, SCR_MissionStruct struct)
	{
		m_sFileName = fileName;
		m_Struct = struct;
		
		m_PageParams = new SCR_UploadSaveCallback_PageParams();
		m_PageParams.limit = 1;
		GetGame().GetBackendApi().GetWorldSaveApi().RequestPage(this, m_PageParams, false);
	}
};
class SCR_UploadSaveCallback_PageParams: PageParams
{
	override void OnPack()
	{
		StoreBoolean("owned", true);
		StoreString("search", SCR_ServerSaveRequestCallback.SESSION_SAVE_NAME);
	}
};

class SCR_ServerSaveUploadCallback: BackendCallback
{
	override void OnSuccess(int code)
	{
		PrintFormat("[SCR_ServerSaveUploadCallback] OnSuccess(): code=%1", code);
	}
	override void OnError(int code, int restCode, int apiCode)
	{
		PrintFormat("[SCR_ServerSaveUploadCallback] OnError: code=%1 ('%4'), restCode=%2, apiCode=%3", code, restCode, apiCode, GetGame().GetBackendApi().GetErrorCode(code));
	}
	override void OnTimeout()
	{
		Print("[SCR_ServerSaveUploadCallback] OnTimeout");
	}
}