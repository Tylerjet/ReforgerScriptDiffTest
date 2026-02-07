class SCR_AIDecoIsFireTeamMember : DecoratorScripted
{
	static const string PORT_MEMBER_IN = "MemberIn";
	static const string PORT_FIRETEAM_IN = "FireteamIn";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_MEMBER_IN,
		PORT_FIRETEAM_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
/*
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{	
		// make sure it works when uncomenting
		// EFireTeams was int internaly actualy
		if (GetVariableType(true, PORT_FIRETEAM_IN) != EFireTeams)
		{
			NodeError(this, owner, PORT_FIRETEAM_IN + " has to be EFireTeams");
		}
		
		if (GetVariableType(true, PORT_MEMBER_IN) != AIAgent)
		{
			NodeError(this, owner, PORT_MEMBER_IN + " has to be AIAgent");
		}
		
		//TODO uncoment OnInit
	}
*/
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		AIAgent agent;
		EFireTeams fireTeam;
		GetVariableIn(PORT_MEMBER_IN, agent);
		GetVariableIn(PORT_FIRETEAM_IN, fireTeam);
		
		if (agent)
		{
			SCR_AIInfoComponent aiInfo = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
			if (aiInfo)
			{
				return aiInfo.GetFireTeam() == fireTeam;
			}		
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "SCR_AIDecoIsFireteamMember: Checks if AIAgent is assigned to the given fireteam";
	}
};