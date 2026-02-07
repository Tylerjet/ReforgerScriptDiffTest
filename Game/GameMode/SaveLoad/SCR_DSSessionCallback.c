/*!
Callback for easy handling of world saving and loading.
Controlled from SCR_SaveLoadComponent.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleObject("m_Struct", "Struct: %1")]
class SCR_DSSessionCallback: DSSessionCallback
{
	protected SCR_MissionStruct m_Struct;
	
	/////////////////////////////////////////////////////////////////////////////
	// Public
	/////////////////////////////////////////////////////////////////////////////
	/*!
	Save current session to backend.
	*/
	void SaveSession(string fileName)
	{
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		
		if (Replication.IsRunning())
		{
			//--- MP
			storage.RequestSave(fileName);
			PrintFormat("RequestSave: $profile:.backend\\%1.json", fileName);
		}
		else
		{
			//--- SP
			storage.LocalSave(fileName);
			PrintFormat("LocalSave: $profile:.backend\\%1.json", fileName);
		}
	}
	/*!
	Load data from backend save.
	*/
	void LoadSession(string fileName)
	{
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		
		// TODO: Solve for Listen servers
		if (RplSession.Mode() == RplMode.Dedicated)
		{
			//--- MP
			storage.RequestLoad(fileName);
			PrintFormat("RequestLoad: $profile:.backed\\%1.json", fileName);
		}
		else if (RplSession.Mode() == RplMode.None)
		{
			//--- SP
			storage.LocalLoad(fileName);
			PrintFormat("LocalLoad: $profile:.backed\\%1.json", fileName);
		}
	}
	/*!
	Print out contents of saved data.
	*/
	void LogSession()
	{
		m_Struct.Log();
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Protected
	/////////////////////////////////////////////////////////////////////////////
	protected bool SaveStruct()
	{
		return m_Struct && m_Struct.Serialize();
	}
	protected bool LoadStruct()
	{
		return m_Struct && m_Struct.Deserialize();
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Override
	/////////////////////////////////////////////////////////////////////////////
	override void OnSaving(string fileName)
	{
		if (SaveStruct())
			GetGame().GetBackendApi().GetStorage().ProcessSave(m_Struct, fileName);
	}
	override void OnLoaded(string fileName)
	{
		GetGame().GetBackendApi().GetStorage().ProcessLoad(m_Struct, fileName);
		LoadStruct();
	}
	override void OnConnect()
	{
		Print("OnConnect", LogLevel.DEBUG);
	}
	override void OnDisconnect()
	{
		Print("OnDisconnect", LogLevel.DEBUG);
	}
	override void OnSetup( string fileName )
	{
		Print("OnSetup", LogLevel.DEBUG);
	}
	override void OnLoadFailed( string fileName )
	{
		Print("OnLoadFailed", LogLevel.DEBUG);
	}
	override void OnSaveFailed( string fileName )
	{
		Print("OnSaveFailed", LogLevel.DEBUG);
	}
	override void OnSaveSuccess( string filename )
	{
		Print("OnSaveSuccess", LogLevel.DEBUG);
	}
	override void OnInitialize()
	{
		Print("OnInitialize", LogLevel.DEBUG);
	}
	override void OnNew()
	{
		Print("OnNew", LogLevel.DEBUG);
	}
	override void OnReady()
	{
		Print("OnReady", LogLevel.DEBUG);
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Constructor
	/////////////////////////////////////////////////////////////////////////////
	void SCR_DSSessionCallback(SCR_MissionStruct struct)
	{
		m_Struct = struct;
	}
};