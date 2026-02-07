[ComponentEditorProps(category: "GameScripted/Editor", description: "Main campaign component for handling building editor mode", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingEditorComponentClass : SCR_BaseEditorComponentClass
{
};

//------------------------------------------------------------------------------------------------

class SCR_CampaignBuildingEditorComponent : SCR_BaseEditorComponent
{	
	protected ref array<IEntity> m_aProviderEntities = {};
	protected ref array<RplId> m_aProvidersRplIds = {};
	protected SCR_FreeCampaignBuildingTrigger m_Trigger;
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected IEntity m_ForcedProvider;
	protected int m_PlayerId;
	
	protected ref ScriptInvoker m_OnProviderSet;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnProviderSet()
	{
		if (!m_OnProviderSet)
			m_OnProviderSet = new ScriptInvoker();
		
		return m_OnProviderSet;
	}
	
	//------------------------------------------------------------------------------------------------
 	void AddProviderEntityEditorComponent(IEntity provider)
	{
		if (!provider)
			return;
		
		SCR_CampaignBase baseEnt = SCR_CampaignBase.Cast(provider);
		if (baseEnt)
			m_aProviderEntities.InsertAt(provider, 0);
		else
			m_aProviderEntities.Insert(provider);
			
		GetOnProviderSet().Invoke(m_aProviderEntities[0]);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveProviderEntityEditorComponent(IEntity provider)
	{		
		m_aProviderEntities.RemoveItemOrdered(provider);			
	}
			
	//------------------------------------------------------------------------------------------------
	IEntity GetProviderEntity()
	{
		if (m_ForcedProvider)
			return m_ForcedProvider;
		
		if (m_aProviderEntities.IsEmpty())
			return null;
		
		return m_aProviderEntities[0];
	}
	
	//------------------------------------------------------------------------------------------------
	// Used when player initiate a building mode via user action - forced provider is an entity owning the user action
	void SetForcedProvider(IEntity ent = null)
	{
		m_ForcedProvider = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetProviderEntitiesCount()
	{
		return m_aProviderEntities.Count();	
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetProviderSuppliesComponent(out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		if (m_aProviderEntities.IsEmpty())
			return false;
		
		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignSuppliesComponent));
		if (suppliesComponent)
			return true;

		IEntity parent = GetProviderEntity();
		while (parent)
		{
			suppliesComponent = SCR_CampaignSuppliesComponent.Cast(parent.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComponent)
				return true;
			
			parent = parent.GetParent();
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_FactionAffiliationComponent GetProviderFactionComponent()
	{
		if (m_aProviderEntities.IsEmpty())
			return null;
		
		SCR_FactionAffiliationComponent factionComp = SCR_FactionAffiliationComponent.Cast(GetProviderEntity().FindComponent(SCR_FactionAffiliationComponent));
		if (factionComp)
				return factionComp;

		IEntity parent = GetProviderEntity();
		while (parent)
		{
			factionComp = SCR_FactionAffiliationComponent.Cast(parent.FindComponent(SCR_FactionAffiliationComponent));
			if (factionComp)
				return factionComp;
			
			parent = parent.GetParent();
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTrigger()
	{
		if (!m_aProviderEntities.IsEmpty())
		{
			IEntity child = GetProviderEntity().GetChildren();
			while (child)
			{
				SCR_FreeCampaignBuildingTrigger trg = SCR_FreeCampaignBuildingTrigger.Cast(child);
				if (trg)
				{
					m_Trigger = trg;
					break;
				}
				
				child = child.GetSibling();
			};
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_FreeCampaignBuildingTrigger GetTrigger()
	{		
		return m_Trigger;
	}
	
	//------------------------------------------------------------------------------------------------
	//~  This function will add/remove the faction label of the saved editor state. Thus displaying the correct faction entities in the menu
	protected void AddRemoveFactionLabel(SCR_CampaignFaction faction, bool addLabel)
	{
		if (!faction)
			return;
		
		m_ContentBrowserManager.AddRemoveLabelOfPersistentBrowserState(faction.GetFactionLabel(), addLabel);
	}
	
	//! Get the trigger on server too as it's needed to get a temporary provider for supplies changes
	override protected void EOnEditorActivateServer()
	{
		SetTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Make the area around where is possibel to build composition visible for player
	override protected void EOnEditorActivate()
	{
		SetTrigger();
		
		if (m_Trigger)
			m_Trigger.SetFlags(EntityFlags.VISIBLE, false);
		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		
		FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
		if (factionComponent)
		{
			// here we have to check both Affiliated faction and Default affiliated faction in case of vehicles. It's because vehicles can't have set a affiliated faction if no one is sitting inside (to prevent AI to shoot at empty vehicles)
			Faction buildingFaction = factionComponent.GetAffiliatedFaction();
			
			if (!buildingFaction)
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();
			
			if (buildingFaction)
				AddRemoveFactionLabel(SCR_CampaignFaction.Cast(buildingFaction), true);
		}
		
		//~ Todo: Fix first tab being broken
		//~ Hotfix for first tab being broken
		m_ContentBrowserManager.SetStateTabVisible(0, false);
		
		//~ Hide services in base show if outside base. Make sure given index is 0 if above hotfix is removed
		m_ContentBrowserManager.SetStateTabVisible(1, SCR_CampaignBase.Cast(GetProviderEntity()) != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(GetProviderEntity());
		if (!base && m_Trigger)
			GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), m_Trigger.GetSphereRadius(), AssociateCompositionsToProvider, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorCloseServer()
	{
		SCR_CampaignBase base = SCR_CampaignBase.Cast(GetProviderEntity());
		if (!base && m_Trigger)
			GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), m_Trigger.GetSphereRadius(), UnassignCompositionProvider, null, EQueryEntitiesFlags.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	bool AssociateCompositionsToProvider(IEntity ent)
	{
		SCR_CampaignBuildingCompositionComponent comp = SCR_CampaignBuildingCompositionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!comp)
			return true;
		
		// compositioni has no owner, set one.
		if (comp.GetProviderEntity() == null)
			comp.SetProviderEntityServer(GetProviderEntity());
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool UnassignCompositionProvider(IEntity ent)
	{
		SCR_CampaignBuildingCompositionComponent comp = SCR_CampaignBuildingCompositionComponent.Cast(ent.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!comp)
			return true;
		
		if (comp.GetProviderEntity() == GetProviderEntity())
			comp.SetProviderEntityServer(null); 
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hide the area of building and remove the active faction
	override protected void EOnEditorDeactivate()
	{
		if (m_Trigger)
			m_Trigger.ClearFlags(EntityFlags.VISIBLE, false);
		
		FactionAffiliationComponent factionComponent = GetProviderFactionComponent();
		if (factionComponent)
		{
			// here we have to check both Affiliated faction and Default affiliated faction in case of vehicles. It's because vehicles can't have set a affiliated faction if no one is sitting inside (to prevent AI to shoot at empty vehicles)
			Faction buildingFaction = factionComponent.GetAffiliatedFaction();
			
			if (!buildingFaction)
				buildingFaction = factionComponent.GetDefaultAffiliatedFaction();
			
			if (buildingFaction)
				AddRemoveFactionLabel(SCR_CampaignFaction.Cast(buildingFaction), false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set a provider, loaded from RPL ID
	void SetProviderFromRplID()
	{
		int count = m_aProvidersRplIds.Count();
		for (int i = 0; i < count; i++)
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_aProvidersRplIds[i]));
			if (!rplComp)
				continue;
			
			AddProviderEntityEditorComponent(rplComp.GetEntity());
			m_aProvidersRplIds.RemoveOrdered(i);
			count--;
		}
		
		if (m_aProvidersRplIds.IsEmpty())
			SCR_FreeCampaignBuildingTrigger.GetOnProviderCreated().Remove(SetProviderFromRplID);	
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		RplId entityRplID;
		int count = m_aProviderEntities.Count();  
		
		writer.WriteInt(count);
		for (int i = 0; i < count; i++)
		{
			RplComponent rplCmp = RplComponent.Cast(m_aProviderEntities[i].FindComponent(RplComponent));
			entityRplID = rplCmp.Id();
			writer.WriteRplId(entityRplID);
		}
		
		return true; 
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		RplId entityRplID;
		IEntity ent;
 		int count;
		reader.ReadInt(count);
		
		for (int i = 0; i < count; i++)
		{
			reader.ReadRplId(entityRplID);
			if (!entityRplID.IsValid())
				continue;
			
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(entityRplID));
			if (!rplComp)
			{
				m_aProvidersRplIds.Insert(entityRplID);
				continue;
			}
				
			ent = IEntity.Cast(rplComp.GetEntity());
			m_aProviderEntities.Insert(ent);
		}

		if (!m_aProvidersRplIds.IsEmpty())
			SCR_FreeCampaignBuildingTrigger.GetOnProviderCreated().Insert(SetProviderFromRplID);
		
		if (!m_aProviderEntities.IsEmpty())
			GetOnProviderSet().Invoke(m_aProviderEntities[0]);
		
		return true;
	}
}