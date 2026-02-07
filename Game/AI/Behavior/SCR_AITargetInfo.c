class SCR_AITargetInfo
{
	IEntity m_TargetEntity;
	vector m_vLastSeenPosition;
	float m_fLastSeenTime;
	EFireTeams m_eFireTeamAssigned;
	
	//----------------------------------------------------------------------------------------------------------
	void SCR_AITargetInfo(IEntity targetEntity = null, vector lastSeenPosition = vector.Zero, float lastSeenTime = 0.0, EFireTeams assignedFireteam = EFireTeams.NONE)
	{
		m_TargetEntity = targetEntity;
		m_vLastSeenPosition = lastSeenPosition;
		m_fLastSeenTime = lastSeenTime;
		m_eFireTeamAssigned = assignedFireteam;	
	};
	
	//----------------------------------------------------------------------------------------------------------
	void CopyFrom(SCR_AITargetInfo other)
	{
		m_TargetEntity = other.m_TargetEntity;
		m_vLastSeenPosition = other.m_vLastSeenPosition;
		m_fLastSeenTime = other.m_fLastSeenTime;
		m_eFireTeamAssigned = other.m_eFireTeamAssigned;
	}
};