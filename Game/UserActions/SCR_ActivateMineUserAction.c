//------------------------------------------------------------------------------------------------
class SCR_ActivateMineUserAction : ScriptedUserAction
{
	[Attribute("30", "Arming delay in seconds.")]
	protected float m_fArmingDelay;
	
	[Attribute("10", "How long the user action will be shown as inactive after starting arming.")]
	protected float m_fArmingProtectionTime;
	
	protected bool m_bCanArm = true;
	
	//------------------------------------------------------------------------------------------------
	void ActivationWrapper()
	{
		SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (pressureTriggerComponent)
			pressureTriggerComponent.Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	void OrientToForward(vector forward, vector mat[4])
	{
		vector origin = mat[3];
		vector basis[4];
		forward = -forward;
		Math3D.AnglesToMatrix(Vector(forward.VectorToAngles()[0], 0, 0), basis);
		Math3D.MatrixMultiply3(mat, basis, mat);
		mat[3] = origin;
	}
	
	//------------------------------------------------------------------------------------------------
	void AllowArming()
	{
		m_bCanArm = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_bCanArm = false;
		GetGame().GetCallqueue().CallLater(AllowArming, 1000 * m_fArmingProtectionTime);
		
		vector matUser[4], mat[4];
		pUserEntity.GetTransform(matUser);
		pOwnerEntity.GetTransform(mat);
		
		OrientToForward(matUser[2], mat);
		pOwnerEntity.SetTransform(mat);
		
		CharacterControllerComponent charController = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		if (charController)
		{
			CharacterAnimationComponent pAnimationComponent = charController.GetAnimationComponent();
			int itemActionId = pAnimationComponent.BindCommand("CMD_Item_Action");
			vector charWorldMat[4];
			pUserEntity.GetWorldTransform(charWorldMat);
			charWorldMat[3] = pOwnerEntity.GetOrigin();
			PointInfo ptWS = new PointInfo();
			ptWS.Set(null, "", charWorldMat);
			charController.TryUseItemOverrideParams(pOwnerEntity, false, itemActionId, 3, 0, 15.0, 0, 0, false, ptWS);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return m_bCanArm;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_PressureTriggerComponent mineTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!mineTriggerComponent)
			return false;
		
		if (mineTriggerComponent.IsActivated())
			return false;
		
		return super.CanBeShownScript(user);
	}
};