class SCR_AIFindTurret: AITaskScripted
{
	static const string PORT_CENTER_OF_SEARCH		= "OriginIn";
	static const string PORT_RADIUS					= "RadiusIn";
	static const string PORT_TURRET_OUT				= "TurretOut";
	static const string PORT_COMPARTMENT_OUT		= "CompartmentOut";
	
	private BaseWorld m_world;
	private IEntity m_turretEntity;
	private TurretCompartmentSlot m_turretCompartment;
	private SCR_AIGroup m_groupOwner;
	
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (!m_world && GetGame())
			m_world = GetGame().GetWorld();
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
				NodeError(this, owner, "Node is not run on SCR_AIGroup agent or owner is not member of SCR_AIGroup!");
		}
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
				SetVariableOut(PORT_COMPARTMENT_OUT, m_turretCompartment);
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
					m_groupOwner.AllocateCompartment(turretComp);
					//turretComp.GetCompartmentSlotID();
					m_turretEntity = turretComp.GetVehicle();
					m_turretCompartment = turretComp;
					return false;
				}
			}
		}
		return true; //continue search
	}
	
	bool FilterEntities(IEntity ent) 
	{
		return Turret.Cast(ent) != null;
	}
	
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TURRET_OUT,
		PORT_COMPARTMENT_OUT
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