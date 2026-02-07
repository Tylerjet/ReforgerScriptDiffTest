[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sStageClassName")]
class SCR_CampaignTutorialArlandStageInfo
{
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sStageClassName;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignTutorialArlandStage))]
	protected SCR_ECampaignTutorialArlandStage m_eStage;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ECampaignTutorialArlandStage GetIndex()
	{
		return m_eStage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetClassName()
	{
		return m_sStageClassName;
	}
}