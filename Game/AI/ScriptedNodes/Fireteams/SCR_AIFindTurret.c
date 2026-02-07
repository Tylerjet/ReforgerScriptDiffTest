class SCR_AIFindTurret: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_TURRET_OUT				= "TurretOut";	
		
	private BaseWorld m_world;
	private IEntity m_turretEntity;
	
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (!m_world && GetGame())
			m_world = GetGame().GetWorld();
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (m_world)
		{	
			vector center;
			float radius;
			
			m_turretEntity = null;
			GetVariableIn(PORT_CENTER_OF_SEARCH, center);
			GetVariableIn(PORT_RADIUS, radius);
			
			m_world.QueryEntitiesBySphere(center, radius, GetFirstEntity, FilterEntities, EQueryEntitiesFlags.DYNAMIC);
			if (m_turretEntity)
			{
				SetVariableOut(PORT_TURRET_OUT, m_turretEntity);
				return ENodeResult.SUCCESS;
			}			
		};		
		ClearVariable(PORT_TURRET_OUT);
		return ENodeResult.FAIL;
	}
	
	bool GetFirstEntity(IEntity ent) 
	{
		BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(ent.FindComponent(BaseCompartmentManagerComponent));
		if (!compComp)
			return true;
		
		array<BaseCompartmentSlot> compartmentSlots = {};
		compComp.GetCompartments(compartmentSlots);
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			TurretCompartmentSlot turretComp = TurretCompartmentSlot.Cast(slot);
			if (turretComp)
			{
				if (!turretComp.AttachedOccupant() && turretComp.IsCompartmentAccessible())
				{
					turretComp.SetCompartmentAccessible(false);
					//turretComp.GetCompartmentSlotID();
					m_turretEntity = ent;
					return false;
				}
			}	
		}
		
		return true; //continue search
	}
	
	bool FilterEntities(IEntity ent) 
	{
		
		if (ent.FindComponent(BaseCompartmentManagerComponent))
			return true;			
				
		return false;		
	}
	
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TURRET_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_CENTER_OF_SEARCH,
		PORT_RADIUS
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
};