class SCR_AdjustCollimatorElevationAction : SCR_AdjustCollimatorAction
{
	[Attribute(defvalue: "1", desc: "")]
	float m_fAngleUnit;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		bool saved = super.OnSaveActionData(writer);
		bool noChanges = float.AlmostEqual(m_fTargetValue, m_SightsComponent.GetVerticalAngularCorrection());
		writer.WriteBool(noChanges);
		if (noChanges)
			return saved;

		writer.WriteFloat(m_fTargetValue);
		if (m_SightsComponent)
			m_SightsComponent.SetVerticalAngularCorrection(m_fTargetValue * m_fAngleUnit);
		
				
		Print(m_fTargetValue);
		Print(m_SightsComponent.GetVerticalAngularCorrection());

		return saved;
	}

	//------------------------------------------------------------------------------------------------
	//! If the one performing the action packed some data in it everybody receiving the action.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	//! Only triggered if the sender wrote anyting to the buffer.
	override protected bool OnLoadActionData(ScriptBitReader reader)
	{
		bool loaded = super.OnLoadActionData(reader);
		if (m_bIsAdjustedByPlayer)
			return loaded;

		bool noChanges;
		reader.ReadBool(noChanges);
		if (noChanges)
			return loaded;

		int outVal;
		reader.ReadFloat(outVal);
		m_fTargetValue = outVal;
		if (float.AlmostEqual(m_fTargetValue, m_SightsComponent.GetVerticalAngularCorrection()))
			return loaded;

		if (m_SightsComponent)
			m_SightsComponent.SetVerticalAngularCorrection(m_fTargetValue * m_fAngleUnit);
		
		return loaded;
	}
}
