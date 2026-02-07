class SCR_UnflipVehicleAction : SCR_PushVehicleAction
{
	protected vector m_vAngles;
	protected ref array<IEntity> m_aUsers = {};

	//------------------------------------------------------------------------------------------------
	//! Can this action be shown in the UI to the provided user entity?
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		m_vAngles = GetOwner().GetAngles();
		if (float.AlmostEqual(0, m_vAngles[0], 45) && float.AlmostEqual(0, m_vAngles[2], 45))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when someone tries to perform the continuous action, user entity is typically character
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		// Offset force target slightly above the center of mass to provide better leverage
		// Slightly unrealistic, but it should be good for gameplay.
		vector mins, maxs;
		IEntity rootEntity = pOwnerEntity.GetRootParent();
		rootEntity.GetWorldBounds(mins, maxs);
		vector forceLeverage = vector.Up * (maxs[1] - mins[1]) * 0.3;
		ApplyForce(rootEntity, pUserEntity, timeSlice, forceLeverage);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);

		if (!m_aUsers.Contains(pUserEntity))
			m_aUsers.Insert(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.OnActionCanceled(pOwnerEntity, pUserEntity);

		m_aUsers.RemoveItem(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! If overridden and true is returned, outName is returned when BaseUserAction.GetActionName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetActionNameScript(out string outName)
	{
		UIInfo info = GetUIInfo();
		if (!info)
			return false;

		int userCount = m_aUsers.Count();
		int suggestedUsers;
		Physics physics = GetOwner().GetPhysics();
		if (physics)
			suggestedUsers = Math.Ceil(physics.GetMass() / m_fForceLimit);

		outName = info.GetName();
		ActionNameParams[0] = userCount.ToString();
		ActionNameParams[1] = suggestedUsers.ToString();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the progress of this action in seconds.
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return 1;

		int suggestedUsers = Math.Ceil(physics.GetMass() / m_fForceLimit);

		return m_aUsers.Count() / suggestedUsers;
	}

	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is false this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return true;
	}
}
