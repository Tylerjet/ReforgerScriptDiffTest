[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_ResupplySupportStationComponentClass : SCR_BaseItemSupportStationComponentClass
{
}

class SCR_ResupplySupportStationComponent : SCR_BaseItemSupportStationComponent
{
	//------------------------------------------------------------------------------------------------
	protected override bool InitValidSetup()
	{		
		//~ Resupply ammo does not support range as only players can resupply themselves or others at the moment
		if (UsesRange())
		{
			Print("'SCR_ResupplySupportStationComponent' does not support range. Make sure m_fRange is set to -1", LogLevel.ERROR);
			return false;
		}
		
		return super.InitValidSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.RESUPPLY_AMMO;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyAmount)
	{
		if (!SCR_BaseResupplySupportStationAction.Cast(action))
		{
			Debug.Error2("SCR_ResupplySupportStationComponent", "'IsValid' fails as the resupply support station is executed with a non SCR_BaseResupplySupportStationAction action! This is will break the support station!");
			return false;
		}
		
		return super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_InventoryStorageManagerComponent inventoryManager = resupplyAction.GetTargetInventory();
		if (!inventoryManager)
			return;
		
		//~ Consume supplies
		if (AreSuppliesEnabled())
		{
			if (!OnConsumeSuppliesServer(GetSupplyAmountAction(actionOwner, actionUser, action)))
				return;
		}
		
		map<ResourceName, int> itemsToResupply = new map<ResourceName, int>;
		itemsToResupply.Insert(resupplyAction.GetItemPrefab(), 1);
		inventoryManager.ResupplyMagazines(itemsToResupply);
		
		super.OnExecutedServer(actionOwner, actionUser, action);
	}	
	
	//------------------------------------------------------------------------------------------------
	//~ Called by OnExecuteBroadcast and is executed both on server and on client
	//~ playerId can be -1 if the user was not a player
	protected override void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{
		//~ On succesfully executed
		OnSuccessfullyExecuted(actionOwner, actionUser, action);
		
		if (!actionUser || !actionOwner)
			return;
		
		SCR_BaseResupplySupportStationAction resupplyAction = SCR_BaseResupplySupportStationAction.Cast(action);
		
		SCR_InventoryStorageManagerComponent targetInventory = resupplyAction.GetTargetInventory();
		if (!targetInventory)
			return;
		
		//~ Player resupplied themselves
		if (targetInventory.GetOwner() == actionUser)
		{
			//~  Play sound on use
			PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
		
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionOwner);
			
			if (GetSendNotificationOnUse() && resupplyAction.GetNotificationOnUse() != ENotification.UNKNOWN)
			{
				//~ ID is not the local player so do not send notification
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(actionUser);
				if (SCR_PlayerController.GetLocalPlayerId() == playerId)
					SCR_NotificationsComponent.SendLocal(resupplyAction.GetNotificationOnUse());
			}
		}
		//~ Player resupplied other	
		else 
		{			
			//~  Play sound on use
			PlaySoundEffect(GetOnUseAudioConfig(), actionUser, action);
		
			//~ Play full done voice event (if any)
			PlayCharacterVoiceEvent(actionUser);
			
			if (GetSendNotificationOnUse() && resupplyAction.GetNotificationOnUse() != ENotification.UNKNOWN)
			{
				//~ ID is not the local player so do not send notification
				playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(actionOwner);
				if (SCR_PlayerController.GetLocalPlayerId() == playerId)
				{
					RplId ownerId;
					RplId userId;
					FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
					SCR_NotificationsComponent.SendLocal(resupplyAction.GetNotificationOnUse(), userId);
				}
			}
		}				
	}
}
