[EntityEditorProps(category: "GameScripted/Campaign", description: "Feeds child SCR_Position entities into SCR_CampaignSpawnPointGroup of SCR_CampaignMilitaryBaseComponent, so players can spawn also on these instead of typical spawn position.", color: "0 0 255 255")]
class SCR_CampaignBunkerComponentClass : SCR_MilitaryBaseLogicComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignBunkerComponent : SCR_MilitaryBaseLogicComponent
{
	protected ref array<SCR_Position> m_aChildrenPositions = {};
	protected ref array<SCR_CampaignSpawnPointGroup> m_aSpawnPoints = {};

	//------------------------------------------------------------------------------------------------
	//! Finds all SCR_Positions in hiarchy and registers them to component
	protected void FindChildrenPositionsInHiearchy()
	{
		SCR_Position position;

		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			position = SCR_Position.Cast(child);
			if (position && !m_aChildrenPositions.Contains(position))
				m_aChildrenPositions.Insert(position);

			child = child.GetSibling();
		}

		ApplyChildrenToSpawnPoints();
	}

	//------------------------------------------------------------------------------------------------
	//! Called after Children are already found, will add child positions to known SpawnPositions
	protected void ApplyChildrenToSpawnPoints()
	{
		foreach (SCR_CampaignSpawnPointGroup spawnPoint : m_aSpawnPoints)
		{
			AddSpawnPositions(spawnPoint);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add child SCR_Positions into Spawnpoint
	// \param spawnPoint SpawnPoint to which should children be added or removed from
	protected void AddSpawnPositions(notnull SCR_CampaignSpawnPointGroup spawnPoint)
	{
		foreach (SCR_Position position : m_aChildrenPositions)
		{
			spawnPoint.InsertChildrenPosition(position);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove child SCR_Positions into Spawnpoint
	// \param spawnPoint SpawnPoint to which should children be added or removed from
	protected void RemoveSpawnPositions(notnull SCR_CampaignSpawnPointGroup spawnPoint)
	{
		foreach (SCR_Position position : m_aChildrenPositions)
		{
			spawnPoint.RemoveChildrenPosition(position);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseRegistered(base);

		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (!campaignBase)
			return;

		SCR_CampaignSpawnPointGroup spawnPoint = SCR_CampaignSpawnPointGroup.Cast(campaignBase.GetSpawnPoint());
		if (!spawnPoint && m_aSpawnPoints.Contains(spawnPoint))
			return;

		m_aSpawnPoints.Insert(spawnPoint);
		
		if (!m_aChildrenPositions.IsEmpty())
			ApplyChildrenToSpawnPoints();
	}

	//------------------------------------------------------------------------------------------------
	override void OnBaseUnregistered(notnull SCR_MilitaryBaseComponent base)
	{
		super.OnBaseUnregistered(base);

		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
		if (!campaignBase)
			return;

		SCR_CampaignSpawnPointGroup spawnPoint = SCR_CampaignSpawnPointGroup.Cast(campaignBase.GetSpawnPoint());
		if (spawnPoint && m_aSpawnPoints.Contains(spawnPoint))
		{
			RemoveSpawnPositions(spawnPoint);
			m_aSpawnPoints.RemoveItem(spawnPoint);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(owner.FindComponent(SCR_EditorLinkComponent));
		if (linkComponent)
			linkComponent.GetOnLinkedEntitiesSpawned().Insert(FindChildrenPositionsInHiearchy);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignBunkerComponent()
	{
		foreach (SCR_CampaignSpawnPointGroup spawnPoint : m_aSpawnPoints)
		{
			if (spawnPoint)
				spawnPoint.RemoveEmptyChildren();
		}
	}
};
