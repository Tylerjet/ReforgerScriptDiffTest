//! Config template for vehicles available for request in Campaign
[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Prefab", true)]
class SCR_CampaignVehicleAssetInfo
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "", "et")]
	protected ResourceName m_Prefab;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignBaseOwner))]
	protected SCR_ECampaignBaseOwner m_eFaction;
	
	[Attribute("Vehicle", desc: "Display name to be shown in UI.")]
	protected string m_sDisplayName;
	
	[Attribute("VEHICLE", desc: "Display name to be shown in UI (upper case).")]
	protected string m_sDisplayNameUC;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Lowest rank that can request this vehicle.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eRankID;
	
	[Attribute("20", desc: "Maximum amount of these assets a base can hold.")]
	protected int m_iMax;
	
	[Attribute("20", desc: "Number of these assets available at the start of the scenario.")]
	protected int m_iStartingAmount;
	
	[Attribute("5", desc: "How many of these assets arrive with reinforcements.")]
	protected int m_iReinforcementsAmount;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Opposite prefab in enemy faction. They can share empty spots in vehicle depots.", "et")]
	protected ResourceName m_PrefabOpposite;
	
	ResourceName GetPrefab()
	{
		return m_Prefab;
	}
	
	FactionKey GetFactionKey()
	{
		if (m_eFaction == SCR_ECampaignBaseOwner.BLUFOR)
			return SCR_GameModeCampaignMP.FACTION_BLUFOR;
		
		return SCR_GameModeCampaignMP.FACTION_OPFOR;
	}
	
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
	
	string GetDisplayNameUpperCase()
	{
		return m_sDisplayNameUC;
	}
	
	SCR_ECharacterRank GetMinimumRankID()
	{
		return m_eRankID;
	}
	
	int GetMaximumAmount()
	{
		return m_iMax;
	}
	
	int GetStartingAmount()
	{
		return m_iStartingAmount;
	}
	
	int GetReinforcementsAmount()
	{
		return m_iReinforcementsAmount;
	}
	
	ResourceName GetPrefabOpposite()
	{
		return m_PrefabOpposite;
	}
};

//! All vehicles available for players to request in Campaign are stored here along with important data
[BaseContainerProps(configRoot: true)]
class SCR_CampaignVehicleAssetList
{
	[Attribute(desc: "Vehicle asset list.")]
	private ref array<ref SCR_CampaignVehicleAssetInfo> m_VehicleAssetList;
	
	
	void GetVehicleAssetList(out notnull array<ref SCR_CampaignVehicleAssetInfo> vehicleAssetList)
	{
		vehicleAssetList = m_VehicleAssetList;
	}
	
	void ~SCR_CampaignVehicleAssetList()
	{
		m_VehicleAssetList = null;
	}
};