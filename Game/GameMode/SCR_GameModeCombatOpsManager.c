[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeCombatOpsManagerClass : SCR_GameModeSFManagerClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------

class SCR_GameModeCombatOpsManager : SCR_GameModeSFManager
{	
	[Attribute( defvalue: "3", desc: "Maximal number of tasks that can be generated", category: "Tasks" )];
	protected int 				m_iMaxNumberOfTasks;
	
	//------------------------------------------------------------------------------------------------
	override void OnTaskFinished( SCR_BaseTask pTask )
	{
		super.OnTaskFinished( pTask );
		if ( pTask == m_pExtractionAreaTask )
			Finish();
	}
	
	//------------------------------------------------------------------------------------------------
	// Main function responsible for selecting available tasks and spawning the areas related to the tasks
	override void GenerateTasks()
	{
	
		if ( m_aTaskTypesAvailable.IsEmpty() )
			return;
		
		if (m_iMaxNumberOfTasks > m_aTaskTypesAvailable.Count())
			m_iMaxNumberOfTasks = m_aTaskTypesAvailable.Count();
		
		ESFTaskType eTaskType;
		CP_TaskType pTaskType;
		
		//shuffle tasks to prevent selecting them always in the same order
		Print( "CP: ---------------------------------------------------------------" );
		for ( int i = 0; i < m_iMaxNumberOfTasks; i++ )
		{
			pTaskType = m_aTaskTypesAvailable.GetRandomElement();
			eTaskType = pTaskType.GetTaskType();	
			if ( eTaskType == ESFTaskType.NONE )
				continue;
			CP_Area pArea;
			if ( m_sForcedArea.IsEmpty() )
			{
				pArea = SelectRandomAreaByTaskType( eTaskType );		
			}
			else
			{
				IEntity pEnt = GetGame().GetWorld().FindEntityByName( m_sForcedArea );	//for debug
				if ( pEnt )
				{
					pArea = CP_Area.Cast( pEnt.FindComponent( CP_Area ) );
				}
			}
			
			if ( pArea )
			{
				CP_LayerTask pLayer = pArea.Create( eTaskType );
				PrintFormat( "CP: Creating area %1", pArea.GetOwner().GetName() );
				m_aTaskTypesAvailable.RemoveItem( pTaskType );
				m_iMaxNumberOfTasks--;
				i--;
			}
		}
		Print( "CP: ---------------------------------------------------------------" );
	
	}
		
	//------------------------------------------------------------------------------------------------
	// Mainly for debug purposes
	/*
	protected void GenerateSingleTask()
	{
		IEntity pEnt = GetGame().GetWorld().FindEntityByName( m_sForcedTaskLayer );
		if ( !pEnt )
			return;
		CP_LayerTask pTaskComponent = CP_LayerTask.Cast( pEnt.FindComponent( CP_LayerTask ) );
		if ( !pTaskComponent )
			return;
		IEntity pEntArea = GetGame().GetWorld().FindEntityByName( m_sForcedArea );
		if ( !pEntArea )
		{
			pEntArea = pEnt.GetParent();
		}
		if ( !pEntArea )
			return;		
		CP_Area pArea = CP_Area.Cast( pEntArea.FindComponent( CP_Area ) );
		if ( !pArea )
			return;
		pArea.Create( pTaskComponent );
		PrintFormat( "CP: Creating area %1", pArea.GetOwner().GetName() );
		SCR_BaseTask pTask = CreateTask( pTaskComponent.GetTaskType(), pArea, pTaskComponent );
		if ( !pTask )
			return;
		SetupTask( pArea, pTask )
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	CP_Area SelectRandomAreaByTaskType( ESFTaskType eTaskType )
	{
		if ( m_aAreas.IsEmpty() )
			return null;
		CP_Area pSelectedArea;
		array<CP_Area> aAreasCopy = {};
		aAreasCopy.Copy( m_aAreas );
		for ( int i = 0; i < m_aAreas.Count(); i++ )
		{
			int iSeed = Math.Randomize( -1 );
			pSelectedArea = aAreasCopy.GetRandomElement();
			if ( !pSelectedArea.GetIsAreaSelected() && pSelectedArea.GetIsTaskSuitableForArea( eTaskType ) )
			{
				pSelectedArea.SetAreaSelected( true );
				return pSelectedArea;
			}
			else
			{
				aAreasCopy.RemoveItem( pSelectedArea );
			}
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PostInit()
	{
		//go throught the Areas	again, find appropriate ones and spawn only task layers
		if ( m_sForcedTaskLayer.IsEmpty() )		//for debug purposes
			GenerateTasks();
		else
			GenerateSingleTask();
	}
	
	//------------------------------------------------------------------------------------------------
	//
	protected void GenerateExtractionArea()
	{
		ESFTaskType eTaskType;
		CP_TaskType pTaskType;
		
		eTaskType = ESFTaskType.EXTRACTION;	
		//CP_Area pArea = SelectRandomAreaByTaskType( eTaskType );		
		CP_Area pArea = SelectNearestAreaByTaskType( eTaskType );		
		if ( pArea )
		{
			/*
			m_pExtractionAreaTask = CreateTask( eTaskType, pArea, pArea.Create( eTaskType ) );
			if ( m_pExtractionAreaTask )
			{
				pArea.StoreTaskToArea( m_pExtractionAreaTask );
				//pArea.MoveTaskIconToArea( m_pExtractionAreaTask );	
			}
			*/
		}
	}	
			
}
