// Script File//------------------------------------------------------------------------------------------------
class SCR_AIIsTargetVisible : DecoratorScripted
{
	[Attribute("1", UIWidgets.EditBox, "How long in past target becomes invisibile", "")]
	float m_lastSeenMax;
	
	private PerceptionComponent m_PerceptionComponent;
	
	//-----------------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_PerceptionComponent = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		IEntity m_target;
		GetVariableIn("TargetEntityIn",m_target);
		ClearVariable("LastPositionOut");
		
		if (!m_target || !m_PerceptionComponent)
			return false;
		BaseTarget targetInPerception;
		
		targetInPerception = m_PerceptionComponent.GetTargetPerceptionObject(m_target,ETargetCategory.ENEMY);
		if (targetInPerception)
		{
			SetVariableOut("LastPositionOut",targetInPerception.GetLastSeenPosition());	
			return IsCurrentTime(targetInPerception.GetTimeSinceSeen());;
		};
		targetInPerception = m_PerceptionComponent.GetTargetPerceptionObject(m_target,ETargetCategory.UNKNOWN);
		if (targetInPerception)
		{
			SetVariableOut("LastPositionOut",targetInPerception.GetLastDetectedPosition());	
			return IsCurrentTime(targetInPerception.GetTimeSinceDetected());
		};
		targetInPerception = m_PerceptionComponent.GetTargetPerceptionObject(m_target,ETargetCategory.DETECTED);
		if (targetInPerception)
		{
			SetVariableOut("LastPositionOut",targetInPerception.GetLastDetectedPosition());	
			return IsCurrentTime(targetInPerception.GetTimeSinceDetected());
		}
		targetInPerception = m_PerceptionComponent.GetTargetPerceptionObject(m_target,ETargetCategory.FACTIONLESS);
		if (targetInPerception)
		{
			SetVariableOut("LastPositionOut",targetInPerception.GetLastSeenPosition());	
			return IsCurrentTime(targetInPerception.GetTimeSinceSeen());
		}
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------
	private bool IsCurrentTime (float oldInSeconds)
	{
		return (oldInSeconds < m_lastSeenMax);
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "SCR_AIIsTargetValid: Checks perception visibility of input target entity";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"TargetEntityIn"
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		"LastPositionOut"
	};
	protected override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};
