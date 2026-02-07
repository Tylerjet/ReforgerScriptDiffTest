[EntityEditorProps(category: "GameScripted/GameMode", description: "Spawn protection component for gamemode.", color: "0 0 255 255")]
class SCR_SpawnProtectionComponentClass: SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_SpawnProtectionComponent: SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "10", desc: "How long should be player protected?")]
	protected float m_fProtectionTime;
	
	[Attribute(defvalue: "1", desc: "Should be protection disabled if player shoots?")]
	protected bool m_bDisableOnAttack;
	
	[Attribute(defvalue: "0", desc: "Allow player spawnpoints?")]
	protected bool m_bAllowPlayerSpawnpoints;
	
	protected RplComponent m_RplComponent;
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{	
		if (IsProxy())
			return;
		
		//Check for radio respawns
		if (!m_bAllowPlayerSpawnpoints)
		{
			SCR_PlayerRadioSpawnPoint spawnPoint = SCR_PlayerRadioSpawnPoint.Cast(m_pGameMode.GetRespawnSystemComponent().GetPlayerSpawnPoint(playerId));
			if (spawnPoint)
				return;
		}
		
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageManager)
			return;
		
		damageManager.EnableDamageHandling(false);
		
		
		GetGame().GetCallqueue().CallLater(DisablePlayerProtection, m_fProtectionTime*1000, false, controlledEntity);
		
		// TODO: Check also for melee and grenade throw
		if (m_bDisableOnAttack)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(controlledEntity.FindComponent(EventHandlerManagerComponent));
			if (!eventHandlerManager)
				return;
			
			eventHandlerManager.RegisterScriptHandler("OnAmmoCountChanged", controlledEntity, OnPlayerAmmoChangeCallback);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback to be used by eventHandler. 
	protected void OnPlayerAmmoChangeCallback(BaseWeaponComponent currentWeapon, BaseMuzzleComponent currentMuzzle, BaseMagazineComponent magazine, int ammoCount, bool isChambered)
	{
		if (!currentWeapon.GetOwner())
			return;
		
		if (!currentWeapon.GetOwner().GetParent())
			return;
		
		IEntity parent = currentWeapon.GetOwner().GetParent();
		DisablePlayerProtection(parent);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	protected void DisablePlayerProtection(IEntity playerEntity)
	{
		if (!playerEntity)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (!GetGame().GetPlayerManager().GetPlayerController(playerID))
			return;
		
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(playerEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageManager)
			return;
		
		damageManager.EnableDamageHandling(true);
		
		if (m_bDisableOnAttack)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(playerEntity.FindComponent(EventHandlerManagerComponent));
			if (!eventHandlerManager)
				return;
			
			eventHandlerManager.RemoveScriptHandler("OnAmmoCountChanged", playerEntity, OnPlayerAmmoChangeCallback);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (IsProxy())
			return;
		
		if (!GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
	}
}