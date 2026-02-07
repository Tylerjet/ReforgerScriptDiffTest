class SCR_RadioTuningUserAction : SCR_InventoryAction
{
	protected SCR_RadioComponent m_RadioComp;
	
	//------------------------------------------------------------------------------------------------
	[Attribute("0")]
	protected bool m_bTuneUp;

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_RadioComp)
			return false;

		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}

	protected override void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_RadioComp.ChangeFrequencyStep(m_bTuneUp))
			manager.PlayItemSound(pOwnerEntity, SCR_SoundEvent.SOUND_ITEM_RADIO_TUNE_ERROR);
		else
		{
			if (m_bTuneUp)
				manager.PlayItemSound(pOwnerEntity,SCR_SoundEvent.SOUND_ITEM_RADIO_TUNE_UP);
			else
				manager.PlayItemSound(pOwnerEntity,SCR_SoundEvent.SOUND_ITEM_RADIO_TUNE_DOWN);
		}
	}

	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_RadioComp = SCR_RadioComponent.Cast(pOwnerEntity.FindComponent(SCR_RadioComponent));
	}
	
	override bool GetActionNameScript(out string outName)
	{
		float targetFreq = m_RadioComp.GetRadioComponent().GetFrequency() + m_RadioComp.GetRadioComponent().GetFrequencyResolution();
		if (!m_bTuneUp)
			targetFreq = m_RadioComp.GetRadioComponent().GetFrequency() - m_RadioComp.GetRadioComponent().GetFrequencyResolution();

		outName = WidgetManager.Translate("#AR-UserAction_TuneRadioToFrequency", targetFreq / 1000);
		return true;
	}
};