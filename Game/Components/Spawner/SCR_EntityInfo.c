/*!
Info for entities
*/
[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleResourceName("m_sPrefab", true)]
class SCR_EntityInfo
{
	[Attribute("1", desc: "By default disabled entries are ignored.", category: "Entity Info")]
	protected bool m_bEnabled;
	
	[Attribute(desc: "The prefab spawned", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Entity Info")]
	protected ResourceName m_sPrefab;
	
	[Attribute("", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SCR_EEntityType))]
	protected SCR_EEntityType m_eEntityType;
	
	[Attribute("-1", desc: "Cost of entity to spawn. Set -1 to ignore cost.", category: "Entity Info", params: "-1 inf 0.0000000001")]
	protected float m_fCost;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Lowest rank that can request this vehicle.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eRankID;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Auto, desc: "Custom name for entity to be shown in selection.")]
	protected string m_sEntityName;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.Auto, desc: "Custom name for entity to be shown in selection, Upper Case.")]
	protected string m_sEntityNameUC;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set entity Cost
	\param cost Cost of entity. -1 is ignore
	*/
	SCR_EEntityType GetEntityType()
	{
		return m_eEntityType;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set entity Cost
	\param cost Cost of entity. -1 is ignore
	*/
	void SetCost(float cost)
	{
		m_fCost = cost;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get entity Cost
	\return float Cost of entity. -1 is ignore
	*/
	float GetCost()
	{
		return m_fCost;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get if entry is enabled. This is an easy way to disable certain entiries
	\return bool returns true if enabled
	*/
	bool GetEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the prefab of the entity
	\return ResourceName Entity Prefab
	*/
	ResourceName GetPrefab()
	{
		return m_sPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Gets the display name of the entity.
	For now only works with editable entities
	\return string DisplayName
	*/
	string GetDisplayName()
	{
		if (!m_sEntityName.IsEmpty())
			return m_sEntityName;
		
		if (m_sPrefab.IsEmpty())
			return "INVALID";
		
		m_sEntityName = GetEntityNameFromEditableEntity(m_sPrefab);
		if (m_sEntityName != "INVALID")
			return m_sEntityName;
		
		//~ Entity not editable entity so get path name for now
		int pathIndex = m_sPrefab.LastIndexOf("/");
		m_sEntityName =  m_sPrefab.Substring(pathIndex + 1, m_sPrefab.Length() - pathIndex - 3);
		
		string errorOutput = string.Format("Could not proper find spawn entity name for %1", m_sEntityName);
		Print(errorOutput, LogLevel.ERROR);
		
		return m_sEntityName; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Display name in upper case
	string GetDisplayNameUC()
	{
		if (!m_sEntityNameUC.IsEmpty())
			return m_sEntityNameUC;
		
		string uppercase = GetDisplayName();
		if (uppercase != "INVALID")
			uppercase = uppercase + "-UC";
		
		return uppercase;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get Name from editable entity
	string GetEntityNameFromEditableEntity(ResourceName resourceName)
	{
		Resource entityResource = BaseContainerTools.LoadContainer(resourceName);
		if (!entityResource && !entityResource.IsValid())
			return "INVALID";
		
		IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entityResource);
		if (!editableEntitySource)
			return "INVALID";
		
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
		if (!info)
			return "INVALID";
		
		return info.GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetMinimumRankID()
	{
		return m_eRankID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EntityInfo()
	{}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EntityInfo(ResourceName prefab, bool enabled = true, string displayName = string.Empty, float cost = -1)
	{
		m_sPrefab = prefab;
		m_sEntityName = displayName;
		m_bEnabled = enabled;
		SetCost(cost);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_EEntityType
{
	MISC = 0,
	LIGHT = 2,
	HEAVY = 4
}