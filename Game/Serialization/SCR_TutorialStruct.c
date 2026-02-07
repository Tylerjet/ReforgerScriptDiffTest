//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_TutorialStruct : SCR_JsonApiStruct
{
	protected SCR_ETutorialCourses m_eCourses;
	protected int m_iFreeRoamActivations;

	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		
		if (!tutorial)
			return false;
		
		m_eCourses = tutorial.GetFinishedCourses();
		m_iFreeRoamActivations = tutorial.GetFreeRoamActivations();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		
		if (!tutorial)
			return false;
		
		tutorial.LoadProgress(m_eCourses);
		tutorial.SetFreeRoamActivation(m_iFreeRoamActivations);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_TutorialStruct()
	{
		RegV("m_eCourses");
		RegV("m_iFreeRoamActivations");
	}
};