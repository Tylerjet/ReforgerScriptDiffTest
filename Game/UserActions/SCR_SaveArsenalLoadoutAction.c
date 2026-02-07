//------------------------------------------------------------------------------------------------
class SCR_SaveArsenalLoadout : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override protected bool CanBeShownScript(IEntity user)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		SCR_ArsenalManagerComponent arsenalManager;
		return (!playerController || !playerController.IsPossessing()) && SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (playerId != 0 && SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			arsenalManager.SetPlayerArsenalLoadout(playerId, GameEntity.Cast(pUserEntity));
		}
	}
};