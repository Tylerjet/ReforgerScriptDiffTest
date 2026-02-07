[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		PlayNarrativeCharacterStage("DRIVING_Copilot", 1);
		
		CreateMarkerCustom("DRIVING_Copilot", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_DRIVING_COURSE_IN_INSTRUCTOR_F_01" || GetDuration() > 3000;
	}
};