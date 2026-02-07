class SCR_BayonetComponentClass : ScriptComponentClass
{
}

class SCR_BayonetComponent : ScriptComponent
{
	[Attribute("100", desc: "Size of damage dealt by the weapon with bayonet attached", category: "Global")]
	protected float m_fDamage;
	
	[Attribute("1.5", desc: "Range of the weapon [m] with bayonet attached", category: "Global")]
	protected float m_fRange;
	
	[Attribute("0.05", desc: "Accuracy of melee attacks with bayonet attached. Smaller values are more accurate", category: "Global")]
	protected float m_fAccuracy;
	
	[Attribute(desc: "Start position of trace used for melee attacks", category: "Global")]
	protected ref PointInfo m_TracePosition;
	
	protected float m_fBloodStainLevel = 0;
	
	//! Blood visibility steps - cannot be 0
	protected const int BLOOD_STEP_SIZE = 10;
	
	//------------------------------------------------------------------------------------------------	
	//! Value of damage dealt to the target
	float GetDamage()
	{
		return m_fDamage;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Range in meters that is used as max raycast length
	float GetRange()
	{
		return m_fRange;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Size of the raysphere used to trace the target
	float GetAccuracy()
	{
		return m_fAccuracy;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds blood texture to bayonet
	void AddBloodToBayonet()
	{
		Rpc_AddBloodToBayonet();
		Rpc(Rpc_AddBloodToBayonet);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_AddBloodToBayonet()
	{
		if (m_fBloodStainLevel >= 255.0)
			return;
		
		ParametricMaterialInstanceComponent materialComponent = ParametricMaterialInstanceComponent.Cast(GetOwner().FindComponent(ParametricMaterialInstanceComponent));
		if (!materialComponent)
			return;
		
		m_fBloodStainLevel += 255.0 / BLOOD_STEP_SIZE;
		
		materialComponent.SetUserParam2(Math.Clamp(m_fBloodStainLevel, 0.0, 255.0));
	}
}
