enum EAimingPreference
{
	NONE,
	RANDOM,
	AUTOMATIC,
	FIXED,	
};

class SCR_AIGetAimErrorOffset: AITaskScripted
{
	static const string PORT_ERROR_OFFSET = "ErrorOffset";
	static const string PORT_ANGULAR_SIZE = "AngularSize";
	static const string PORT_ENTITY = "EntityIn";
	static const float CLOSE_RANGE_THRESHOLD = 250.0;
	
	private vector m_vPositionInLocalSpace;
	private SCR_AICombatComponent m_CombatComponent;
	private SCR_AIInfoComponent m_InfoComponent;
	
	[Attribute("2", UIWidgets.ComboBox, "Hit zone selection", "", ParamEnumArray.FromEnum(EAimingPreference) )]
	private EAimingPreference m_eAimingPreference;
	
	[Attribute("1", UIWidgets.ComboBox, "Hit zone specification for fixed option", "", ParamEnumArray.FromEnum(EAICharacterAimZones) )]
	private EAICharacterAimZones m_eAimingHitzone;
	
	[Attribute("0.03", UIWidgets.EditBox, "Default PrecisionXY")]
	private float m_fDefaultPrecisionXY;
	
	private EAICharacterAimZones m_eCurrentHitzoneForAiming = EAICharacterAimZones.TORSO;
	private int m_iTorsoCount = 0;	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (GetVariableType(false, PORT_ERROR_OFFSET) != vector)
		{
			NodeError(this, owner, PORT_ERROR_OFFSET+" should be vector");
		}
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));	
		m_InfoComponent = SCR_AIInfoComponent.Cast(owner.FindComponent(SCR_AIInfoComponent));
		if (!m_InfoComponent)
			NodeError(this,owner, "Cannot find info component in GetAimErrorOffset!");			
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ERROR_OFFSET,
		PORT_ANGULAR_SIZE
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENTITY
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
    //------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		IEntity enemy;
		if (m_CombatComponent.GetCurrentEnemy())
			enemy = m_CombatComponent.GetCurrentEnemy().GetTargetEntity();
		else // for testing purposes, fallback
			GetVariableIn(PORT_ENTITY, enemy);
		
		if (enemy)
		{
			// get skill based on threat level
			//EAISkill currentSkill = GetSkillFromThreat(m_CombatComponent.GetAISkill(),m_InfoComponent.GetThreatState());
			EAISkill currentSkill = m_CombatComponent.GetAISkill();
			// choose a hitzone
			m_eCurrentHitzoneForAiming = SelectHitzone(m_eAimingPreference, m_eCurrentHitzoneForAiming);
			m_vPositionInLocalSpace = m_CombatComponent.GetAimingOffsetByHitzone(m_eCurrentHitzoneForAiming);
			// correction for close range
			// float distanceToEnemySq = vector.DistanceSq(enemy.GetOrigin(), owner.GetControlledEntity().GetOrigin()); 
			// currentSkill = GetSkillFromDistance(currentSkill, distanceToEnemySq);
			
			// get bounds from BB and multiply with random based on skill
					
			vector offsetX,offsetY;
			float angularSize = m_fDefaultPrecisionXY;
			
			string result;
			GetTargetAngularBounds(owner.GetControlledEntity(), enemy, offsetX, offsetY, angularSize);
			
			offsetX = GetRandomFactor(currentSkill,0) * offsetX * 0.5;
			offsetY = GetRandomFactor(currentSkill,0) * offsetY * 0.5;
			
			SetVariableOut(PORT_ERROR_OFFSET, m_vPositionInLocalSpace + offsetX + offsetY);
			SetVariableOut(PORT_ANGULAR_SIZE, angularSize);
			
			return ENodeResult.SUCCESS;
		}		
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	// returns bounding box of target projected to the plane perpendicular to vector conneting target and shooter 
	// WARNING: does not take into consideration changing stance on characters!
	void GetTargetAngularBounds(IEntity shooter, IEntity target, out vector sizeVectorInX, out vector sizeVectorInY, out float angularSize)
	{
		vector min, max, join, joinNorm, shooterCenter, targetCenter, originToCenter, x1, y1, xTilde, yTilde;
		float sizeX, sizeY;
		shooterCenter = shooter.GetOrigin();
		target.GetWorldBounds(min,max);
		targetCenter = (min + max) * 0.5;
		join = targetCenter - shooterCenter;
		joinNorm = join.Normalized();
		// calculate size vectors of BB, find plane that is allingned with the BB orientation and is intersecting target origin
		target.GetBounds(min,max);
		// rotate according to orientation of entity
		vector mat[3];
		target.GetWorldTransform(mat);
		min = min.Multiply3(mat);
		max = max.Multiply3(mat);
		// taking positive direction in + x,z plane, min and max will point in same halfplane
		min[0] = -min[0]; 
		min[2] = -min[2];
		// transforming vectors to center of BB if this is different than GetOrigin() 
		// - for characters it is different, some objects have pivot in BB center
		originToCenter = targetCenter - target.GetOrigin();
		xTilde = (min - originToCenter);
		yTilde = (max - originToCenter);
		// find plane that is perpendicular to the joinNorm vector between target and shooter
		if (vector.Dot(joinNorm * xTilde,vector.Up)>0)
		{
			x1 = (vector.Up * joinNorm).Normalized(); //must be normalized!
			y1 = joinNorm * x1; // is normalized by definition
		}	
		else
		{
 			x1 = (joinNorm * vector.Up).Normalized();
			y1 = x1 * joinNorm;
		}
		/*  result is projection of xTilde to the axis made by x1, 
			size of this vector is calculated as cos(alpha) = ||sizeVectorInX||/||xTilde||
			but also cos(alpha) = vector.Dot(xTilde,x1) / ||xTilde|| * 1, from there we get the final result:
		*/	
		sizeX = vector.Dot(xTilde, x1);
		sizeY = vector.Dot(yTilde, y1);
		sizeVectorInX = x1 * sizeX;
		sizeVectorInY = y1 * sizeY;
		
		// angular size of the target will be arctangens corner of projected bb / join between shooter and target <- this is the only distance dependent factor!
		angularSize = Math.Atan2(sizeX + sizeY, join.Length()) * Math.RAD2DEG;
		
		// debug showing calculated vectors
		/*
		ref Shape planeX = Shape.CreateArrow(shooterCenter, shooterCenter + x1, 0.1, COLOR_RED_A, ShapeFlags.ONCE); 
		ref Shape planeY = Shape.CreateArrow(shooterCenter, shooterCenter + y1, 0.1, COLOR_BLUE_A, ShapeFlags.ONCE); 
		ref Shape joinVec = Shape.CreateArrow(shooterCenter, targetCenter, 0.1, COLOR_YELLOW, ShapeFlags.ONCE);
		ref Shape BBMax = Shape.CreateArrow(targetCenter, targetCenter + xTilde, 0.1, COLOR_RED_A, ShapeFlags.ONCE);
		ref Shape BBMin = Shape.CreateArrow(targetCenter, targetCenter + yTilde, 0.1, COLOR_BLUE_A, ShapeFlags.ONCE);
		ref Shape resultSizeX = Shape.CreateArrow(shooterCenter, shooterCenter + sizeVectorInX, 0.2, COLOR_RED, ShapeFlags.ONCE); 
		ref Shape resultSizeY = Shape.CreateArrow(shooterCenter, shooterCenter + sizeVectorInY, 0.2, COLOR_BLUE, ShapeFlags.ONCE); 	
		PrintFormat("AngularSize of target is: %1", angularSize);
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	// returns random factor based on AI skill
	float GetRandomFactor(EAISkill skill,float mu)
	{
		float sigma;
		switch (skill)
		{
			case EAISkill.ROOKIE :
			{
				sigma = 4;
				break;
			}
			case EAISkill.REGULAR :
			{
				sigma = 2;
				break;
			}
			case EAISkill.VETERAN :
			{
				sigma = 0.5;
				break;
			}
			case EAISkill.EXPERT :
			{
				sigma = 0.1;
				break;
			}
			case EAISkill.CYLON :
			{
				return 0;
			}
		}
		// PrintFormat("Gauss: %1, sigma: %2, skill: %3",result,sigma,typename.EnumToString(EAISkill,skill));
		return Math.RandomGaussFloat(sigma,mu);		
	}
		
	//------------------------------------------------------------------------------------------------
	EAICharacterAimZones SelectHitzone(EAimingPreference aimingPreference, EAICharacterAimZones currentSelection)
	{
		switch (aimingPreference)
		{
			case EAimingPreference.AUTOMATIC:
			{
				switch(currentSelection)
				{
					case EAICharacterAimZones.HEAD:
					{
						return EAICharacterAimZones.LIMBS;
					}
					case EAICharacterAimZones.LIMBS:
					{
						return EAICharacterAimZones.TORSO;
					}
					case EAICharacterAimZones.TORSO:
					{
						if (m_iTorsoCount < 2)
						{
							m_iTorsoCount += 1;
							return EAICharacterAimZones.TORSO;
						}
						else 
						{
							m_iTorsoCount = 0;
							return EAICharacterAimZones.HEAD;
						}
					}
				};
				break;
			}
			case EAimingPreference.RANDOM: 
			{
				return Math.RandomInt(EAICharacterAimZones.TORSO,EAICharacterAimZones.LIMBS);
			}
			case EAimingPreference.FIXED:
			{
				return m_eAimingHitzone;
			}
		};
		return currentSelection;
	}
	
	//------------------------------------------------------------------------------------------------
	// returns skill corrected by current threat level and if AI can shoot under such suppression
	EAISkill GetSkillFromThreat(EAISkill inSkill, EAIThreatState threat)
	{
		switch (threat)
		{
			case EAIThreatState.THREATENED : 
			{		 
				switch (inSkill)
				{
					case EAISkill.ROOKIE :
					{
						return EAISkill.ROOKIE;
					}
					case EAISkill.REGULAR :
					{
						return EAISkill.ROOKIE;
					}
					case EAISkill.VETERAN :
					{
						return EAISkill.REGULAR;
					}
					case EAISkill.EXPERT :
					{
						return EAISkill.VETERAN;
					}
					case EAISkill.CYLON :
					{
						return EAISkill.CYLON;
					}
				};
				break;
			}
			case EAIThreatState.ALERTED :
			{
				switch (inSkill)
				{
					case EAISkill.ROOKIE :
					{
						return EAISkill.REGULAR;
					}
					case EAISkill.REGULAR :
					{
						return EAISkill.VETERAN;
					}
					case EAISkill.VETERAN :
					{
						return EAISkill.EXPERT;
					}
					case EAISkill.EXPERT :
					{
						return EAISkill.CYLON;
					}
					case EAISkill.CYLON :
					{
						return EAISkill.CYLON;
					}
				};
				break;
			}
			default :
			{
				return inSkill;
				break;
			}	
		}	
		return EAISkill.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	// returns skill corrected by enemy distance
	EAISkill GetSkillFromDistance(EAISkill inSkill, float distanceSq)
	{
		switch (inSkill)
		{
			case EAISkill.ROOKIE :
			{
				if (distanceSq < CLOSE_RANGE_THRESHOLD)
					return EAISkill.REGULAR;
				return EAISkill.ROOKIE;
			}
			case EAISkill.REGULAR :
			{
				if (distanceSq < CLOSE_RANGE_THRESHOLD)
					return EAISkill.VETERAN;
				return EAISkill.REGULAR;
			}
			case EAISkill.VETERAN :
			{
				if (distanceSq < CLOSE_RANGE_THRESHOLD)
					return EAISkill.EXPERT;
				return EAISkill.VETERAN;
			}
			case EAISkill.EXPERT :
			{
				if (distanceSq < CLOSE_RANGE_THRESHOLD)
					return EAISkill.CYLON;
				return EAISkill.EXPERT;
			}
			case EAISkill.CYLON :
			{
				return EAISkill.CYLON;
			}
		};
		return EAISkill.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Get aiming error position from the center of the target";}
};

