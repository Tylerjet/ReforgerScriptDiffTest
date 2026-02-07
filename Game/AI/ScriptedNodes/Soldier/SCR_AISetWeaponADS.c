class SCR_AISetWeaponADS : AITaskScripted
{
	[Attribute("true", UIWidgets.CheckBox, "Use ADS",)]
	private bool m_bUseADS;

	static const string PORT_USE_ADS = "UseADS";
	
	private CharacterControllerComponent m_CharacterController;

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CharacterController)
			return ENodeResult.FAIL;

		bool useADS;
		if(!GetVariableIn(PORT_USE_ADS,useADS))
			useADS = m_bUseADS;
			
		if (m_CharacterController.IsWeaponADS() != useADS)
			m_CharacterController.SetWeaponADS(useADS);

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_USE_ADS
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		IEntity controlled = owner.GetControlledEntity();
		if (!controlled)
			return;

		m_CharacterController = CharacterControllerComponent.Cast(controlled.FindComponent(CharacterControllerComponent));
	}
};