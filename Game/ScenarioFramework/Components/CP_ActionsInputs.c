//------------------------------------------------------------------------------------------------
//TODO: make this a generic action which can be used anywhere anytime ( i.e. on task finished, etc )
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionInputBase
{
	protected CP_LogicInput		m_pInput;
	//------------------------------------------------------------------------------------------------
	void Init( CP_LogicInput pInput )
	{
		m_pInput = pInput;
	}
}	

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionInputOnTaskEventIncreaseCounter : CP_ActionInputBase
{
	[Attribute( desc: "Insert name of the task layer or leave empty for any task" )];
	protected string			m_sTaskLayerName;
	
	[Attribute( "1", UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum( SCR_TaskState ) ) ];
	protected SCR_TaskState			m_eEventName;
	
	protected int 					m_iActionsInput;
	
	override void Init( CP_LogicInput pInput )
	{	
		super.Init( pInput );
		SCR_GameModeSFManager pGameModeComp = SCR_GameModeSFManager.Cast( GetGame().GetGameMode().FindComponent( SCR_GameModeSFManager ) );
		if ( !pGameModeComp ) 
			return;
		pGameModeComp.GetOnTaskStateChanged().Insert( OnActivate );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate( SCR_BaseTask pTask, SCR_ETaskEventMask mask)
	{
		if ((pTask.GetTaskState() == m_eEventName))
		{
			if ( !m_pInput )
				return;
			
			CP_LayerTask pTaskLayer
			string sTaskLayerName = "";
			if ( CP_Task.Cast( pTask ) )
			{
				pTaskLayer = CP_Task.Cast( pTask ).GetTaskLayer();
				if ( pTaskLayer )
					sTaskLayerName = pTaskLayer.GetOwner().GetName();
			} 
			if ( m_sTaskLayerName.IsEmpty() ||  m_sTaskLayerName == sTaskLayerName )
			{
				m_pInput.OnActivate( 1 );
				
				if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED))
				{	
					CP_SlotTask pSubject = pTaskLayer.GetTaskSubject();
					if (pSubject)
						pSubject.OnTaskStateChanged(m_eEventName)
				}	
			}
		}
	}	
}

enum CP_EOperatorCompare
{
	LESS_THEN,
	LESS_OR_EQUAL,
	BIGGER_THEN,
	BIGGER_OR_EQUAL,
	EQUAL
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class CP_ActionInputCheckEntitiesInTrigger : CP_ActionInputBase
{
	[Attribute( desc: "Trigger" )];
	protected ref CP_Get					m_pGetter;
	
	[Attribute( "0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum( CP_EOperatorCompare ) ) ]
	protected CP_EOperatorCompare			m_EOperatorCompare;
	
	[Attribute( desc: "Value" )];
	protected int							m_iValue;
	
	protected SCR_CharacterTriggerEntity	m_pTrig;

		
	//------------------------------------------------------------------------------------------------
	override void Init( CP_LogicInput pInput )
	{	
		super.Init( pInput );
		if ( !m_pGetter )
			return;
		CP_Param<IEntity> pEntWrapper =  CP_Param<IEntity>.Cast( m_pGetter.Get() );
		if ( !pEntWrapper )
		{
			PrintFormat( "CP: the selected getter %1 is not suitable for this operation", m_pGetter.ClassName() );
			return;
		}
		//SCR_BaseTriggerEntity pTrig = SCR_BaseTriggerEntity.Cast( pEntWrapper.GetValue() );
		
		IEntity pEnt = pEntWrapper.GetValue();
		CP_LayerBase pLayer = CP_LayerBase.Cast( pEnt.FindComponent( CP_LayerBase ) );
		if ( !pLayer )
			return;
		SCR_CharacterTriggerEntity pTrig = SCR_CharacterTriggerEntity.Cast( pLayer.GetSpawnedEntity() );
		if ( !pTrig )
			return;		//TODO: add a universal method for informing user about errors
		m_pTrig = pTrig;
		pTrig.GetOnChange().Insert( OnActivate );
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate( CP_Param<IEntity> param )
	{
		if ( !m_pTrig )
			return;
		
		array<IEntity> aEntities = {};
		int iNrOfEnts = m_pTrig.GetCountEntitiesInside();
		
		if ( 
				( ( m_EOperatorCompare == CP_EOperatorCompare.LESS_THEN ) 			&& ( iNrOfEnts < m_iValue ) ) 	||
				( ( m_EOperatorCompare == CP_EOperatorCompare.LESS_OR_EQUAL ) 		&& ( iNrOfEnts <= m_iValue ) ) 	||
				( ( m_EOperatorCompare == CP_EOperatorCompare.EQUAL ) 				&& ( iNrOfEnts == m_iValue ) ) 	||
				( ( m_EOperatorCompare == CP_EOperatorCompare.BIGGER_OR_EQUAL ) 	&& ( iNrOfEnts >= m_iValue ) ) 	||
				( ( m_EOperatorCompare == CP_EOperatorCompare.BIGGER_THEN ) 		&& ( iNrOfEnts > m_iValue ) ) 
			)
		{
			m_pInput.OnActivate( true );
		}
	}
	
}
