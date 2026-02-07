class SCR_DefendWaypointClass: SCR_TimedWaypointClass
{
};

[BaseContainerProps()]
class SCR_DefendWaypointPreset
{
	[Attribute("", UIWidgets.EditBox, "Preset name, only informative. Switch using index.")];
	private string m_sName;
	
	[Attribute("true", UIWidgets.CheckBox, "Use turrets?")];
	bool m_bUseTurrets;
	
	[Attribute("", UIWidgets.Auto, "List tags to search in the preset")];
	ref array<string> m_aTagsForSearch;	
};

class SCR_DefendWaypoint : SCR_TimedWaypoint
{
	[Attribute("0", UIWidgets.Object, "Fast init - units will be spawned on their defensive locations")];
	private bool m_bFastInit;
	
	[Attribute("", UIWidgets.Object, "Defend presets")];
	private ref array<ref SCR_DefendWaypointPreset> m_DefendPresets;
	
	private int m_iCurrentDefendPreset;
	 
	//----------------------------------------------------------------------------------------
	array<string> GetTagsForSearch()
	{
		return m_DefendPresets[m_iCurrentDefendPreset].m_aTagsForSearch;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetUseTurrets()
	{
		return m_DefendPresets[m_iCurrentDefendPreset].m_bUseTurrets;
	}
	
	//----------------------------------------------------------------------------------------
	int GetCurrentDefendPreset()
	{
		return m_iCurrentDefendPreset;
	}
	
	//----------------------------------------------------------------------------------------
	bool SetCurrentDefendPreset(int newDefendPresetIndex)
	{
		if ((newDefendPresetIndex >= 0) && (newDefendPresetIndex < m_DefendPresets.Count()))
		{ 
			m_iCurrentDefendPreset = newDefendPresetIndex;
			return true;
		}
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	bool GetFastInit()
	{
		return m_bFastInit;
	}
	
	//----------------------------------------------------------------------------------------
	void SetFastInit(bool isFastInit)
	{
		m_bFastInit = isFastInit;
	}
};