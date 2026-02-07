class SCR_AISetStance : SCR_AICharacterStats
{
	[Attribute("0", UIWidgets.ComboBox, "Desired character stance", "", ParamEnumArray.FromEnum(ECharacterStance) )]
	private ECharacterStance m_eStance;

	static const string STANCE_PORT = "Stance";
	
	private CharacterControllerComponent m_CharacterController;

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_AIInfo || !m_CharacterController)
			return ENodeResult.FAIL;
		
		ECharacterStance stance;
		ECharacterStance allowedStance;
		
		if(!GetVariableIn(STANCE_PORT,stance))
			stance = m_eStance;
		
		AIGroup group = owner.GetParentGroup();
		if (!group)
			return ENodeResult.SUCCESS;
		
		SCR_AIGroupInfoComponent groupInfoComp = SCR_AIGroupInfoComponent.Cast(group.FindComponent(SCR_AIGroupInfoComponent));
		if (!groupInfoComp)
			return ENodeResult.FAIL;
		
		allowedStance = groupInfoComp.GetAllowedStance(stance);
		
		SCR_AIStanceHandling.SetStance(m_AIInfo, m_CharacterController, allowedStance);

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		STANCE_PORT
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_eStance: %1\n", typename.EnumToString(ECharacterStance, m_eStance));
		return s;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		
		if(owner.GetControlledEntity())
		{
			m_CharacterController = CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(CharacterControllerComponent));
			if (!m_CharacterController)
				NodeError(this, owner, "Can't find CharacterControllerComponent.");
		}	
	}
};
