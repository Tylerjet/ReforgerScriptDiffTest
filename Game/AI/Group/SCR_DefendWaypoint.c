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
	
	[Attribute("true", UIWidgets.CheckBox, "Use observation posts for surveillance")];
	bool m_bUseObservationPosts;
	
	[Attribute("false", UIWidgets.CheckBox, "Use gate posts for opening gates for vehicles")];
	bool m_bUseGatePosts;
	
	[Attribute("true", UIWidgets.CheckBox, "Use cover posts?")];
	bool m_bUseCoverPosts;	
};

class SCR_DefendWaypoint : SCR_TimedWaypoint
{
	[Attribute("", UIWidgets.Object, "Defend presets")];
	private ref array<ref SCR_DefendWaypointPreset> m_DefendPresets;
	
	private int m_iCurrentDefendPreset;
	 
	//----------------------------------------------------------------------------------------
	array<string> MakeTagsForSearch ()
	{
		array<string> result = new array<string>;
		if (m_DefendPresets[m_iCurrentDefendPreset].m_bUseObservationPosts)
			result.Insert("ObservationPost");
		if (m_DefendPresets[m_iCurrentDefendPreset].m_bUseGatePosts)
			result.Insert("GatePost");
		if (m_DefendPresets[m_iCurrentDefendPreset].m_bUseCoverPosts)
			result.Insert("CoverPost");
		return result;	
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
};