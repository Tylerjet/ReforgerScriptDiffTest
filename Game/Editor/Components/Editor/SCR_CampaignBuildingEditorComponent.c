[ComponentEditorProps(category: "GameScripted/Editor", description: "Main conflict component for handling building editor mode", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingEditorComponentClass : SCR_BaseEditorComponentClass
{
};

//------------------------------------------------------------------------------------------------

class SCR_CampaignBuildingEditorComponent : SCR_BaseEditorComponent
{
	protected ref array<SCR_CampaignBuildingProviderComponent> m_aProvidersComponents = {};
	protected ref array<RplId> m_aProvidersRplIds = {};
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected SCR_CampaignBuildingProviderComponent m_ForcedProviderComponent;
	protected int m_PlayerId;

	protected ref ScriptInvoker m_OnProviderChanged;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnProviderChanged()
	{
		if (!m_OnProviderChanged)
			m_OnProviderChanged = new ScriptInvoker();

		return m_OnProviderChanged;
	}

	//------------------------------------------------------------------------------------------------
	void AddProviderEntityEditorComponent(SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (!providerComponent || m_aProvidersComponents.Contains(providerComponent))
			return;

		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(providerComponent.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (base)
			m_aProvidersComponents.InsertAt(providerComponent, 0);
		else
			m_aProvidersComponents.Insert(providerComponent);

		GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveProviderEntityEditorComponent(SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (m_ForcedProviderComponent == providerComponent)
			SetForcedProvider(null);

		m_aProvidersComponents.RemoveItemOrdered(providerComponent);

		if (!m_aProvidersComponents.IsEmpty())
			GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignBuildingProviderComponent GetProviderComponent()
	{
		if (m_ForcedProviderComponent)
			return m_ForcedProviderComponent;

		if (m_aProvidersComponents.IsEmpty())
			return null;

		return m_aProvidersComponents[0];
	}

	//------------------------------------------------------------------------------------------------
	// Used when player initiate a building mode via user action - forced provider is an entity owning the user action
	void SetForcedProvider(SCR_CampaignBuildingProviderComponent forcedProviderComponent = null)
	{
		m_ForcedProviderComponent = forcedProviderComponent;
	}

	//------------------------------------------------------------------------------------------------
	int GetProviderEntitiesCount()
	{
		return m_aProvidersComponents.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsActiveProvider(notnull SCR_CampaignBuildingProviderComponent providerComponent)
	{
		if (providerComponent == m_ForcedProviderComponent)
			return true;

		// there exist a forced provider and it's not a tested entity, then it can't be an active provider. ToDo: Zamyslet se a zeptat
		if (m_ForcedProviderComponent)
			return false;

		if (!m_aProvidersComponents.IsEmpty() && m_aProvidersComponents[0] == providerComponent)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool GetProviderSuppliesComponent(out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		if (m_aProvidersComponents.IsEmpty())
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
		if (m_aProvidersComponents.IsEmpty())
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
	SCR_FreeRoamBuildingClientTriggerEntity GetTrigger()
	{
		if (m_aProvidersComponents.IsEmpty())
			return null;

		//ToDo: Is it still needed?
		// clear possible null, better solution - some invoker when the value in array change to null?
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
		{
			RemoveProviderEntityEditorComponent(providerComponent);
			return null;
		}

		IEntity child = GetProviderEntity().GetChildren();
		while (child)
		{
			SCR_FreeRoamBuildingClientTriggerEntity trg = SCR_FreeRoamBuildingClientTriggerEntity.Cast(child);
			if (trg)
				return trg;

			child = child.GetSibling();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//~  This function will add/remove the faction label of the saved editor state. Thus displaying the correct faction entities in the menu
	protected void AddRemoveFactionLabel(SCR_CampaignFaction faction, bool addLabel)
	{
		if (!faction)
			return;

		m_ContentBrowserManager.AddRemoveLabelOfPersistentBrowserState(faction.GetFactionLabel(), addLabel);
	}

	//------------------------------------------------------------------------------------------------
	ScriptedGameTriggerEntity SpawnClientTrigger()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return null;

		IEntity provider = GetProviderEntity();
		if (!provider)
			return null;

		Resource resource = Resource.Load(buildingManagerComponent.GetClientTriggerResourceName());
		if (!resource || !resource.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Parent = provider;

		return ScriptedGameTriggerEntity.Cast(GetGame().SpawnEntityPrefab(resource, provider.GetWorld(), params));
	}

	//------------------------------------------------------------------------------------------------
	//! Make the area around where is possible to build composition visible for player
	override protected void EOnEditorActivate()
	{
		SCR_CampaignBuildingProviderComponent providerComponenet = GetProviderComponent();
		if (!providerComponenet)
			return;
		
		if (!System.IsConsoleApp() && GetGame().GetPlayerController())
		{
			ScriptedGameTriggerEntity trigger = SpawnClientTrigger();

			if (trigger)
			{
				trigger.SetSphereRadius(providerComponenet.GetBuildingRadius());
				
				SCR_CampaignBuildingAreaMeshComponent areaMeshComponent = SCR_CampaignBuildingAreaMeshComponent.Cast(trigger.FindComponent(SCR_CampaignBuildingAreaMeshComponent));
				if (areaMeshComponent && areaMeshComponent.ShouldEnableFrameUpdateDuringEditor())
				{
					areaMeshComponent.ActivateEveryFrame();
					areaMeshComponent.GenerateAreaMesh();
				}

				trigger.SetFlags(EntityFlags.VISIBLE, false);

			}
		}

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
		m_ContentBrowserManager.SetStateTabVisible(1, GetProviderEntity().FindComponent(SCR_CampaignMilitaryBaseComponent) != null);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetUserRank()
	{		
		int playerId = SCR_PlayerController.GetLocalPlayerId();
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return SCR_ECharacterRank.INVALID;
		
		return SCR_CharacterRankComponent.GetCharacterRank(playerController.GetControlledEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpenServer()
	{
		SCR_CampaignMilitaryBaseComponent base;
		
		if (GetProviderEntity())
			base = SCR_CampaignMilitaryBaseComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (base)
			return; 
		
		GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), GetProviderComponent().GetBuildingRadius(), AssociateCompositionsToProvider, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorCloseServer()
	{
		// In case the provider of the building was the base, don't remove it's stamp from component. So the composition can't be build from another providers
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(GetProviderEntity().FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (base)
			return; 
		
		GetGame().GetWorld().QueryEntitiesBySphere(GetProviderEntity().GetOrigin(), GetProviderComponent().GetBuildingRadius(), UnassignCompositionProvider, null, EQueryEntitiesFlags.ALL);
	}

	//------------------------------------------------------------------------------------------------
	bool AssociateCompositionsToProvider(IEntity ent)
	{
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent || editableComponent.GetParentEntity())
			return true;

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
		SCR_EditableEntityComponent editableComponent = SCR_EditableEntityComponent.Cast(ent.FindComponent(SCR_EditableEntityComponent));
		if (!editableComponent || editableComponent.GetParentEntity())
			return true;
		
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
		SCR_FreeRoamBuildingClientTriggerEntity trigger = GetTrigger();
		if (trigger)
		{
			trigger.ClearFlags(EntityFlags.VISIBLE, false);

			SCR_CampaignBuildingAreaMeshComponent areaMeshComponent = SCR_CampaignBuildingAreaMeshComponent.Cast(trigger.FindComponent(SCR_CampaignBuildingAreaMeshComponent));
			if (areaMeshComponent && areaMeshComponent.ShouldEnableFrameUpdateDuringEditor())
			{
				areaMeshComponent.DeactivateEveryFrame();
			}
		}

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
	// Return the entity which belongs to currently active provider componenet.
	IEntity GetProviderEntity()
	{
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent();
		if (!providerComponent)
			return null;
		
		return providerComponent.GetOwner();
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

			IEntity provider = rplComp.GetEntity();
			if (!provider)
				continue;

			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (!providerComponent)
				continue;

			AddProviderEntityEditorComponent(providerComponent);
			m_aProvidersRplIds.RemoveOrdered(i);
			count--;
		}

		if (m_aProvidersRplIds.IsEmpty())
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Remove(SetProviderFromRplID);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplSave(ScriptBitWriter writer)
	{
		RplId entityRplID;
		int count = m_aProvidersComponents.Count();

		writer.WriteInt(count);
		for (int i = 0; i < count; i++)
		{
			IEntity provider = m_aProvidersComponents[i].GetOwner();
			if (!provider)
				continue;

			RplComponent rplCmp = RplComponent.Cast(provider.FindComponent(RplComponent));
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
			if (!ent)
				continue;

			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(ent.FindComponent(SCR_CampaignBuildingProviderComponent));
			AddProviderEntityEditorComponent(providerComponent);
		}

		if (!m_aProvidersRplIds.IsEmpty())
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Insert(SetProviderFromRplID);

		if (!m_aProvidersComponents.IsEmpty())
			GetOnProviderChanged().Invoke(m_aProvidersComponents[0]);

		return true;
	}
};
