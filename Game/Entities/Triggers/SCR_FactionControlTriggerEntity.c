[EntityEditorProps(category: "GameScripted/Triggers", description: "")]
class SCR_FactionControlTriggerEntityClass: SCR_BaseFactionTriggerEntityClass
{
};
class SCR_FactionControlTriggerEntity: SCR_BaseFactionTriggerEntity
{
	[Attribute("1", UIWidgets.ComboBox, "How should friendly ratio limit be evaluated.", category: "Faction Control Trigger", enums: { ParamEnum("More than", "0"), ParamEnum("Equals to", "1"), ParamEnum("Less than", "2") })]
	protected int m_iRatioMethod;
	
	[Attribute("0.5", UIWidgets.Slider, "Limit for how large portion of trigger entities are friendly.\n\nExamples:\n1 = Only friendlies are present\n0 = Only enemies are present\n0.5 = Equal number of friendlies and enemies\n\nEvaluated only when at least some friendlies or enemies are inside.\nWhen the trigger is empty, condition for ratio = 0 won't activate the trigger.", category: "Faction Control Trigger", params: "0 1 0.1")]
	protected float m_fFriendlyRatioLimit;
	
	protected int m_iFriendlyCount, m_iEnemyCount;
	
	/*!
	Get number of entities inside trigger for each side
	\param[out] outFriendlyCount Number of friendlies
	\param[out] outEnemyCount Number of enemies
	*/
	void GetSideCounts(out int outFriendlyCount, out int outEnemyCount)
	{
		outFriendlyCount = m_iFriendlyCount;
		outEnemyCount = m_iEnemyCount;
	}
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		//--- No faction defined
		if (!m_OwnerFaction)
		{
			m_iFriendlyCount = 0;
			m_iEnemyCount = 0;
			return false;
		}
		
		//--- Querying itself - initiate new cycle
		if (ent == this)
		{
			int friendlyCount = m_iFriendlyCount;
			int enemyCount = m_iEnemyCount;
			m_iFriendlyCount = 0;
			m_iEnemyCount = 0;
			
			//--- Nobody is in the trigger, skip evaluation
			if (friendlyCount == 0 && enemyCount == 0)
				return false;
			
			float friendlyRatio = friendlyCount / Math.Max(friendlyCount + enemyCount, 1);
			//PrintFormat("%1: %2 / %3", m_OwnerFaction.GetFactionKey(), friendlyRatio, m_fFriendlyRatioLimit);
			switch (m_iRatioMethod)
			{
				case 0:
					return friendlyRatio > m_fFriendlyRatioLimit;
				case 1:
					return float.AlmostEqual(friendlyRatio, m_fFriendlyRatioLimit);
				case 2:
					return friendlyRatio < m_fFriendlyRatioLimit;
			}
		}
		
		//--- Evaluate engine-driven conditions, e.g., entity class
		if (!DefaultEntityFilterForQuery(ent) || !IsAlive(ent))
			return false;
		
		//--- Increase faction counters
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
		{
			Faction entFaction = factionAffiliation.GetAffiliatedFaction();
			if (entFaction)
			{
				if (m_OwnerFaction.IsFactionEnemy(entFaction))
					m_iEnemyCount++;
				else
					m_iFriendlyCount++;
			}
		}
		
		return false;
	}
};
