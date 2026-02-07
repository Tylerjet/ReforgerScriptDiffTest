[BaseContainerProps(configRoot: true)]
class SCR_PlayMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Featured game header", "conf")]
	ResourceName m_FeaturedMission;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Tutorial mission", "conf")]
	ResourceName m_TutorialMission;
	
	[Attribute("", UIWidgets.ResourceAssignArray, "Recommended game headers", "conf")]
	ref array<ResourceName> m_aRecommendedMissions;
	
	[Attribute("", UIWidgets.ResourceAssignArray, "Game master-only game headers", "conf")]
	ref array<ResourceName> m_aGameMasterMissions;

	[Attribute("3", desc: "How many recent entries should be shown")]
	int m_iShowRecentMissions;
};