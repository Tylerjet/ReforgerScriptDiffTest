[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_RPG_DISPOSEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_RPG_DISPOSE : SCR_BaseTutorialStage
{
	IEntity m_CourseRPG;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_CourseRPG = GetGame().GetWorld().FindEntityByName("COURSE_RPG");
		if (!m_CourseRPG)
		{
			m_bFinished = true;
			return;
		}
		
		m_TutorialComponent.EnableRefunding(m_CourseRPG, true);

		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 11);

		RegisterWaypoint("SW_ARSENAL_USSR", "", "CUSTOM");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return !m_CourseRPG;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_RPG_DISPOSE()
	{
		if (m_TutorialComponent)
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_USSR", false);
	}
}