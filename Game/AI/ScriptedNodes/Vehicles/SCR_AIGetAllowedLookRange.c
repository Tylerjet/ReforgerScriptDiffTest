class SCR_AIGetAllowedLookRange : AITaskScripted
{
	protected static const string RANGE_PORT = "AngularRangeDeg";
	protected static const string LOOK_AXIS_PORT = "LookAxis";
	protected static const string IS_FREE_LOOK = "IsFreeLook";
	protected static const float FREE_LOOK_ANGLE = 30.0;	
	
	[Attribute("20", UIWidgets.EditBox, "Distance for looking" )]
	protected float m_fDistance;
	
	
	
	protected static ref TStringArray s_aVarsOut = { LOOK_AXIS_PORT, RANGE_PORT, IS_FREE_LOOK };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner.GetControlledEntity());
		if (!character)
			return ENodeResult.FAIL;
		if (!character.IsInVehicle())
			return ENodeResult.FAIL;
		CompartmentAccessComponent compAcc = character.GetCompartmentAccessComponent();
		if (!compAcc)
			return ENodeResult.FAIL;
		BaseCompartmentSlot comp = compAcc.GetCompartment();
		if (!comp)
			return ENodeResult.FAIL;
		vector aimingLimitHoriz, aimingLimitVert, forwardVec;
		IEntity vehicle = comp.GetVehicle();
		forwardVec = vehicle.GetTransformAxis(2).Normalized();
		float angleBound;
		bool isFreeLook = true;
		
		TurretCompartmentSlot turret = TurretCompartmentSlot.Cast(comp);
		if (turret)
		{
			TurretControllerComponent turretController = TurretControllerComponent.Cast(turret.GetController());
			if (!turretController)
				return ENodeResult.FAIL;		
			TurretComponent turretComponent = turretController.GetTurretComponent();
			if (!turretComponent)
				return ENodeResult.FAIL;
			turretComponent.GetAimingLimits(aimingLimitHoriz,aimingLimitVert);
			angleBound = aimingLimitHoriz[1];
			isFreeLook = false;		
		}	
		else
		{
			angleBound = FREE_LOOK_ANGLE;		
		}
		
		SetVariableOut(LOOK_AXIS_PORT, comp.GetPosition() + forwardVec * m_fDistance);
		SetVariableOut(RANGE_PORT, angleBound);
		SetVariableOut(IS_FREE_LOOK, isFreeLook);
				
		return ENodeResult.SUCCESS;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() { return true; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override string GetOnHoverDescription()
	{
		return "GetAllowedLookRange: Gets the range of look for character in vehicle";
	}
};