class SCR_MissionHeader : MissionHeader
{
	[Attribute("", UIWidgets.EditBox, "Name of this mission.")]
	string m_sName;

	[Attribute("", UIWidgets.EditBox, "Author of this mission.")]
	string m_sAuthor;

	[Attribute("", UIWidgets.EditBox, "Path to mission.")]
	string m_sPath;

	[Attribute("", UIWidgets.EditBox, "Brief description of this mission's purpose.")]
	string m_sDescription;

	[Attribute("", UIWidgets.EditBox, "Detailed description of this mission (i.e. rules).")]
	string m_sDetails;

	[Attribute("{6CFADAEE9287D1D2}UI/Textures/WorldSelection/Default.edds", UIWidgets.ResourceNamePicker, "Icon texture of this mission visible e.g. in menus.", "edds")]
	ResourceName m_sIcon;

	[Attribute("{C58FCC06AF13075B}UI/Textures/MissionLoadingScreens/placeholder_1.edds", UIWidgets.ResourceNamePicker, "Texture of this mission visible when loading this mission.", "edds")]
	ResourceName m_sLoadingScreen;

	[Attribute("{C58FCC06AF13075B}UI/Textures/MissionLoadingScreens/placeholder_1.edds", UIWidgets.ResourceNamePicker, "Texture of this mission visible when loading this mission.", "edds")]
	ResourceName m_sPreviewImage;

	[Attribute("Sandbox", UIWidgets.EditBox, "Game mode of this mission.")]
	string m_sGameMode;

	[Attribute("1", UIWidgets.EditBox, "The count of players for this mission")]
	int m_iPlayerCount;

	[Attribute("0", uiwidget: UIWidgets.Flags, "Editable Game Flags", "", ParamEnumArray.FromEnum(EGameFlags))]
	EGameFlags m_eEditableGameFlags;

	[Attribute("0", uiwidget: UIWidgets.Flags, "Default Game Flags", "", ParamEnumArray.FromEnum(EGameFlags))]
	EGameFlags m_eDefaultGameFlags;

	[Attribute(desc: "When true, saving mission state is enabled.")]
	protected bool m_bIsSavingEnabled;

	[Attribute("", UIWidgets.EditBox, "Name of save file for this mission.\nWhen undefined, the name of associated world file will be used.")]
	protected string m_sSaveFileName;

	protected bool m_bLoadOnStart;
	string m_sOwner;

	[Attribute("", UIWidgets.ResourceNamePicker, "Configuration file for briefing screen.", "conf")]
	ResourceName m_sBriefingConfig;

	//------------------------------------------------------------------------------------------------
	//! Returns whether mission can be played in multiplayer or not.
	bool IsMultiplayer()
	{
		return m_iPlayerCount > 1;
	}

	/*!
	\return True if saving is enabled in this mission.
	*/
	bool IsSavingEnabled()
	{
		return m_bIsSavingEnabled;
	}

	/*!
	Get name of athesave file for this mission.
	\return File name
	*/
	string GetSaveFileName()
	{
		if (m_sSaveFileName)
			return m_sSaveFileName;
		else
			return FilePath.StripPath(FilePath.StripExtension(GetWorldPath()));
	}
};
