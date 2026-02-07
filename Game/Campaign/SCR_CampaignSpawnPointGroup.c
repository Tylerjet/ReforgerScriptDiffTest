[EntityEditorProps(category: "GameScripted/GameMode", description: "Campaign Spawn point group entity", visible: false)]
class SCR_CampaignSpawnPointGroupClass : SCR_SpawnPointClass
{
};

//------------------------------------------------------------------------------------------------
//! Asside of normal spawnpoint functionality, this spawnpoint allows usage of default SpawnPoint if child SCR_Positions are currently occupied by players
class SCR_CampaignSpawnPointGroup : SCR_SpawnPoint
{
	[Attribute("2", desc: "Children spawn radius range.", category: "Spawnpoint group")]
	protected float m_fChildrenRadius;

	//------------------------------------------------------------------------------------------------
	//! Goes through m_aChildren array and removes unexisting ones
	void RemoveEmptyChildren()
	{
		for (int i = m_aChildren.Count() - 1; i >= 0; i--)
		{
			if (!m_aChildren[i])
				m_aChildren.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Allows insertion of Children position in runtime
	// \param position Position to be added as child 
	void InsertChildrenPosition(notnull SCR_Position position)
	{
		if (!m_aChildren.Contains(position))
			m_aChildren.Insert(position);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Allows removal of Children position in runtime
	// \param position Position to be added as child 
	void RemoveChildrenPosition(notnull SCR_Position position)
	{
		m_aChildren.RemoveItem(position);
	}

	//------------------------------------------------------------------------------------------------
	//! callback for IsChildEmpty query. Returns false, if position is empty
	protected bool IsChildrenPositionEmptyCallback(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return false;

		SCR_DamageManagerComponent damageManager = character.GetDamageManager();
		if (!damageManager || !damageManager.IsDestroyed())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsChildEmpty(SCR_Position position)
	{
		if (!GetGame().GetWorld().QueryEntitiesBySphere(position.GetOrigin(), m_fChildrenRadius, IsChildrenPositionEmptyCallback, null, EQueryEntitiesFlags.ALL))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_Position GetEmptyChildrenPosition()
	{
		if (m_aChildren.IsEmpty())
			return null;

		int count = m_aChildren.Count();
		int startIndex = Math.RandomInt(0, count-1);
		int currentIndex;

		for (int i = 0; i < count; i++)
		{
			if (startIndex + i < count)
				currentIndex = startIndex + 1;
			else
				currentIndex = count - i;

			if (IsChildEmpty(m_aChildren[currentIndex]))
				return m_aChildren[currentIndex];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPositionAndRotation(out vector pos, out vector rot)
	{
		SCR_Position position = GetEmptyChildrenPosition();

		if (position)
		{
			pos = position.GetOrigin();
			rot = position.GetAngles();
		}
		else
		{
			SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOrigin(), GetSpawnRadius());
			rot = GetAngles();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetSpawnPointName()
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(GetParent().FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (base)
			return base.GetBaseName();

		return "BASED";
	}
};
