//------------------------------------------------------------------------------------------------
class SCR_AIIsEndangered : DecoratorScripted
{
	static private string OUT_ENTITY = "EntityOut";
	
	SCR_AICombatComponent m_CombatComponent;
	
	//------------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{		
		if (!m_CombatComponent)
		{
			return false;
		}
		
		BaseTarget target = m_CombatComponent.GetEndangeringEnemy();

		if (target)
		{
			SetVariableOut(OUT_ENTITY, target.GetTargetEntity());
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Checks if there is known enemy who is aiming at me and returns its entity.";
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		OUT_ENTITY
	};
	protected override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
};
