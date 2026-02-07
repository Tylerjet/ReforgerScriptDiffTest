class SCR_AIReleaseUsedVehicle : AITaskScripted
{
	private CompartmentAccessComponent m_rCompartmentAccess;
	private AIGroup m_rGroup;
	private AIAgent m_rAgent;
	private IEntity m_vehicleEnt;
	private ref array<BaseCompartmentSlot> m_aOutCompartments;
	
	[Attribute("0", UIWidgets.ComboBox, "Find vehicle for:", "", ParamEnumArray.FromEnum(ECompartmentType) )]
	private int m_compartmentType;
		
	override void OnInit(AIAgent owner)
	{
		m_rGroup = AIGroup.Cast(owner);
		m_rAgent = owner;
		m_aOutCompartments = new ref array<BaseCompartmentSlot>;		
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		GetVariableIn("VehicleIn",m_vehicleEnt);
		if (m_vehicleEnt)
		{
			Vehicle vehicle = Vehicle.Cast(m_vehicleEnt);
			if (vehicle)
			{
				BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
				if (!compartmentMan)
					return ENodeResult.FAIL;
				int numOfComp = compartmentMan.GetCompartments(m_aOutCompartments);				
				for (int j = 0; j< numOfComp; j++ )
				{
					if (SCR_AIGetUsableVehicle.CompartmentClassToType(m_aOutCompartments[j].Type()) == m_compartmentType)
						if (!m_aOutCompartments[j].AttachedOccupant() && !m_aOutCompartments[j].IsCompartmentAccessible())
						{
							m_aOutCompartments[j].SetCompartmentAccessible(true);
							return ENodeResult.SUCCESS;									
						};										
				}			
			}
		}
		else
		{
			if (m_rGroup) 
				m_rAgent = m_rGroup.GetLeaderAgent();
			if (!m_rAgent)
				return ENodeResult.FAIL;
			
			GenericEntity ent = GenericEntity.Cast(m_rAgent.GetControlledEntity());
			m_rCompartmentAccess = CompartmentAccessComponent.Cast(ent.FindComponent(CompartmentAccessComponent));
			
			if (m_rCompartmentAccess)
			{
				BaseCompartmentSlot compartment = m_rCompartmentAccess.GetCompartment();
				if (!compartment)
					return ENodeResult.FAIL;
				compartment.SetCompartmentAccessible(true);
				return ENodeResult.SUCCESS;
			}
		}				
		return ENodeResult.FAIL;
	}	
	
	protected static ref TStringArray s_aVarsIn = {
		"VehicleIn"
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
		return "ReleaseUsedVehicle: before leaving used vehicle you should release lock on the seat AI took! Define vehicle if owner is not using a vehicle.";
	}
	
	void ~SCR_AIReleaseUsedVehicle()
	{
		if (m_aOutCompartments)
		{
			m_aOutCompartments.Clear();
			m_aOutCompartments = null;
		}
	}
};