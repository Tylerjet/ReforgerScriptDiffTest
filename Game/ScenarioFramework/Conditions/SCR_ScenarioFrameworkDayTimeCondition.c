[BaseContainerProps()]
class SCR_ScenarioFrameworkDayTimeCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(defvalue: "1", desc: "If true, this can be activated only during the day. If false, only during the night.", category: "Time")]
	bool m_bOnlyDuringDay;

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

		if (m_bOnlyDuringDay)
			return manager.IsDayHour(currentHour);
		else
			return manager.IsNightHour(currentHour);
	}
}