class SCR_AISelectWeapon: AITaskScripted
{
	IEntity m_Target;
	
    override bool VisibleInPalette() {return true;}

	protected static ref TStringArray s_aVarsIn = {
		"Target"
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
    
	protected static ref TStringArray s_aVarsOut = {
		"WeaponIndex"
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }

    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		GetVariableIn("Target",m_Target);
		if (!m_Target || !owner)
			return ENodeResult.FAIL;

		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
			return ENodeResult.FAIL;

		// TODO: Functionality
		
		SetVariableOut("WeaponIndex",0);
		return ENodeResult.SUCCESS;
    }
};

