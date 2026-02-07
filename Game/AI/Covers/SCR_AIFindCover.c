class SCR_AIFindCover : AITaskScripted
{
	// Inputs
	protected static const string PORT_TARGET_POS = "TargetPos";
	protected static const string PORT_COVER_QUERY_PROPERTIES = "CoverQueryProps";
	
	// Outputs
	protected static const string PORT_COVER_LOCK = "CoverLock";
	
	
	protected SCR_AICombatMoveState m_State;

	
#ifdef WORKBENCH
	protected ref array<ref Shape> m_aDebugShapes = {};
	protected ref Shape m_CoverShape;

	protected static const ShapeFlags m_SphereFlags = ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP;
	protected static const ShapeFlags m_LineFlags = ShapeFlags.NOOUTLINE|ShapeFlags.TRANSP;
	protected static const float DEBUGSPHERE_RADIUS = 0.1;
#endif

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utilityComp)
			m_State = utilityComp.m_CombatMoveState;
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity ownerEntity = owner.GetControlledEntity();
		if (!ownerEntity || !m_State)
			return ENodeResult.FAIL;

		//------------------------------------------------------------------------------------------------
		// Read inputs
		
		CoverQueryProperties queryProps;
		GetVariableIn(PORT_COVER_QUERY_PROPERTIES, queryProps);
		if (!queryProps)
			return ENodeResult.FAIL;

		//------------------------------------------------------------------------------------------------
		// Find cover

		CoverQueryComponent coverComp = GetCoverQueryComponent(owner);
		if (!coverComp)
			return ENodeResult.FAIL;

#ifdef WORKBENCH
		ClearDebug();
#endif

		vector coverPos, coverTallestPos;
		int tilex, tiley, coverId;
		
		ECoverSearchState coverSearchState = coverComp.GetBestCover("Soldiers", queryProps, coverPos, coverTallestPos, tilex, tiley, coverId);
		if (coverSearchState == ECoverSearchState.RUNNING)
			return ENodeResult.RUNNING;
		
		bool coverFound = coverSearchState == ECoverSearchState.SUCCESS;
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
		{
			m_aDebugShapes.Insert(Shape.CreateSphere(Color.GREEN, m_SphereFlags, ownerEntity.GetOrigin(), 0.2));
		}
#endif

		if (!coverFound)
		{
			// Release previous cover lock
			//m_State.ReleaseCover(); // Do not release previous cover if new one was not found. We still want to occupy it.
			ClearVariable(PORT_COVER_LOCK);
			return ENodeResult.FAIL;
		}

		// Create new cover lock, and release the old one
		m_State.AssignCover(new SCR_AICoverLock(tilex, tiley, coverId, coverPos, coverTallestPos));
		
		SetVariableOut(PORT_COVER_LOCK, m_State.GetAssignedCover());
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_DEBUG_COVERS))
		{
			m_CoverShape = Shape.CreateSphere(Color.PINK, m_SphereFlags, coverPos, 0.5);
		}
#endif

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	CoverQueryComponent GetCoverQueryComponent(AIAgent owner)
	{
		AIGroup myGroup = owner.GetParentGroup();
		if (!myGroup)
			return null;
		CoverQueryComponent coverComp = CoverQueryComponent.Cast(myGroup.FindComponent(CoverQueryComponent));
		return coverComp;
	}

	protected static ref TStringArray s_aVarsIn = {
		PORT_COVER_QUERY_PROPERTIES
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_COVER_LOCK
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}

	override static protected bool VisibleInPalette() {	return true; }

	override string GetOnHoverDescription()
	{
		return "Finds and locks cover through cover manager. Keep in mind that cover is also assigned to CombatMoveState!";
	}
	
	override bool CanReturnRunning() { return true; }

	#ifdef WORKBENCH
	private void ClearDebug()
	{
		m_aDebugShapes.Clear();
	}
	#endif
}
