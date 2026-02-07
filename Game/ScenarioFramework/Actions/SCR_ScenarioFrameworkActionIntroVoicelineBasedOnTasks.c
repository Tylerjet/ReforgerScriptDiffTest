[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionIntroVoicelineBasedOnTasks : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Sound to play.")]
	string m_sSound;

	[Attribute(desc: "(Optional) If getter is provided, sound will come from the provided entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	ref array<int> m_aAffectedPlayers = {};

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerID = playerManager.GetPlayerIdFromControlledEntity(object);
		if (m_aAffectedPlayers.Contains(playerID))
			return;

		m_aAffectedPlayers.Insert(playerID);

		SCR_GameModeSFManager manager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!manager)
			return;

		EntityID entityID;
		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (entityWrapper)
			{
				IEntity entity = entityWrapper.GetValue();
				if (entity)
					entityID = entity.GetID();
			}
		}

		manager.PlayIntroVoiceline(playerID, m_sSound, entityID);
	}
}