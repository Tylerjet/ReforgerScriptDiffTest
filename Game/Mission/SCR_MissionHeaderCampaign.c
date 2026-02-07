class SCR_MissionHeaderCampaign : SCR_MissionHeader
{
	[Attribute("1", UIWidgets.CheckBox, "Allow dynamic despawn of Remnant forces")]
	bool m_bDespawnRemnants;

	[Attribute("0", UIWidgets.CheckBox, "Start the game with some bases pre-owned")]
	bool m_bAdvancedStage;
	
	[Attribute("-1", UIWidgets.EditBox, "Maximum amount of active respawn radios (override, -1 for default)")]
	int m_iMaximumRespawnRadios;
	
	[Attribute("-1", UIWidgets.EditBox, "Minimum starting amount of supplies in small bases (override, -1 for default)")]
	int m_iMinimumBaseSupplies;
	
	[Attribute("-1", UIWidgets.EditBox, "Maximum starting amount of supplies in small bases (override, -1 for default")]
	int m_iMaximumBaseSupplies;
	
	[Attribute("-1", UIWidgets.EditBox, "The range of radio signal relayed by bases (global override, -1 for default")]
	int m_iCustomRadioRange;
	
	[Attribute("", UIWidgets.EditBox, "Must be valid Main or Major Conflict base name (empty = default)")]
	string m_sCustomHQWest;
	
	[Attribute("", UIWidgets.EditBox, "Must be valid Main or Major Conflict base name (empty = default)")]
	string m_sCustomHQEast;
	
	[Attribute("0", UIWidgets.CheckBox, "Use custom base list whitelist instead of blacklist")]
	bool m_bCustomBaseWhitelist;
	
	[Attribute("0", UIWidgets.CheckBox, "Disable rank requirements for spawning vehicles ")]
	bool m_bIgnoreMinimumVehicleRank;
	
	[Attribute("1", UIWidgets.EditBox, "XP multiplier (1 for default)")]
	float m_fXpMultiplier;
	
	[Attribute()]
	ref array<ref SCR_CampaignCustomBase> m_aCampaignCustomBaseList;
};