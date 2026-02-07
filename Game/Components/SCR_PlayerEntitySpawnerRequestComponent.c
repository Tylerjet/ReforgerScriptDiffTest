//------------------------------------------------------------------------------------------------
class SCR_PlayerEntitySpawnerRequestComponentClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Used for handling entity spawning requests for SCR_EntitySpawnerComponent and inherited classes, attached to SCR_PlayerController
class SCR_PlayerEntitySpawnerRequestComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	//! Entity spawn request from SCR_SpawnEntityUserAction
	//! \param index item index in User Action 
	//! \param spawnerComponent SpawnerComponent on which should be entity spawned 
	//! \param User requesting spawn
	void RequestEntitySpawn(int index, SCR_EntitySpawnerComponent spawnerComponent, IEntity user)
	{
		int userId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		
		IEntity spawnerOwnerEntity = spawnerComponent.GetOwner();
		if (!spawnerOwnerEntity)
			return;
		
		RplComponent spawnerRplComp = RplComponent.Cast(spawnerOwnerEntity.FindComponent(RplComponent));
		if (!spawnerRplComp)
			return;
		
		Rpc(RpcDo_RequestSpawn, spawnerRplComp.Id(), index, userId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Performs vehicle request on server
	//! \param rplCompId RplComp id of entity with spawner component
	//! \param index item index in User Action 
	//! \param userId id of user requesting spawn
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_RequestSpawn(RplId rplCompId, int index, int userId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;
		
		SCR_EntitySpawnerComponent entitySpawnerComp = SCR_EntitySpawnerComponent.Cast(rplComp.GetEntity().FindComponent(SCR_EntitySpawnerComponent));
		if(!entitySpawnerComp)
			return;
		
		entitySpawnerComp.InitiateSpawn(index, userId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send notification to player
	void SendPlayerFeedback(int msgID, int assetId)
	{
		Rpc(RpcDo_PlayerFeedbackImpl, msgID, assetId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackImpl(int msgID, int assetID)
	{
		string msg;
		string msg2;
		switch (msgID)
		{
			case SCR_EntityRequestDeniedReason.NOT_ENOUGH_SPACE:
			{
				msg = "#AR-Campaign_DeliveryPointObstructed-UC";
				break;
			}
			
			case SCR_EntityRequestDeniedReason.CAN_SPAWN:
			{
				SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
				if (!player)
					return;
				
				SCR_Faction faction = SCR_Faction.Cast(player.GetLocalControlledEntityFaction());
				if (!faction)
					return;
				
				SCR_EntityAssetList assetList = faction.GetVehicleList();
				if (!assetList)
					return;
				
				SCR_EntityInfo asset = assetList.GetEntryAtIndex(assetID);
				if (!asset)
					return;
				
				msg = "#AR-Campaign_VehicleReady-UC";
				msg2 = asset.GetDisplayName();
				break;
			}
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg(msg, 2, msg2);
	}
	
	//------------------------------------------------------------------------------------------------		
	override void EOnInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}
}