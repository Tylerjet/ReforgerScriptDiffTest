class SCR_AIGetRandomVectorInCone: AITaskScripted
{
	static const string POSITION_PORT			= "WantedPosition";
	static const string DIRECTION_PORT			= "WantedDirection";	
	static const string RANGE_PORT				= "AngularRange";
	static const string POSITION_OUT_PORT		= "PositionOut";
	static const string DIRECTION_OUT_PORT		= "DirectionOut";	
	
	[Attribute("360", UIWidgets.EditBox, "Limit of random angle +/-" )]
	protected float m_fRange;

#ifdef WORKBENCH		
	protected ref Shape m_Shape;
#endif	
	
	//-------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() 
	{	
		return true;
	}
	
	//-------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		POSITION_PORT,
		DIRECTION_PORT,
		RANGE_PORT
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
    
	protected static ref TStringArray s_aVarsOut = {
		POSITION_OUT_PORT,
		DIRECTION_OUT_PORT
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		vector wantedVector, wantedLocation, result;
		float range, angle, distance;
		bool calculateLocation = true;
		IEntity controlledEntity = owner.GetControlledEntity();
		if (!controlledEntity)
		{
			AIGroup gr = AIGroup.Cast(owner);
			if(!gr)
				return ENodeResult.FAIL;
			controlledEntity = gr.GetLeaderEntity();	
		};
				
		if (!GetVariableIn(POSITION_PORT, wantedLocation))
		{
			calculateLocation = false;
			if (!GetVariableIn(DIRECTION_PORT, wantedVector)) 
			{
				NodeError(this, owner, "Not provided wanted position nor wanted direction!");
				return ENodeResult.FAIL;
			}						
		}
		else
		{
			wantedVector = wantedLocation - controlledEntity.GetOrigin();
		}	
		
		if(!GetVariableIn(RANGE_PORT, range))
			range = m_fRange;
		
		distance = wantedVector.Length();
		angle = Math.Atan2(wantedVector[2],wantedVector[0]);
		
		angle += Math.RandomFloat(-range, range) *  Math.DEG2RAD; // adding random angle
		result[0] =  Math.Cos(angle) * distance;
		result[2] =  Math.Sin(angle) * distance;
		result[1] =  wantedVector[1];
		
		if (calculateLocation)
		{
#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
				m_Shape = Shape.CreateSphere(COLOR_YELLOW, ShapeFlags.NOZBUFFER, controlledEntity.GetOrigin() + result, 0.5);
#endif		
			SetVariableOut(POSITION_OUT_PORT, result + controlledEntity.GetOrigin());
			ClearVariable(DIRECTION_OUT_PORT);
		}
		else
		{
#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
				m_Shape = Shape.CreateArrow(controlledEntity.GetOrigin(), controlledEntity.GetOrigin() + result, 0.5, COLOR_YELLOW, ShapeFlags.NOZBUFFER);			
#endif		
			ClearVariable(POSITION_OUT_PORT);
			SetVariableOut(DIRECTION_OUT_PORT, result);
		}
		return ENodeResult.SUCCESS;		
    }
	
	protected override string GetOnHoverDescription()
	{
		return "Returns either location or direction that is within 2D cone of the provided location / direction input.";
	}
};

