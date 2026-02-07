//! Returns true when character is bleeding
class SCR_AIDecoTestIsBleeding : DecoratorTestScripted
{
	SCR_CharacterDamageManagerComponent m_DamageMgr;
	
	override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!m_DamageMgr)
			m_DamageMgr = SCR_CharacterDamageManagerComponent.Cast(controlled.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_DamageMgr)
			return false;
		
		return m_DamageMgr.IsDamagedOverTime(EDamageType.BLEEDING);
	}
}