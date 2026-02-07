class SCR_EnableDefendersAction : ScriptedUserAction
{

	protected SCR_DefenderSpawnerComponent m_DefenderSpawner;
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_DefenderSpawner)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_SpawnerRequestComponent playerReqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (!playerReqComponent)
			return;

		playerReqComponent.EnableSpawning(m_DefenderSpawner, !m_DefenderSpawner.IsSpawningEnabled(), playerController.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_DefenderSpawner)
			return false;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return false;

		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != m_DefenderSpawner.GetCurrentFaction())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (m_DefenderSpawner.IsSpawningEnabled())
			outName = "#AR-DefenderSpawner_DisableSpawning";
		else
			outName = "#AR-DefenderSpawner_EnableSpawning";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_DefenderSpawner = SCR_DefenderSpawnerComponent.Cast(pOwnerEntity.FindComponent(SCR_DefenderSpawnerComponent));
	}
};
