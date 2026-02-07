[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowHint : SCR_ScenarioFrameworkActionBase
{
	[Attribute()]
	string		m_sTitle;

	[Attribute()]
	string		m_sText;

	[Attribute(defvalue: "15")]
	int			m_iTimeout;

	[Attribute()]
	FactionKey m_sFactionKey;

	[Attribute(desc: "Getter to get either a specific player or array of player entities")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerID = -1;
		if (EntityUtils.IsPlayer(object))
			playerID = playerManager.GetPlayerIdFromControlledEntity(object);

		array<IEntity> aEntities;

		if (m_Getter)
		{
			// Getter takes the priority. We set it back to -1 in case that object was player.
			playerID = -1;

			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!arrayOfEntitiesWrapper)
					return;

				aEntities = arrayOfEntitiesWrapper.GetValue();
				if (!aEntities)
					return;
			}
			else
			{
				IEntity entityFrom = entityWrapper.GetValue();
				if (entityFrom)
					playerID = playerManager.GetPlayerIdFromControlledEntity(entityFrom);
			}
		}

		if (!aEntities)
		{
			manager.ShowHint(m_sText, m_sTitle, m_iTimeout, m_sFactionKey, playerID);
		}
		else
		{
			foreach (IEntity entity : aEntities)
			{
				if (!EntityUtils.IsPlayer(entity))
					continue;

				playerID = playerManager.GetPlayerIdFromControlledEntity(entity);
				manager.ShowHint(m_sText, m_sTitle, m_iTimeout, m_sFactionKey, playerID);
			}
		}
	}
}