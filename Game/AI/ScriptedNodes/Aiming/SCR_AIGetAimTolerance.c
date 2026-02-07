class SCR_AIGetAimTolerance: AITaskScripted
{
	static const string PORT_TOLERANCE = "AimingTolerance";
	static const string PORT_ENTITY = "EntityIn";
	static const string PORT_ANGULAR_SIZE = "AngularSizeIn";
	static const float BIG_TOLERANCE = 10.0;
	static const float NORMAL_TOLERANCE = 2.2;
	
	private SCR_AICombatComponent m_CombatComponent;
	private SCR_AIUtilityComponent m_Utility;
	
	[Attribute("0.5", UIWidgets.EditBox, "FractionOfAngularSize", precision : 8)]
	private float m_fPrecisionDefault;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		if (GetVariableType(false, PORT_TOLERANCE) != float)
		{
			NodeError(this, owner, PORT_TOLERANCE +" should be float");
		}
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));		
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {PORT_TOLERANCE};
	override array<string> GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ANGULAR_SIZE,
		PORT_ENTITY
	};
	override array<string> GetVariablesIn()
	{
		return s_aVarsIn;
	}
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		
		if (!m_CombatComponent || !m_Utility)
			return ENodeResult.FAIL;
		
		IEntity enemy;
		if (m_CombatComponent.GetCurrentEnemy())
			enemy = m_CombatComponent.GetCurrentEnemy().GetTargetEntity();
		else 
			GetVariableIn(PORT_ENTITY, enemy);
				
		if (enemy)
		{
			bool setBigTolerance;
			float angularSize;
			float tolerance;
			
			if(!GetVariableIn(PORT_ANGULAR_SIZE,angularSize))
				angularSize = 1.0;
			
			// basic tolerance based on angular size of target
			tolerance = angularSize * m_fPrecisionDefault;
			
			// TODO: weapon sway contribution
			
			// angular speed
			tolerance *= GetAngularSpeedFactor(enemy, setBigTolerance);
			
			// suppression of shooter
			//tolerance *= GetSuppressionFactor(m_Utility.m_ThreatSystem.GetThreatMeasure(), setBigTolerance);
			
			if (setBigTolerance)
				tolerance = BIG_TOLERANCE;
			else 
			{
			// weapon type tolerance modifier
				tolerance *= GetWeaponTypeFactor(m_CombatComponent.GetCurrentWeaponType());
			}
			
			//PrintFormat("Aiming tolerance is: %1", tolerance);
			SetVariableOut(PORT_TOLERANCE, tolerance);
			return ENodeResult.SUCCESS;
		}
		else
		{
			SetVariableOut(PORT_TOLERANCE, NORMAL_TOLERANCE);
			return ENodeResult.SUCCESS;
		}
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	
	float GetAngularSpeedFactor(IEntity enemy, out bool setBigTolerance)
	{
		Physics ph = enemy.GetPhysics();
		if (ph)
		{
			float angularVelocity = ph.GetAngularVelocity().Length();
			if (angularVelocity < 0.07) // rougly 4 degs in radians
				return 1.0;
			else if (angularVelocity < 0.17) // roughly 10 degs in radians
				return 2;
		}	
		setBigTolerance = true;
		return 0;
	}
	
	float GetSuppressionFactor(EAIThreatState threat, out bool setBigTolerance)
	{
		switch (threat)
		{
			case EAIThreatState.THREATENED : 
			{		 
				setBigTolerance = true;
				return 0;
				break;
			}
			case EAIThreatState.ALERTED :
			{
				return 2.0;
				break;
			}
			default :
			{
				return 1.0;	
				break;
			}
		}
		return 0;
	}
	
	float GetWeaponTypeFactor(EWeaponType weaponType)
	{
		switch(weaponType)
		{
			case EWeaponType.WT_RIFLE:
			{
				return 1.0;
			}
			case EWeaponType.WT_MACHINEGUN:
			{
				return 2.0;
			}
			case EWeaponType.WT_HANDGUN:
			{
				return 1.5;
			}
			case EWeaponType.WT_FRAGGRENADE:
			{
				return 3.0;
			}
			case EWeaponType.WT_SMOKEGRENADE:
			{
				return 4.0;
			}
			case EWeaponType.WT_ROCKETLAUNCHER:
			{
				return 0.5;
			}
			case EWeaponType.WT_SNIPERRIFLE:
			{
				return 0.25;
			}
		}
		return 1.0;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription() {return "Get aiming tolerance for decorator AIM";}
};

