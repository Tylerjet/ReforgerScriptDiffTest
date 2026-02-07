class SCR_AILookAction // : SCR_AIActionBase
{
	SCR_AIUtilityComponent m_Utility;
	vector m_vPosition;
	bool m_bCanLook;
	bool m_bValidLookPosition;
	bool m_bResetLook;
	
	static const float PRIO_ENEMY_TARGET = 80;
	static const float PRIO_UNKNOWN_TARGET = 50;
	static const float PRIO_DANGER_EVENT = 20;
	
	protected float m_fPriority;
	
	void SCR_AILookAction(SCR_AIBaseUtilityComponent utility, bool prioritize)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(utility);
	}
	
	void LookAt(vector pos, float priority)
	{
		if (priority < m_fPriority)
			return;
		
		m_vPosition = pos;
		m_fPriority = priority;
		m_bValidLookPosition = true;
	}
	
	void LookAt(IEntity ent, float priority)
	{
		if (priority <= m_fPriority && ent)
			return;

		vector min, max;
		ent.GetBounds(min, max);
		m_vPosition = ent.GetOrigin() + (min + max) * 0.5;
		m_fPriority = priority;
		m_bValidLookPosition = true;
	}

	float Evaluate()
	{
		m_bCanLook = m_bValidLookPosition && m_Utility.m_CurrentBehavior.m_bAllowLook;
		return m_fPriority;
	}

	void Cancel()
	{
		Complete();
		m_bResetLook = true;
	}
	
	void Complete()
	{
		m_bValidLookPosition = false;
		m_bCanLook = false;
		m_vPosition = vector.Zero;
		m_fPriority = 0;
	}
};