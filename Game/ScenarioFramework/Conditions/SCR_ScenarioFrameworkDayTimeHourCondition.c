[BaseContainerProps()]
class SCR_ScenarioFrameworkDayTimeHourCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(defvalue: "0", desc: "Minimal day time hour", params: "0 24 1", category: "Time")]
	int m_iMinHour;

	[Attribute(defvalue: "24", desc: "Maximal day time hour", params: "0 24 1", category: "Time")]
	int m_iMaxHour;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(entity.GetWorld());
		if (!world)
			return true;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return true;

		TimeContainer timeContainer = manager.GetTime();
		int currentHour = timeContainer.m_iHours;

		if (currentHour < m_iMinHour || currentHour > m_iMaxHour)
			return false;

		return true;
	}
}