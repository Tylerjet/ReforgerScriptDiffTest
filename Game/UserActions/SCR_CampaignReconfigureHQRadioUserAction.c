//! Action to reconfigure relays in Campaign
class SCR_CampaignReconfigureHQRadioUserAction : ScriptedUserAction
{
	protected SCR_CampaignBase m_Base;
	protected IEntity m_HQRadio;
	
	static const float MAX_BASE_DISTANCE = 50;
	
	//------------------------------------------------------------------------------------------------
	protected bool IsParentBase(IEntity ent)
	{
		m_Base = SCR_CampaignBase.Cast(ent);

		// Base was found, stop query
		if (m_Base)
		{
			m_Base.RegisterHQRadio(m_HQRadio);
			return false;
		}
		
		// Keep looking
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckParentBase(vector origin)
	{
		if (!m_Base)
			Print("HQ radio at " + string.ToString(origin) + " is not close enough (" + MAX_BASE_DISTANCE + "m) to any Conflict base!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	static void ToggleBaseCaptured(SCR_CampaignBase base, bool isBeingCaptured)
	{
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		
		if (!playerController)
			return;
		
		// Find campaign network component to send RPC to server
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

		if (campaignNetworkComponent)
			campaignNetworkComponent.ToggleBaseCapture(base, isBeingCaptured);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (!pOwnerEntity)
			return;
		
		m_HQRadio = pOwnerEntity;
		
		if (SCR_GameModeCampaignMP.NotPlaying())
			return;
		
		BaseWorld world = GetGame().GetWorld();
		
		if (!world)
			return;
		
		vector origin = pOwnerEntity.GetOrigin();
		
		// Register parent base - find one in the nearest vicinity
		world.QueryEntitiesBySphere(origin, MAX_BASE_DISTANCE, IsParentBase, null, EQueryEntitiesFlags.ALL);
		
		// Throw an error if no base has been found in the vicinity
		GetGame().GetCallqueue().CallLater(CheckParentBase, 1000, false, origin);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity) == 0;
		
		if (!isAI && m_Base)
			ToggleBaseCaptured(m_Base, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ToggleBaseCaptured(m_Base, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{	
		if (!m_Base)
			return;	
		
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity) == 0;
		
		if (isAI)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(pUserEntity.FindComponent(FactionAffiliationComponent));
			
			if (!comp)
				return;
			
			if (m_Base.BeginCapture(SCR_CampaignFaction.Cast(comp.GetAffiliatedFaction())))
				m_Base.FinishCapture();
			
			return;
		}
		else
		{
			PlayerController playerController = GetGame().GetPlayerController();
			
			if (!playerController)
				return;
			
			SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
	
			if (campaignNetworkComponent)
				campaignNetworkComponent.CaptureBase(m_Base);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		bool isAI = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user) == 0;
		
		if (isAI)
		{
			FactionAffiliationComponent comp = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			
			if (!comp)
				return false;
			
			return (comp.GetAffiliatedFaction() != m_Base.GetOwningFaction());
		}
		else
		{
			SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_RespawnSystemComponent.GetLocalPlayerFaction());
			
			if (!playerFaction)
				return false;
			
			// Already ours
			if (m_Base.GetOwningFaction() == playerFaction)
			{
				SetCannotPerformReason("#AR-Campaign_Action_Done-UC");
				return false;
			}
			
			// No radio signal
			if (m_Base != playerFaction.GetMainBase() && !m_Base.IsBaseInFactionRadioSignal(playerFaction))
			{
				SetCannotPerformReason("#AR-Campaign_Action_NoSignal-UC");
				return false;
			}
			
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		
		if (!comp)
			return false;
		
		if (comp.IsDead())
			return false;
		
		return (m_Base != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
};
