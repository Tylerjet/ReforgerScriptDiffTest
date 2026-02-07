[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArrayOfPlayers : SCR_ScenarioFrameworkGet
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "(Optional) You can filter players by putting the Faction Key of desired faction(s). If not used, all players will be fetched.")]
	ref array<string> m_aFactionKeys;
	
	ref array<IEntity> m_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		m_aEntities.Clear();
		GetPlayerEntities(m_aEntities);

		return new SCR_ScenarioFrameworkParam<array<IEntity>>(m_aEntities);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] aEntities
	void GetPlayerEntities(out notnull array<IEntity> aEntities)
	{
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		
		IEntity playerEntity;
		SCR_ChimeraCharacter chimeraCharacter;
		bool factionKeyIsEmpty = m_aFactionKeys.IsEmpty();
		foreach (int playerID : playerIDs)
		{
			playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!playerEntity)
				continue;
			
			if (!factionKeyIsEmpty)
			{
				chimeraCharacter = SCR_ChimeraCharacter.Cast(playerEntity);
				if (!chimeraCharacter || !m_aFactionKeys.Contains(chimeraCharacter.GetFactionKey()))
					continue;
			}

			aEntities.Insert(playerEntity);
		}
	}
}