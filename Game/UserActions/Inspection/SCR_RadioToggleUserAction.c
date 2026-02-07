class SCR_RadioToggleUserAction : SCR_InventoryAction
{
	protected SCR_RadioComponent m_RadioComp;

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_RadioComp)
			return false;

		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}
	
	protected override void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_RadioComp.RadioToggle();
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RadioComp = SCR_RadioComponent.Cast(pOwnerEntity.FindComponent(SCR_RadioComponent));
	}

	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	override bool GetActionNameScript(out string outName)
	{
		if (m_RadioComp.GetRadioComponent().IsPowered())
			outName = "#AR-UserAction_TurnOff";
		else
			outName = "#AR-UserAction_TurnOn";

		return true;
	}
};