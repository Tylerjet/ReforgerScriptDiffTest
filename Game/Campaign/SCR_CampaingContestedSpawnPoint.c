[EntityEditorProps(category: "GameScripted/GameMode")]
class SCR_CampaignContestedSpawnPointClass : SCR_SpawnPointClass
{
};

class SCR_CampaignContestedSpawnPoint : SCR_SpawnPoint
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox)]
	protected bool m_bIsOppositeGroup;
	
	protected string m_sDisplayName;
	
	//------------------------------------------------------------------------------------------------
	bool GetIsOppositeGroup()
	{
		return m_bIsOppositeGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisplayName(string name)
	{
		m_sDisplayName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
};