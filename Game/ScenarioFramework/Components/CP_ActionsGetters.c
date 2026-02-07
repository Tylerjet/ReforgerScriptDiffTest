
//------------------------------------------------------------------------------------------------
class CP_ParamBase {}

class CP_Param<Class T> : CP_ParamBase
{
	T m_pValue;
	
	T GetValue()
	{
		return m_pValue;	
	}
	
	void CP_Param( T value )
	{
		m_pValue = value;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_Get
{
	private CP_ParamBase m_pValue;
	protected void SetValue( CP_ParamBase pValue ) 
	{
		m_pValue = pValue;
	}
	CP_ParamBase Get() { return m_pValue; }
}


//------------------------------------------------------------------------------------------------
//[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetLastFinishedTaskLayer : CP_Get
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		SCR_GameModeSFManager pManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pManager )
			return null;
		CP_Param<CP_LayerTask> param = new CP_Param<CP_LayerTask>( CP_LayerTask.Cast( pManager.GetLastFinishedTaskLayer() ) );
		return param;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetLastFinishedTaskEnity : CP_Get
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		SCR_GameModeSFManager pManager = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pManager )
			return null;
		CP_Param<IEntity> param = new CP_Param<IEntity>( pManager.GetLastFinishedTask() );
		return param;
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
		if ( !GetGame().GetWorld() )
			return null;
		CP_Param<IEntity> param = new CP_Param<IEntity>( GetGame().GetWorld().FindEntityByName( m_sEntityName ) );
		return param;
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
		if ( !GetGame().GetWorld() )
			return null;
		IEntity pEnt = GetGame().GetWorld().FindEntityByName( m_sLayerTaskName );
		if ( !pEnt )
			return null;
		CP_LayerTask pLayer = CP_LayerTask.Cast( pEnt.FindComponent( CP_LayerTask ) );
		if ( !pLayer )
			return null;
		CP_Param<IEntity> param = new CP_Param<IEntity>( pLayer.GetTask() );
		return param;
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
		if ( !GetGame().GetWorld() )
			return null;	
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger( aEntities );

		return new CP_Param<array<IEntity>>( aEntities );
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetEntitiesInTrigger( out notnull array<IEntity> aEntities )
	{
		BaseGameTriggerEntity pTrig = BaseGameTriggerEntity.Cast( GetGame().GetWorld().FindEntityByName( m_sTriggerName ) );
		if ( !pTrig )
			return;
		pTrig.GetEntitiesInside( aEntities );	
	}
}


//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetCountEntitiesInTrigger : CP_GetListEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if ( !GetGame().GetWorld() )
			return null;
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger( aEntities );
		
		
		return new CP_Param<int>( aEntities.Count() );
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_GetPlayerEntity: CP_Get
{
	//------------------------------------------------------------------------------------------------
	override CP_ParamBase Get()
	{
		if ( !GetGame().GetWorld() )
			return null;
			SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!pc)
			return null;
				
		CP_Param<IEntity> param = new CP_Param<IEntity>( pc.GetMainEntity() );
		return param;
	}
}