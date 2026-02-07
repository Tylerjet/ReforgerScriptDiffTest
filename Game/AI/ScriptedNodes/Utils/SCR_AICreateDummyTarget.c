class SCR_AICreateDummyTarget: AITaskScripted
{
	static const string PORT_SHOOTER_ENTITY = "ShooterIn";
	static const string PORT_TARGET_ENTITY 	= "TargetOut";
	static const string PORT_POSITION_RESET	= "NewCycleOut";
	
	ref private array<int> m_aDistances = {5,10,25,50,100,150,200,250};
	
	private int m_iDistanceIndex = 0;
	ref private Resource m_resource;	
	
	override void OnInit(AIAgent owner)
	{
		m_resource = Resource.Load("{FDEA1BDCB9C49297}Prefabs/Test/TestPenetration/TargetPlate_100mm.et");		
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity shooter, target;
		vector position;
		if (GetVariableIn(PORT_SHOOTER_ENTITY,shooter))
		{
			position = shooter.GetOrigin();
			position[0] = position[0] + m_aDistances[m_iDistanceIndex];
			position[1] = position[1] + 1.5;
			m_iDistanceIndex = (m_iDistanceIndex + 1) % m_aDistances.Count();
			
			bool result = m_iDistanceIndex == 0;
			SetVariableOut(PORT_POSITION_RESET,result);
			
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			Math3D.AnglesToMatrix("90 0 0",params.Transform);
			params.Transform[3] = position;
			
			target = GetGame().SpawnEntityPrefab(m_resource, null, params);
			SetVariableOut(PORT_TARGET_ENTITY,target);
			return ENodeResult.SUCCESS;	
		}		
		return ENodeResult.FAIL;
	} 
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_TARGET_ENTITY,
		PORT_POSITION_RESET
	};
	override TStringArray GetVariablesOut()
	{
        return s_aVarsOut;
    }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_SHOOTER_ENTITY
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override bool VisibleInPalette()
	{
		return true;
	}	
	
	override string GetOnHoverDescription() 
	{ 
		return "CreateDummyTarget: spawns target entity at location in front of shooter entity and returns reference to it";	
	};
};