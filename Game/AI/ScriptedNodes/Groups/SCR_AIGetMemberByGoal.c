enum EGroupGoals
{
	NONE,
	HEAL,
	RESUPPLY,
};

class SCR_AIGetMemberByGoal: AITaskScripted
{
	static const string PORT_REQUIREMENTS_IN	= "RequirementsIn";
	static const string PORT_ENTITY_IN	= "EntityIn";
	static const string PORT_GROUP_MEMBER_OUT	= "GroupMemberOut";
	static const string PORT_IS_UNIQUE_OUT	= "IsUniqueOut";
	
	[Attribute("0", UIWidgets.ComboBox, "Find member best for goal", "", ParamEnumArray.FromEnum(EGroupGoals) )]
	protected EGroupGoals m_goalToAchieve;
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	int m_selectedIndex = 0;
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent || !m_GroupUtilityComponent.m_aListOfAIInfo)
			return ENodeResult.FAIL;
				
		bool notFound = true, isUnique = true, isRolePresent = false;
		SCR_AIInfoComponent aIInfoComponent;
		
		typename magazineWell;
		int magazineMaxCountIndex,magazineCount,magazineMaxCount = 0;
		
		IEntity inEntity;
		GetVariableIn(PORT_ENTITY_IN, inEntity);
		
		if (m_goalToAchieve == EGroupGoals.RESUPPLY && !GetVariableIn(PORT_REQUIREMENTS_IN, magazineWell))
			Debug.Error("No MagazineWell provided for resupply?!");
		
		for (int i = 0, length = m_GroupUtilityComponent.m_aListOfAIInfo.Count(); i < length; i++)
		{
			aIInfoComponent = m_GroupUtilityComponent.m_aListOfAIInfo[(i + m_selectedIndex) % length];
			if( !aIInfoComponent )
				return ENodeResult.FAIL;
			//prevent selecting requesting EntityID
			AIAgent agent = AIAgent.Cast(aIInfoComponent.GetOwner());
			if(!agent)
				continue;
			if(agent.GetControlledEntity() == inEntity)
				continue;
			
			switch (m_goalToAchieve)
			{
				case EGroupGoals.RESUPPLY:
				{
					magazineCount = aIInfoComponent.GetMagazineCountByWellType(magazineWell);
					if (magazineCount > magazineMaxCount)
					{
						if (isRolePresent)
							isUnique = false;
						else
							isRolePresent = true;						
						magazineMaxCount = magazineCount;
						magazineMaxCountIndex = i;
						if (aIInfoComponent.GetAIState() == EUnitAIState.AVAILABLE)
						{
							if (notFound) 
							{
								notFound = false;
								m_selectedIndex = (i + m_selectedIndex) % length;
							}	
							else
							{ 
								m_selectedIndex = (i + m_selectedIndex) % length;
								isUnique = false;
							}
						}
					}			
					break;
				}				
				case EGroupGoals.HEAL:
				{
					if (aIInfoComponent.HasRole(EUnitRole.MEDIC))
					{
						isRolePresent = true;
						if (aIInfoComponent.GetAIState() == EUnitAIState.AVAILABLE)
						{
							if (notFound) 
							{
								notFound = false;
								m_selectedIndex = (i + m_selectedIndex) % length;
							}	
							else 
								isUnique = false;		
						}
					}			
					break;
				}
			}
		}
		
		if (!isRolePresent)
		{
			return ENodeResult.FAIL;
		}
				
		if (!notFound)
		{
			AIAgent agent = AIAgent.Cast(m_GroupUtilityComponent.m_aListOfAIInfo[m_selectedIndex].GetOwner());
			if (agent)
				SetVariableOut(PORT_GROUP_MEMBER_OUT,agent);
			else 
				return ENodeResult.FAIL;
			
			SetVariableOut(PORT_IS_UNIQUE_OUT,isUnique);
			return ENodeResult.SUCCESS;
		}
		else if (m_goalToAchieve == EGroupGoals.RESUPPLY && isUnique) // I have only one unit that is not available = unit that requested the resupply 
		{
			ClearVariable(PORT_GROUP_MEMBER_OUT);
			SetVariableOut(PORT_IS_UNIQUE_OUT,isUnique);
			return ENodeResult.FAIL;
		}
		
		ClearVariable(PORT_GROUP_MEMBER_OUT);
		ClearVariable(PORT_IS_UNIQUE_OUT);
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanReturnRunning()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_GROUP_MEMBER_OUT,
		PORT_IS_UNIQUE_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUIREMENTS_IN,
		PORT_ENTITY_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }

	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return "Goal " + typename.EnumToString(EGroupGoals,m_goalToAchieve);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Getter returns group member available for specific role";
	}	
};