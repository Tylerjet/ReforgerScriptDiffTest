
//------------------------------------------------------------------------------------------------
class CP_ParamBase {}

class CP_Param<Class T> : CP_ParamBase
{
	T m_pValue;
	
	T GetValue()
	{
		return m_pValue;	
	}
	
	void CP_Param(T value)
	{
		m_pValue = value;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_Get
{
	private CP_ParamBase m_pValue;
	protected void SetValue(CP_ParamBase pValue) 
	{
		m_pValue = pValue;
	}
	
	CP_ParamBase Get()
	{ 
		return m_pValue; 
	}
}


//------------------------------------------------------------------------------------------------
//[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetLastFinishedTaskLayer : CP_Get
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;
		
		return new CP_Param<CP_LayerTask>(CP_LayerTask.Cast(gameModeManager.GetLastFinishedTaskLayer()));
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetLastFinishedTaskEnity : CP_Get
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;
		
		return new CP_Param<IEntity>(gameModeManager.GetLastFinishedTask());
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetEntityByName: CP_Get
{
	[Attribute()];
	protected string 		m_sEntityName;
	
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		return new CP_Param<IEntity>(GetGame().GetWorld().FindEntityByName(m_sEntityName));
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetTask: CP_Get
{
	[Attribute()];
	protected string 		m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sLayerTaskName);
		if (!entity)
			return null;
		
		CP_LayerTask layer = CP_LayerTask.Cast(entity.FindComponent(CP_LayerTask));
		if (!layer)
			return null;
		
		return new CP_Param<IEntity>(layer.GetTask());
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetLayerTask: CP_Get
{
	[Attribute()];
	protected string 		m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sLayerTaskName);
		if (!entity)
			return null;
		
		CP_LayerTask layer = CP_LayerTask.Cast(entity.FindComponent(CP_LayerTask));
		if (!layer)
			return null;

		return new CP_Param<IEntity>(entity);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetLayerBase: CP_Get
{
	[Attribute()];
	protected string 		m_sLayerBaseName;
	
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sLayerBaseName);
		if (!entity)
			return null;
		
		CP_LayerBase layer = CP_LayerBase.Cast(entity.FindComponent(CP_LayerBase));
		if (!layer)
			return null;

		return new CP_Param<IEntity>(entity);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetArea: CP_Get
{
	[Attribute()];
	protected string 		m_sAreaName;
	
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sAreaName);
		if (!entity)
			return null;
		
		CP_Area area = CP_Area.Cast(entity.FindComponent(CP_Area));
		if (!area)
			return null;

		return new CP_Param<IEntity>(entity);
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetListEntitiesInTrigger : CP_Get
{
	[Attribute()];
	protected string 		m_sTriggerName;
	
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
			
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);

		return new CP_Param<array<IEntity>>(aEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetEntitiesInTrigger(out notnull array<IEntity> aEntities)
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sTriggerName));
		if (!trigger)
			return;
		
		trigger.GetEntitiesInside(aEntities);	
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetCountEntitiesInTrigger : CP_GetListEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);
		
		
		return new CP_Param<int>(aEntities.Count());
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetPlayerEntity: CP_Get
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!playerController)
			return null;
				
		return new CP_Param<IEntity>(playerController.GetMainEntity());
	}
}