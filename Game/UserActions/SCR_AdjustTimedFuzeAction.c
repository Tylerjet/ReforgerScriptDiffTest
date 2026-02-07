class SCR_AdjustTimedFuzeAction : SCR_AdjustSignalAction
{
	protected SCR_ExplosiveChargeComponent m_ChargeComp;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		m_ChargeComp = SCR_ExplosiveChargeComponent.Cast(pOwnerEntity.FindComponent(SCR_ExplosiveChargeComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_ChargeComp || m_ChargeComp.GetUsedFuzeType() != SCR_EFuzeType.NONE)
			return false;

		return super.CanBeShownScript(user);
	}

	override bool GetActionNameScript(out string outName)
	{
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;

		outName = WidgetManager.Translate(actionInfo.GetName(), m_fTargetValue);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (GetMaximumValue() - GetMinimumValue() != 0)
			return (m_fTargetValue - GetMinimumValue()) / (GetMaximumValue() - GetMinimumValue());

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		bool noChanges = float.AlmostEqual(m_fTargetValue, m_ChargeComp.GetFuzeTime());
		writer.WriteBool(noChanges);
		if (noChanges)
			return false;

		writer.WriteIntRange(m_fTargetValue, GetMinimumValue(), GetMaximumValue());
		if (m_ChargeComp)
			m_ChargeComp.SetFuzeTime(m_fTargetValue);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		if (m_bIsAdjustedByPlayer)
			return true;

		bool noChanges;
		reader.ReadBool(noChanges);
		if (noChanges)
			return true;

		int outVal;
		reader.ReadIntRange(outVal, GetMinimumValue(), GetMaximumValue());
		m_fTargetValue = outVal;
		if (float.AlmostEqual(m_fTargetValue, m_ChargeComp.GetFuzeTime()))
			return false;

		if (m_ChargeComp)
			m_ChargeComp.SetFuzeTime(m_fTargetValue);

		return true;
	}
}
