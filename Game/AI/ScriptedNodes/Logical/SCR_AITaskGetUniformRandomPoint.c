class SCR_AITaskGetUniformRandomPoint: AITaskScripted
{
	[Attribute("10", UIWidgets.EditBox, "Radius")]
	float m_Radius;
	
	[Attribute("0", UIWidgets.EditBox, "Minimum distance from specified position.")]
	float m_MinPositionRadius;
	
	protected const int MAX_NUMBER_OF_MIN_POS_ITERATIONS = 5;
	
	//------------------------------------------------------------------------------------------------
	protected vector GetUniformRandomPoint(vector origin, float radius)
	{		
		float a = Math.RandomFloat01() * Math.PI2;
		float r = radius * Math.Sqrt( Math.RandomFloat01() );
		float spawnX = r * Math.Cos(a);
		float spawnZ = r * Math.Sin(a);

		vector randomPos = origin;
		randomPos[0] = randomPos[0] + spawnX;
		randomPos[2] = randomPos[2] + spawnZ;
		randomPos[1] = origin[1]; //GetGame().GetWorldEntity().GetSurfaceY(x,z);
		
		return randomPos;
	}
	
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{	
		
	}	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent myAgent = AIAgent.Cast(owner);
		if (myAgent == null) { return ENodeResult.FAIL; }
		
		GenericEntity controlledEntity = GenericEntity.Cast(myAgent.GetControlledEntity());		
		if (controlledEntity == null) { return ENodeResult.FAIL; }
		
		// Get a Radius or use attribute value instead.
		if (GetVariableType(true, "Radius") == float)
		{
			float input = 0;	
			GetVariableIn("Radius", input);
			m_Radius = input;
		}
		
		// Get a Min Radius used when generating multiple random points in row so we can have distance between them specified by this radius.
		if (GetVariableType(true, "MinPositionRadius") == float)
		{
			float input = 0;
			GetVariableIn("MinPositionRadius", input);
			m_MinPositionRadius = input;
		}
		
		// Get an origin point or calculate from 0 0 0 instead.
		vector originVector = vector.Zero;
		if (GetVariableType(true, "Origin") == vector)
		{
			GetVariableIn("Origin", originVector);
		}
		
		// Get an posotion vector which works alongside with MinPositionRadius.
		vector positionVector = controlledEntity.GetOrigin();
		if (GetVariableType(true, "Position") == vector)
		{
			GetVariableIn("Position", positionVector);
		}
		
		vector randomVector;
		
		// When MinPosRadius is 0, then just generate random uniform position.
		if (m_MinPositionRadius == 0)
		{
			randomVector = GetUniformRandomPoint(originVector, m_Radius);
		}
		else // Else iterate to find position within min radius or best match in specified number of iterations. 
		{
			float biggestDistance = 0;
			
			for (int i; i < MAX_NUMBER_OF_MIN_POS_ITERATIONS; i++)
			{
				vector generatedVector = GetUniformRandomPoint(originVector, m_Radius);
				float distance = vector.Distance(generatedVector, originVector);
				
				// We got random point which matches minimum required distance.
				if (distance >= m_MinPositionRadius)
				{
					randomVector = generatedVector;
					break;
				}
				else if (distance > biggestDistance) // If we cannot get required point, lets then keep the best one which we found.
				{
					biggestDistance = distance;
					randomVector = generatedVector;
				}
			}
		}
		
		if (GetVariableType(false, "ResultVector") == vector)
		{
			SetVariableOut("ResultVector", randomVector);
		}
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Scripted Node: For obtaining uniform random point from specified positing with enchance of specifying min range from other defined position.";
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"Origin",
		"Radius",
		"Position",
		"MinPositionRadius"
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		"ResultVector"
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};