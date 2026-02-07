/*
System to move any ammobelt UV's that are currently being fired
*/

class SCR_AnimatedBeltSystem : GameSystem
{
	protected ref array<SCR_AnimatedBeltComponent> m_aComponents = {};

	//------------------------------------------------------------------------------------------------
	protected override void OnInit()
	{
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetTimeSlice();

		foreach (SCR_AnimatedBeltComponent comp : m_aComponents)
		{
			comp.Update(timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Register(SCR_AnimatedBeltComponent component)
	{
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
		
		if (!IsEnabled())
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_AnimatedBeltComponent component)
	{
		m_aComponents.RemoveItem(component);
		if (m_aComponents.IsEmpty())
			Enable(false);
	}
}
