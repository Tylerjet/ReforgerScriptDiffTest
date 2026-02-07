class SCR_AIActionTask : AITaskScripted
{
	// This is parent class not to be used / visible in palette of AI nodes
	
	static const string ENTITY_PORT = "Entity";
	static const string POSITION_PORT = "Position";
	static const string STANCE_PORT = "Stance";
	static const string ROLEINVEHICLE_PORT = "RoleInVehicle";
	static const string REPORTER_PORT = "ReportingAgent";
	static const string NOVEHICLES_PORT = "noVehicles";
	static const string DESIREDDISTANCE_PORT = "DesiredDistance";
	static const string TARGET_PORT = "Target";
	static const string TARGETPOSITION_PORT = "TargetPosition";
	static const string NEXTCOVERPOSITION_PORT = "NextCoverPosition";
	static const string SMARTACTION_PORT = "SmartActionComponent";
	static const string MAGAZINE_WELL_PORT = "MagazineWell";
	static const string PRIORITIZE_PORT = "Prioritize";	
	static const string IS_DANGEROUS_PORT = "isDangerous";
	static const string REINIT_PORT = "Reinit";
	static const string RADIUS_PORT = "Radius";
	
	protected SCR_AIActionBase m_Action;
	protected SCR_AIBaseUtilityComponent m_UtilityComp;

	//------------------------------------------------------------------------------------------------
	bool IsActionValid()
	{
		if (!m_UtilityComp)
			return false;
		
		m_Action = m_UtilityComp.m_ExecutedAction;
		if (!m_Action)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_UtilityComp = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
		if (!m_UtilityComp)
		{
			NodeError(this, owner, "Can't find base utility component.");
		}
	}
	
};