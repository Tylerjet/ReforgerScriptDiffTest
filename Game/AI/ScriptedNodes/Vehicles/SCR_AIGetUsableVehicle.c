class SCR_AIGetUsableVehicle : AITaskScripted
{
	static const string PORT_COMPARTMENT = "CompartmentIn";
	static const string PORT_VEHICLE = "VehicleOut";
	
	[Attribute("0", UIWidgets.ComboBox, "Find vehicle for:", "", ParamEnumArray.FromEnum(ECompartmentType) )]
	private int m_compartmentType;
	
	[Attribute("0", UIWidgets.CheckBox, "Occupy the vehicle?")]
	private bool m_bOccupyVehicle;
	
	private ref array<BaseCompartmentSlot> m_aOutCompartments;
	private ref BaseCompartmentSlot compartmentUsed;
	private ECompartmentType m_compType;
	
	override void OnInit(AIAgent owner)
	{
		m_aOutCompartments = new ref array<BaseCompartmentSlot>;		
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		Vehicle vehicle;
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group) 
			group = SCR_AIGroup.Cast(owner.GetParentGroup());
		if (!group)
			return ENodeResult.FAIL;
		
		if (!GetVariableIn(PORT_COMPARTMENT,m_compType))
			m_compType = m_compartmentType;
		
		array<IEntity> vehicles = {};
		group.GetUsableVehicles(vehicles);
		if (!vehicles)
		 return ENodeResult.FAIL;
		
		for(int i = 0, lenght = vehicles.Count(); i < lenght; i++)
		{
			vehicle = Vehicle.Cast(vehicles[i]);
			if (vehicle)
			{
				BaseCompartmentManagerComponent compartmentMan = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
				if (!compartmentMan)
					break;
				int numOfComp = compartmentMan.GetCompartments(m_aOutCompartments);				
				for (int j = 0; j< numOfComp; j++ )
				{
					if (CompartmentClassToType(m_aOutCompartments[j].Type()) == m_compType)
						if (!m_aOutCompartments[j].AttachedOccupant() && m_aOutCompartments[j].IsCompartmentAccessible())
						{
							SetVariableOut("VehicleOut",vehicle);
							if (m_bOccupyVehicle)
							{
								compartmentUsed = m_aOutCompartments[j];	
								compartmentUsed.SetCompartmentAccessible(false);
							};
							return ENodeResult.SUCCESS;									
						};										
				}			
			}			
		}
		ClearVariable("VehicleOut");
		return ENodeResult.FAIL;
	}	
	
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (compartmentUsed)
		{
			compartmentUsed.SetCompartmentAccessible(true);
			compartmentUsed = null;
		}	
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_COMPARTMENT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	override bool VisibleInPalette()
    {
        return true;
    }
	
	override string GetOnHoverDescription()
	{
		return "GetUsableVehicle: takes vehicles from the list of known vehicles of group and checks if the slot of m_compartmentType is available";
	}
	
	static ECompartmentType CompartmentClassToType(typename type)
	{
		switch (type)
		{
			case PilotCompartmentSlot:	return ECompartmentType.Pilot;
			case CargoCompartmentSlot: 	return ECompartmentType.Cargo;
			case TurretCompartmentSlot:	return ECompartmentType.Turret;
		}
		return 0;			
	}
	
	void ~SCR_AIGetUsableVehicle()
	{
		if (m_aOutCompartments)
		{
			m_aOutCompartments.Clear();
			m_aOutCompartments = null;
		}	
	}
};