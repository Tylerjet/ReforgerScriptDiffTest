//! Config template for XP rewards in Campaign
[BaseContainerProps()]
class SCR_CampaignXPRewardInfo
{	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "ID of this reward.", enums: ParamEnumArray.FromEnum(CampaignXPRewards))]
	protected CampaignXPRewards m_eRewardID;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Corresponding skill.", enums: ParamEnumArray.FromEnum(EProfileSkillID))]
	protected EProfileSkillID m_eSkillID;
	
	[Attribute("Reward name", desc: "Name of this reward.")]
	protected string m_sRewardName;

	[Attribute("10", desc: "Amount of XP awarded.")]
	protected int m_iRewardXP;
	
	CampaignXPRewards GetRewardID()
	{
		return m_eRewardID;
	}
	
	string GetRewardName()
	{
		return m_sRewardName;
	}
	
	int GetRewardXP()
	{
		return m_iRewardXP;
	}
	
	EProfileSkillID GetRewardSkill()
	{
		return m_eSkillID;
	}
};

//! All Campaign ranks are stored here along with important data
[BaseContainerProps(configRoot: true)]
class SCR_CampaignXPRewardList
{
	[Attribute(desc: "Reward list.")]
	private ref array<ref SCR_CampaignXPRewardInfo> m_RewardList;
	
	void GetRewardList(out notnull array<ref SCR_CampaignXPRewardInfo> rewardList)
	{
		rewardList = m_RewardList;
	}
	
	void ~SCR_CampaignXPRewardList()
	{
		m_RewardList = null;
	}
};

enum CampaignXPRewards
{
	UNDEFINED,
	ENEMY_KILL,
	ENEMY_KILL_VEH,
	FRIENDLY_KILL,
	RELAY_DISCOVERED,
	RELAY_RECONFIGURED,
	BASE_SEIZED,
	SUPPLIES_DELIVERED,
	SUPPORT_EVAC,
	CHEAT,
	SUPPORT_FUEL,
	TASK_DEFEND,
	TASK_TRANSPORT,
	SERVICE_BUILD
};