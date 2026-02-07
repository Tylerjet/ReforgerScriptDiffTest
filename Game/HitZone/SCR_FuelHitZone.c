class SCR_FuelHitZone : ScriptedHitZone
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.Auto, desc: "Fuel Tank ID", category: "Fuel Damage Config")]
	private int m_iFuelTankID;
	
	private	SCR_FuelNode m_pFuelTank;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		SetFuelNodeID(m_iFuelTankID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assign hitzone to a simulation fuel tank
	void SetFuelNodeID(int fuelNodeID)
	{
		m_iFuelTankID = fuelNodeID;
		
		IEntity parent = SCR_EntityHelper.GetMainParent(GetOwner(), true);
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(parent.FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return;
		
		SCR_FuelNode scrFuelNode;
		array<BaseFuelNode> fuelNodes = {};
		fuelManager.GetFuelNodesList(fuelNodes);
		foreach (BaseFuelNode fuelNode: fuelNodes)
		{
			scrFuelNode = SCR_FuelNode.Cast(fuelNode);
			if (scrFuelNode && scrFuelNode.GetFuelTankID() == m_iFuelTankID)
			{
				m_pFuelTank = scrFuelNode;
				break;
			}
		}
		
		UpdateFuelTankState();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();
		
		UpdateFuelTankState();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateFuelTankState()
	{
		EDamageState state = GetDamageState();
 		float health = GetDamageStateThreshold(state);
		
		if (m_pFuelTank)
			m_pFuelTank.SetHealth(health);
	}
};
