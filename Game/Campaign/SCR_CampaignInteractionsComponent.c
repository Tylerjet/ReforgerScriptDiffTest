[EntityEditorProps(category: "GameScripted/Campaign", description: "Sets the actions available to a particular Campaign AI unit", color: "0 0 255 255")]
class SCR_CampaignInteractionsComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignInteractionsComponent : ScriptComponent
{
	[Attribute(defvalue: "true", desc: "Can this entity seize bases?", UIWidgets.CheckBox)]
	bool m_bCanSeize;
	
	[Attribute(defvalue: "false", desc: "Can this entity defend bases?", UIWidgets.CheckBox)]
	bool m_bCanDefend;
	
	[Attribute(defvalue: "true", desc: "Can this entity defend bases, but only after a delay since spawn?", UIWidgets.CheckBox)]
	protected bool m_bCanDefendDelayed;
	
	/*[Attribute(defvalue: "true", desc: "Can this entity discover relays?", UIWidgets.CheckBox)]
	bool m_bCanDetect;*/
	
	/*[Attribute(defvalue: "100", desc: "Radius in which relays are detected. (in m)")]
	protected float m_fRadius;*/
	
	/*[Attribute(defvalue: "2", desc: "How often should we detect relays. (in s)")]
	protected float m_fDetectionTick;*/
	
	//TODO REFACTOR TO USE FLAGS NOT BOOLS
	//[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_ECampaignPresentFactionsFlags))]
	//protected SCR_ECampaignPresentFactionsFlags m_eCampaignBaseInteractionFlags;
	
	//protected static ref array<SCR_CampaignBase> s_aRelaysToIterate = new array<SCR_CampaignBase>();
	
	protected RplComponent m_RplComponent;
	//protected float m_fCurrentDetectionTickTime;
	//protected float m_fRadiusSquared;
	protected static bool s_bClearRelayArray = true;
	
	protected static ref array<SCR_CampaignBase> s_aRelays = new ref array<SCR_CampaignBase>();
	protected static SCR_CampaignBaseManager s_BaseManager = null;
	
	//------------------------------------------------------------------------------------------------
	//! Checks if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for the ability to seize bases
	bool CanSeize()
	{
		return m_bCanSeize;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for the ability to defend bases
	bool CanDefend()
	{
		return m_bCanDefend;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for the ability to detect relays
	/*bool CanDetectRelays()
	{
		return m_bCanDetect;
	}*/
	
	//------------------------------------------------------------------------------------------------
	//! Setter for the ability to defend bases
	void SetCanDefend(bool status)
	{
		m_bCanDefend = status;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setter for the ability to seize bases
	void SetCanSeize(bool status)
	{
		m_bCanSeize = status;
	}
	
	//------------------------------------------------------------------------------------------------
	/*protected void OnCharacterDeath()
	{
		SCR_GameModeCampaignMP gameModeCampaign = SCR_GameModeCampaignMP.GetInstance();
		if (gameModeCampaign)
			gameModeCampaign.InteractionComponentsUnregister(this);
	}*/
	
	//------------------------------------------------------------------------------------------------
	//! Goes through all the registered relays and checks the distance for detection
	/*void IterateRelays()
	{
		if (!m_bCanDetect)
			return;
		
		if (!GetOwner() || !s_BaseManager)
			return;
		
		vector myOrigin = GetOwner().GetOrigin();
		s_BaseManager.GetFilteredBases(CampaignBaseType.RELAY, s_aRelaysToIterate);
		for (int i = 0, count = s_aRelaysToIterate.Count(); i < count; i++)
		{
			
			// Check distance from each relay
			float distance = vector.DistanceSq(s_aRelaysToIterate.Get(i).GetOrigin(), myOrigin);
			
			// Am I close enough?
			if (distance <= m_fRadiusSquared)
			{
				// Yes -> Detect the base
				s_aRelaysToIterate.Get(i).DetectBase(SCR_RespawnSystemComponent.GetLocalPlayerFaction(GetOwner()), GetOwner());
			}
		}
	}*/
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		//characterController.m_OnPlayerDeath.Insert(OnCharacterDeath);
		
		//m_fCurrentDetectionTickTime = m_fDetectionTick;
		
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//m_fRadiusSquared = m_fRadius * m_fRadius;
		if (!s_BaseManager)
			s_BaseManager = SCR_CampaignBaseManager.GetInstance();
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignInteractionsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		/*SCR_GameModeCampaignMP gameModeCampaign = SCR_GameModeCampaignMP.GetInstance();
		if (gameModeCampaign)
			gameModeCampaign.InteractionComponentsRegister(this);*/
		
		
		if (!IsProxy())
		{
			// Enable players to defend bases some time after respawning
			if (m_bCanDefendDelayed && !m_bCanDefend)
				GetGame().GetCallqueue().CallLater(SetCanDefend, 60000, false, true);
			
			// Enable players to seize bases some time after respawning (failsafe)
			if (m_bCanSeize)
			{
				SetCanSeize(false);
				GetGame().GetCallqueue().CallLater(SetCanSeize, 2000, false, true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignInteractionsComponent()
	{
		
	}
};
