/*!
Callback for easy handling of world saving and loading.
Controlled from SCR_SaveLoadComponent.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESaveType, "m_eType")]
class SCR_DSSessionCallback: DSSessionCallback
{
	[Attribute(ESaveType.USER.ToString(), UIWidgets.ComboBox, "Save file type.", enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eType;
	
	[Attribute(desc: "Unique extension added in front of .json extension.\nUsed for identifying save types without opening files.")]
	protected string m_sExtension;
	
	[Attribute("-", desc: "Character added between mission name and custom name.\nWhen empty, custom name will not be used.")]
	protected string m_sCustomNameDelimiter;
	
	[Attribute(desc: "When enabled, save file will never be saved in cloud.")]
	protected bool m_bAlwaysLocal;
	
	[Attribute(desc: "When enabled, save of this type will also become the latest save for given mission.")]
	protected bool m_bRegisterLatestSave;
	
	protected SCR_MissionStruct m_Struct;
	protected bool m_bLoadPreview;
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Public
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	\return Save type of this callback
	*/
	ESaveType GetSaveType()
	{
		return m_eType;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Save current session to a file.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if saving operation was requested
	*/
	bool SaveSession(string fileName, string customName = string.Empty)
	{
		fileName = GetFileName(fileName, customName);
		
		if (!IsCompatible(fileName))
			return false;
		
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		
		if (!m_bAlwaysLocal && GetGame().GetSaveManager().CanSaveToCloud())
		{
			storage.RequestSave(fileName);
			PrintFormat("SCR_DSSessionCallback: RequestSave: %1", fileName);
		}
		else
		{
			storage.LocalSave(fileName);
			PrintFormat("SCR_DSSessionCallback: LocalSave: %1", fileName);
		}
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Load session from given save file.
	\param fileName Full save file name
	\return True if loading operation was requested
	*/
	bool LoadSession(string fileName)
	{
		if (!IsCompatible(fileName))
			return false;
		
		m_bLoadPreview = false;
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		
		if (!m_bAlwaysLocal && GetGame().GetSaveManager().CanSaveToCloud())
		{
			storage.RequestLoad(fileName);
			PrintFormat("SCR_DSSessionCallback: RequestLoad: %1", fileName);
		}
		else
		{
			storage.LocalLoad(fileName);
			PrintFormat("SCR_DSSessionCallback: LocalLoad: %1", fileName);
		}
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if file of given type exists.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if the file exists
	*/
	bool FileExists(string fileName, string customName = string.Empty)
	{
		return GetGame().GetBackendApi().GetStorage().CheckFileID(GetFileName(fileName, customName));
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Open file name and read its meta header.
	\param Save file name
	\return Meta header
	*/
	SCR_MetaStruct GetMeta(string fileName)
	{
		if (!IsCompatible(fileName))
			return null;
		
		m_bLoadPreview = true;
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		storage.LocalLoad(fileName);

		return m_Struct.GetMeta();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set JSON struct that defines what will be saved.
	\param struct Save struct
	*/
	void SetStruct(SCR_MissionStruct struct)
	{
		m_Struct = struct;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return JSON struct that defines what will be saved.
	*/
	SCR_MissionStruct GetStruct()
	{
		return m_Struct;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Print out JSON struct that is currently kept in the memory.
	*/
	void Log()
	{
		m_Struct.Log();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return True if this callback is configured correctly
	*/
	bool IsConfigured()
	{
		return m_Struct != null;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if a save file name is compatible with this callback, and can be load using its settings.
	\param fileName Save file name
	\return True if compatible
	*/
	bool IsCompatible(string fileName)
	{
		return m_Struct && fileName.Contains(m_sExtension);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract mission file name from save file name.
	\param fileName Save file name
	\return Mission file name
	*/
	string GetMissionFileName(string fileName)
	{
		if (!m_sCustomNameDelimiter)
		{
			return FilePath.StripExtension(fileName);
		}
		
		int delimiterIndex = fileName.IndexOf(m_sCustomNameDelimiter);
		if (delimiterIndex >= 0)
			return fileName.Substring(0, delimiterIndex);
		else
			return string.Empty;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract custom name from save file name.
	\param fileName Save file name
	\return Custom name
	*/
	string GetCustomName(string fileName)
	{
		if (!m_sCustomNameDelimiter)
			return string.Empty;
		
		int delimiterIndex = fileName.IndexOf(m_sCustomNameDelimiter);
		if (delimiterIndex < 0)
			return string.Empty;
		
		delimiterIndex += m_sCustomNameDelimiter.Length();
		int length = fileName.Length() - delimiterIndex;
		fileName = fileName.Substring(delimiterIndex, length);
		
		string ext;
		return FilePath.StripExtension(fileName, ext);
	}
	
	//----------------------------------------------------------------------------------------
	protected string GetFileName(string fileName, string customName)
	{
		if (m_sCustomNameDelimiter)
		{
			customName = SCR_StringHelper.Filter(customName, SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + "_");
			fileName += m_sCustomNameDelimiter + customName;
		}
		
		return fileName + m_sExtension;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Override
	//////////////////////////////////////////////////////////////////////////////////////////
	override void OnSaving(string fileName)
	{
		if (m_Struct.Serialize())
			GetGame().GetBackendApi().GetStorage().ProcessSave(m_Struct, fileName);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnSaveSuccess(string filename)
	{
		//--- Set the save file as the latest save (with a delay; this callback is still used by storage)
		if (m_bRegisterLatestSave)
			GetGame().GetCallqueue().CallLater(GetGame().GetSaveManager().SetCurrentMissionLatestSave, 0, false, filename);
		
		Print(string.Format("SCR_DSSessionCallback: Saving save file of type %1 in '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), filename), LogLevel.VERBOSE);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnSaveFailed(string fileName)
	{
		Print(string.Format("SCR_DSSessionCallback: Saving save file of type %1 in '%2' failed!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.WARNING);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoaded(string fileName)
	{
		GetGame().GetBackendApi().GetStorage().ProcessLoad(m_Struct, fileName);
		
		if (m_bLoadPreview)
		{
			Print(string.Format("SCR_DSSessionCallback: Previewing save file of type %1 from '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.VERBOSE);
		}
		else
		{
			m_Struct.Deserialize();
			
			Print(string.Format("SCR_DSSessionCallback: Loading save file of type %1 from '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.VERBOSE);
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoadFailed(string fileName)
	{
		Print(string.Format("SCR_DSSessionCallback: Loading save file of type %1 from '%2' failed!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.WARNING);
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_DSSessionCallback()
	{
		m_sExtension = FilePath.AppendExtension(string.Empty, m_sExtension);
	}
	
	//----------------------------------------------------------------------------------------
	void ~SCR_DSSessionCallback()
	{
		m_Struct = null;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESaveType, "m_eType")]
class SCR_NumberedDSSessionCallback: SCR_DSSessionCallback
{
	[Attribute("1", UIWidgets.Slider, "", params: "1 10 1")]
	protected int m_iMaxSaves;
	
	//----------------------------------------------------------------------------------------
	override protected string GetFileName(string fileName, string customName)
	{
		if (m_iMaxSaves > 1)
		{
			int saveId;
			
			string latestSaveName;
			if (GetGame().GetSaveManager().FindCurrentMissionLatestSave(latestSaveName))
				saveId = GetCustomName(latestSaveName).ToInt();
			
			saveId = (saveId % m_iMaxSaves) + 1;
			customName = saveId.ToString();
		}
		else
		{
			customName = "1";
		}
		return super.GetFileName(fileName, customName);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESaveType, "m_eType")]
class SCR_DisposableDSSessionCallback: SCR_DSSessionCallback
{
	override void OnLoaded(string fileName)
	{
		super.OnLoaded(fileName);
		GetGame().GetBackendApi().GetStorage().LocalDelete(fileName);
	}
};