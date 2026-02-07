//------------------------------------------------------------------------------------------------
// BASIC REACTIONS
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIReactionBase
{
	// TODO: Behavior overriding will break the behavior, it's forbidden for now
	//[Attribute(defvalue: "", uiwidget: UIWidgets.stringPicker, desc: "BT assigned to behavior", params: "bt")]
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "(TEMP) Relavive path to a behavior tree with forward slashes \nExample: 'AI/BehaviorTrees/Chimera/Soldier/Wait.bt'", params: "bt")]
	string m_OverrideBehaviorTree;
	
	//200 - sorry for that, its defined in Soldier.bt in a node GetTarget, should be moved somewhere we can call Get()
	static const int AI_PERCEPTION_DISTANCE = 200;
	
	// will be defined on AI component hopefully later
	static const int AI_WEAPONFIRED_REACTION_DISTANCE = 500;

	void PerformReaction(notnull SCR_AIUtilityComponent utility) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility) {}
};

//------------------------------------------------------------------------------------------------
// GENERIC REACTIONS
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIWaitReaction : SCR_AIReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility) 
	{
		auto behavior = new SCR_AIWaitBehavior(utility, false);
		//if (behavior.m_sBehaviorTree != string.Empty)
			//behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIIdleReaction : SCR_AIReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility) 
	{
		auto behavior = new SCR_AIIdleBehavior(utility, false);
		//if (behavior.m_sBehaviorTree != string.Empty)
			//behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility) 
	{
		auto activity = new SCR_AIIdleActivity(utility, false, false);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIReturnToDefaultReaction : SCR_AIReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility) 
	{
		auto behavior = new SCR_AIReturnToDefaultBehavior(utility, false);
		utility.AddAction(behavior);
	}
};