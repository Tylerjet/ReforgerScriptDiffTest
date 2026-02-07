class SCR_AdjustSignalAction : ScriptedSignalUserAction
{
	//! Adjustment step of normalized value
	[Attribute(defvalue: "0.1", desc: "Adjustment step [-1 to 1]", params: "-1 1 0.1")]
	protected float m_fAdjustmentStep;

	//! Flag for enabling adjustment with scroll wheel
	[Attribute(desc: "If action should wait for player to use their scroll wheel in order to change value")]
	protected bool m_bManualAdjustment;

	//! Flag for restarting the process when end value is reached
	[Attribute(desc: "Determines if this action will start from the begining when max value is reached - or from the other side if Adjustment Step is below 0")]
	protected bool m_bLoopAction;

	//! Name of action to control the input
	[Attribute(defvalue: "SelectAction", desc: "Input action for increase")]
	protected string m_sActionIncrease;

	//! Name of action to control the input
	[Attribute(desc: "Input action for decrease")]
	protected string m_sActionDecrease;

	//! Action start sound event name
	[Attribute(desc: "Action start sound event name")]
	protected string m_sActionStartSoundEvent;

	//! Action canceled sound event name
	[Attribute(desc: "Action canceled sound event name")]
	protected string m_sActionCanceledSoundEvent;

	//! Movement sound event name
	[Attribute(desc: "Movement sound event name")]
	protected string m_sMovementSoundEvent;

	//! Movement stop sound event name
	[Attribute(desc: "Movement stop sound event name")]
	protected string m_sMovementStopSoundEvent;

	//! Normalized current value
	protected float m_fTargetValue;

	//! Interacted with by main entity. Allows reading input actions.
	protected bool m_bIsAdjustedByPlayer;

	//! Sound component on owner entity
	protected SoundComponent m_SoundComponent;

	//! Last lerp value
	protected float m_fLerpLast;

	//! Movement sound AudioHandle
	protected AudioHandle m_MovementAudioHandle;

	//------------------------------------------------------------------------------------------------
	//!	Used to tell if this action is meant to be adjustable with usage of the scroll wheel
	bool IsManuallyAdjusted()
	{
		return m_bManualAdjustment;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_SoundComponent = SoundComponent.Cast(pOwnerEntity.FindComponent(SoundComponent));
		if (GetActionDuration() != 0)
			m_fAdjustmentStep /= Math.AbsFloat(GetActionDuration());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_bLoopAction && !m_bManualAdjustment)
		{
			if (m_fAdjustmentStep > 0 && GetCurrentValue() >= GetMaximumValue())
				return false;

			if (m_fAdjustmentStep < 0 && GetCurrentValue() <= GetMinimumValue())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Temporary fix for an issue of SetSendActionDataFlag not working properly from PerformAction
	void ToggleActionBypass()
	{
		HandleAction(1);
	}

	//------------------------------------------------------------------------------------------------
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		if (!m_bManualAdjustment)
			HandleAction(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		if (m_SoundComponent && m_sActionStartSoundEvent != string.Empty)
			m_SoundComponent.SoundEvent(m_sActionStartSoundEvent);

		m_bIsAdjustedByPlayer = SCR_PlayerController.GetLocalControlledEntity() == pUserEntity;

		if (!m_bIsAdjustedByPlayer)
			return;

		m_fTargetValue = Math.InverseLerp(GetMinimumValue(), GetMaximumValue(), GetCurrentValue());
		if (!GetActionDuration())
			ToggleActionBypass();

		if (!m_bManualAdjustment)
			return;

		if (!m_sActionIncrease.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_sActionIncrease, EActionTrigger.VALUE, HandleAction);

		if (!m_sActionDecrease.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_sActionDecrease, EActionTrigger.VALUE, HandleActionDecrease);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Play sound
		if (m_SoundComponent && m_sActionCanceledSoundEvent != string.Empty)
				m_SoundComponent.SoundEvent(m_sActionCanceledSoundEvent);

		if (!m_bIsAdjustedByPlayer)
			return;

		m_bIsAdjustedByPlayer = false;

		if (!m_bManualAdjustment)
			return;

		if (!m_sActionIncrease.IsEmpty())
			GetGame().GetInputManager().RemoveActionListener(m_sActionIncrease, EActionTrigger.VALUE, HandleAction);

		if (!m_sActionDecrease.IsEmpty())
			GetGame().GetInputManager().RemoveActionListener(m_sActionDecrease, EActionTrigger.VALUE, HandleActionDecrease);
	}

	//------------------------------------------------------------------------------------------------
	//! Increment target value
	//! \param[in] value multiplayer which will be applied to the step value
	void HandleAction(float value)
	{
		if (value == 0)
			return;

		if (m_bManualAdjustment)
			value /= Math.AbsFloat(value);

		value *= m_fAdjustmentStep;

		m_fTargetValue += value;
		if (m_bLoopAction)
		{
			if (value > 0 && float.AlmostEqual(GetCurrentValue(), GetMaximumValue()))
				m_fTargetValue = GetMinimumValue();
			else if (value < 0 && float.AlmostEqual(GetCurrentValue(), GetMinimumValue()))
				m_fTargetValue = GetMaximumValue();
		}

		if (float.AlmostEqual(m_fTargetValue, GetCurrentValue(), Math.AbsFloat(m_fAdjustmentStep)))
			return;

		// Round to adjustment step
		m_fTargetValue = Math.Floor(m_fTargetValue / m_fAdjustmentStep) * m_fAdjustmentStep;

		// Limit to min/max value
		m_fTargetValue = Math.Clamp(m_fTargetValue, GetMinimumValue(), GetMaximumValue());

		if (!float.AlmostEqual(m_fTargetValue, GetCurrentValue()))
			SetSendActionDataFlag();
	}

	//------------------------------------------------------------------------------------------------
	//! Decrement target value
	//! \param[in] value multiplayer which will be applied to the step value
	void HandleActionDecrease(float value)
	{
		HandleAction(-value);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays movement and stop movement sound events
	void PlayMovementAndStopSound(float lerp)
	{
		if (!m_SoundComponent)
			return;

		if (m_fLerpLast == lerp)
			return;

		if (float.AlmostEqual(lerp, 1))
		{
			if (!float.AlmostEqual(m_fLerpLast, 1))
			{
				m_SoundComponent.Terminate(m_MovementAudioHandle);
				if (m_sMovementStopSoundEvent != string.Empty)
					m_SoundComponent.SoundEvent(m_sMovementStopSoundEvent);
			}
		}
		else if (float.AlmostEqual(lerp, 0))
		{
			if (!float.AlmostEqual(m_fLerpLast, 0))
			{
				m_SoundComponent.Terminate(m_MovementAudioHandle);
				if (m_sMovementStopSoundEvent != string.Empty)
					m_SoundComponent.SoundEvent(m_sMovementStopSoundEvent);
			}
		}
		else
		{
			if (m_SoundComponent.IsFinishedPlaying(m_MovementAudioHandle) && m_sMovementStopSoundEvent != string.Empty)
				m_MovementAudioHandle = m_SoundComponent.SoundEvent(m_sMovementSoundEvent);
		}

		m_fLerpLast = lerp;
	}

	//------------------------------------------------------------------------------------------------
	//! Is the script broadcast to the server?
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is true this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Before performing the action the caller can store some data in it which is delivered to others.
	//! Only available for actions for which HasLocalEffectOnly returns false.
	override protected bool OnSaveActionData(ScriptBitWriter writer)
	{
		float lerp = Math.Lerp(GetMinimumValue(), GetMaximumValue(), m_fTargetValue);
		writer.WriteFloat01(lerp);

		SetSignalValue(m_fTargetValue);

		PlayMovementAndStopSound(lerp);

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

		float lerp;
		reader.ReadFloat01(lerp);

		m_fTargetValue = Math.InverseLerp(GetMinimumValue(), GetMaximumValue(), lerp);
		SetSignalValue(m_fTargetValue);

		PlayMovementAndStopSound(lerp);

		return true;
	}
}
