class SCR_FuelHitZone : ScriptedHitZone
{
	[Attribute( defvalue: "1", uiwidget: UIWidgets.Auto, desc: "Fuel Tank ID", category: "Fuel Damage Config")]
	protected int m_iFuelTankID;
	
	protected SCR_FuelNode m_FuelTank;
	
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
				m_FuelTank = scrFuelNode;
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
		if (!m_FuelTank)
			return;

		EDamageState state = GetDamageState();
 		float health = GetDamageStateThreshold(state);
		m_FuelTank.SetHealth(health);
	}
};
