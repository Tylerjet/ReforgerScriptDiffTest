//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDebugArea : ScriptAndConfig
{
	[Attribute(desc: "Name of the Area which will be selected to spawn", category: "Debug")];
	protected string m_sForcedArea;
	
	[Attribute(desc: "Name of the Layer Task that will get spawned under attached Area. If left empty, Layer Task will be randomly selected", category: "Debug")];
	protected string m_sForcedLayerTask;

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkArea GetForcedArea()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sForcedArea))
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedArea);
		if (!entity)
			return null;
		
		return SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerTask GetForcedLayerTask()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sForcedLayerTask))
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedLayerTask);
		if (!entity)
			return null;
		
		return SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
	}
}