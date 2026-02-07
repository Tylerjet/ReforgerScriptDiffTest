[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ArsenalManagerComponentClass: SCR_BaseGameModeComponentClass
{
};
class SCR_ArsenalManagerComponent : SCR_BaseGameModeComponent
{
	//=== Authority
	protected ref map<int, string> m_aPlayerLoadouts = new map<int, string>();
	
	//=== Broadcast
	protected ref ScriptInvoker Event_OnPlayerLoadoutUpdated = new ScriptInvoker;
	
	protected bool m_bLocalPlayerLoadoutAvailable;
	
	static bool GetArsenalManager(out SCR_ArsenalManagerComponent arsenalManager)
	{
		arsenalManager = SCR_ArsenalManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_ArsenalManagerComponent));
		return arsenalManager != null;
	}
	
	bool GetLocalPlayerLoadoutAvailable()
	{
		return m_bLocalPlayerLoadoutAvailable;
	}
	
	ScriptInvoker GetOnLoadoutUpdated()
	{
		return Event_OnPlayerLoadoutUpdated;
	}
	
	//=== Authority
	bool GetPlayerArsenalLoadout(int playerId, out string jsonCharacter)
	{
		return m_aPlayerLoadouts.Find(playerId, jsonCharacter) && jsonCharacter != string.Empty;
	}
	
	//=== Authority
	void SetPlayerArsenalLoadout(int playerId, GameEntity characterEntity)
	{
		if (playerId == 0)
		{
			return;
		}
		if (!characterEntity)
		{
			DoSetPlayerLoadout(playerId, string.Empty);
			return;
		}
		
		SCR_PlayerController clientPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!clientPlayerController || clientPlayerController.IsPossessing())
		{
			return;
		}
		
		string factionKey = SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTIONKEY_NONE;
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
		{
			factionKey = factionAffiliation.GetAffiliatedFaction().GetFactionKey();
		}
		
		SCR_JsonSaveContext context = new SCR_JsonSaveContext();
		if (!context.WriteStrValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTION_KEY, factionKey) || !context.WriteGameEntity(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_KEY, characterEntity))
		{
			return;
		}
		
		DoSetPlayerLoadout(playerId, context.ExportToString());
	}
	
	protected void DoSetPlayerLoadout(int playerId, string loadoutString)
	{
		bool loadoutValid = !loadoutString.IsEmpty();
		bool loadoutChanged = loadoutValid && loadoutString != m_aPlayerLoadouts.Get(playerId);
		
		m_aPlayerLoadouts.Set(playerId, loadoutString);
		
		DoSetPlayerHasLoadout(playerId, loadoutValid, loadoutChanged);
		Rpc(DoSetPlayerHasLoadout, playerId, loadoutValid, loadoutChanged);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSetPlayerHasLoadout(int playerId, bool loadoutValid, bool loadoutChanged)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			if (m_bLocalPlayerLoadoutAvailable != loadoutValid || loadoutChanged)
			{
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_SAVED);
			}
			m_bLocalPlayerLoadoutAvailable = loadoutValid;
		}
		Event_OnPlayerLoadoutUpdated.Invoke(playerId, loadoutValid);
	}
};