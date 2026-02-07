[BaseContainerProps()]
class SCR_CampaignTutorialArlandStages
{
	[Attribute()]
	private ref array<ref SCR_CampaignTutorialArlandStageInfo> m_TutorialArlandStages;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialArlandStageMasters))]
	protected SCR_ETutorialArlandStageMasters m_ConfigClassName;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, "Config with voice over data for tutorial stages and subtitles.", "conf")]
	protected ResourceName m_sVoiceOverDataConfig;
	
	[Attribute(defvalue :"{D54A817EA5E7184F}Prefabs/Characters/Campaign/Final/Campaign_US_Player_Tutorial.et", UIWidgets.ResourceNamePicker, "Player loadout for specified stage", "et")]
	protected ResourceName m_sPlayerLoadoutName;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EWeatherStates), category: "Environment")]
	protected SCR_EWeatherStates m_WeatherOverride;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, enums: ParamEnumArray.FromEnum(SCR_EWeatherStates), category: "Environment")]
	protected bool m_bLoopWeather;
	
	[Attribute(defvalue: "12", params: "0.0 24.0 ", uiwidget: UIWidgets.Slider, category: "Environment")]
	protected float m_fTime;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ETutorialArlandStageMasters GetConfigClassName()
	{
		return m_ConfigClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] TutorialArlandStages
	void GetStages(out notnull array<ref SCR_CampaignTutorialArlandStageInfo> TutorialArlandStages)
	{
		TutorialArlandStages = m_TutorialArlandStages;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ETutorialArlandStageMasters GetStagesFromConfig()
	{
		return m_ConfigClassName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPlayerLoadout()
	{
		return m_sPlayerLoadoutName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EWeatherStates GetWeatherOverride()
	{
		return m_WeatherOverride;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool ShouldWeatherLoop()
	{
		return m_bLoopWeather;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTime()
	{
		return m_fTime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetVoiceOverDataConfig()
	{
		return m_sVoiceOverDataConfig;
	}
}