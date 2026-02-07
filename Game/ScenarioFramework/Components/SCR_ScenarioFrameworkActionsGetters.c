//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkParamBase
{
};

class SCR_ScenarioFrameworkParam<Class T> : SCR_ScenarioFrameworkParamBase
{
	T m_Value;
	
	//------------------------------------------------------------------------------------------------
	T GetValue()
	{
		return m_Value;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkParam(T value)
	{
		m_Value = value;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGet
{
	protected SCR_ScenarioFrameworkParamBase m_Value;
	
	//------------------------------------------------------------------------------------------------
	protected void SetValue(SCR_ScenarioFrameworkParamBase value) 
	{
		m_Value = value;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkParamBase Get()
	{ 
		return m_Value; 
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity FindEntityByName(IEntity entity, string name)
	{
		if (!GetGame().GetWorld())
			return null;
		
		entity = GetGame().GetWorld().FindEntityByName(name);
		if (!entity)
			return null;
	
		return entity;
	}
};


//------------------------------------------------------------------------------------------------
//[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskLayer : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;
		
		return new SCR_ScenarioFrameworkParam<SCR_ScenarioFrameworkLayerTask>(SCR_ScenarioFrameworkLayerTask.Cast(gameModeManager.GetLastFinishedTaskLayer()));
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskEnity : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(gameModeManager.GetLastFinishedTask());
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetEntityByName: SCR_ScenarioFrameworkGet
{
	[Attribute()];
	protected string 		m_sEntityName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(GetGame().GetWorld().FindEntityByName(m_sEntityName));
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetTask: SCR_ScenarioFrameworkGet
{
	[Attribute()];
	protected string 		m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(entity, m_sLayerTaskName);
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layer)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(layer.GetTask());
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLayerTask: SCR_ScenarioFrameworkGet
{
	[Attribute()];
	protected string 		m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(entity, m_sLayerTaskName);
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLayerBase: SCR_ScenarioFrameworkGet
{
	[Attribute()];
	protected string 		m_sLayerBaseName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity;
		FindEntityByName(entity, m_sLayerBaseName);
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArea: SCR_ScenarioFrameworkGet
{
	[Attribute()];
	protected string 		m_sAreaName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(entity, m_sAreaName);
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetListEntitiesInTrigger : SCR_ScenarioFrameworkGet
{
	[Attribute()];
	protected string 		m_sTriggerName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
			
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);

		return new SCR_ScenarioFrameworkParam<array<IEntity>>(aEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetEntitiesInTrigger(out notnull array<IEntity> aEntities)
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sTriggerName));
		if (!trigger)
			return;
		
		trigger.GetEntitiesInside(aEntities);	
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetCountEntitiesInTrigger : SCR_ScenarioFrameworkGetListEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);
		
		
		return new SCR_ScenarioFrameworkParam<int>(aEntities.Count());
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetPlayerEntity: SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!playerController)
			return null;
				
		return new SCR_ScenarioFrameworkParam<IEntity>(playerController.GetMainEntity());
	}
};