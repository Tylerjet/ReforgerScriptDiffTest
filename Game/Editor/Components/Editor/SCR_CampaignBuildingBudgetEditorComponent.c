[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingBudgetEditorComponentClass : SCR_BudgetEditorComponentClass
{
};

class SCR_CampaignBuildingBudgetEditorComponent : SCR_BudgetEditorComponent
{
	SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	SCR_CampaignSuppliesComponent m_SuppliesComponent;
	SCR_ECharacterRank m_eHighestRank;

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivateServer()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		array<ref SCR_RankID> ranks = factionManager.GetAllAvailableRanks();
		if (!ranks)
			return;

		ranks.Sort();

		SCR_RankID highestRankID = ranks[ranks.Count() - 1];
		if (!highestRankID)
			return;

		m_eHighestRank = highestRankID.GetRankID();

	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivate()
	{
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
		if (!m_CampaignBuildingComponent)
			return;

		RefreshBudgetSettings();
		m_CampaignBuildingComponent.GetOnProviderChanged().Insert(OnTargetBaseChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorDeactivate()
	{
		if (m_CampaignBuildingComponent)
			m_CampaignBuildingComponent.GetOnProviderChanged().Remove(OnTargetBaseChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void GetMainBudgetType(out array<ref EEditableEntityBudget> budgets)
	{
		budgets.Insert(EEditableEntityBudget.CAMPAIGN);
		budgets.Insert(EEditableEntityBudget.RANK_CORPORAL);
		budgets.Insert(EEditableEntityBudget.RANK_SERGEANT);
	}

	//------------------------------------------------------------------------------------------------
	protected bool RefreshSuppliesComponent()
	{
		if (!m_CampaignBuildingComponent)
			return false;

		SCR_CampaignSuppliesComponent providerSuppliesComponent;
		if (m_CampaignBuildingComponent.GetProviderSuppliesComponent(providerSuppliesComponent))
		{
			if (m_SuppliesComponent)
			{
				if (m_SuppliesComponent != providerSuppliesComponent)
				{
					m_SuppliesComponent.m_OnSuppliesChanged.Remove(OnBaseSuppliesChanged);
				}
			}
			else
			{
				providerSuppliesComponent.m_OnSuppliesChanged.Insert(OnBaseSuppliesChanged);
			}
		}

		m_SuppliesComponent = providerSuppliesComponent;

		return m_SuppliesComponent != null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseSuppliesChanged(int supplies)
	{
		RefreshSuppliesComponent();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTargetBaseChanged(IEntity targetEntity)
	{
		RefreshBudgetSettings();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudgetValue(EEditableEntityBudget type, out int maxBudget)
	{
		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
			{
				if (m_SuppliesComponent)
					maxBudget = m_SuppliesComponent.GetSuppliesMax();
					return true;
			};
			case EEditableEntityBudget.RANK_CORPORAL:
			{
				maxBudget = m_eHighestRank;
				return true;
			};
			case EEditableEntityBudget.RANK_SERGEANT:
			{
				maxBudget = m_eHighestRank;
				return true;
			};
		}

		return true;

	}

	//------------------------------------------------------------------------------------------------
	override int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		switch (type)
		{
			case EEditableEntityBudget.CAMPAIGN:
			{
				return m_SuppliesComponent.GetSuppliesMax() - m_SuppliesComponent.GetSupplies();
			};
			case EEditableEntityBudget.RANK_CORPORAL:
			{
				return m_eHighestRank - GetUserRank();
			};
			case EEditableEntityBudget.RANK_SERGEANT:
			{
				return m_eHighestRank - GetUserRank();
			};
		}

		return -1;
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
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected override void OnEntityCoreBudgetUpdatedOwner(EEditableEntityBudget entityBudget, int budgetValue, int budgetChange, bool sendBudgetMaxEvent, bool budgetMaxReached)
	{
		if (entityBudget != EEditableEntityBudget.CAMPAIGN)
			return;

		if (m_SuppliesComponent && m_SuppliesComponent.GetSupplies() - budgetChange <= 0)
			Event_OnBudgetMaxReached.Invoke(entityBudget, true);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RefreshBudgetSettings()
	{
		array<ref EEditableEntityBudget> budgetsType = {};
		GetMainBudgetType(budgetsType);

		SCR_EditableEntityCoreBudgetSetting budget;
		for (int i = budgetsType.Count() - 1; i >= 0; i--)
		{
			EEditableEntityBudget budgetType = budgetsType[i];
			int maxBudgetValue;
			if (m_EntityCore.GetBudget(budgetType, budget) && GetMaxBudgetValue(budgetType, maxBudgetValue))
			{
				m_BudgetSettingsMap.Insert(budgetType, budget);
			}
		}

		RefreshSuppliesComponent();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanPlaceEntitySource(IEntityComponentSource editableEntitySource, out EEditableEntityBudget blockingBudget, bool isPlacingPlayer = false, bool updatePreview = true, bool showNotification = true)
	{
		bool canPlace = true;
		if (IsBudgetCapEnabled() && editableEntitySource)
		{
			array<ref SCR_EntityBudgetValue> budgetCosts = {};
			if (!GetEntitySourcePreviewBudgetCosts(editableEntitySource, budgetCosts))
			{
				canPlace = false;
			}
			// Clear budget cost when placing as player
			if (isPlacingPlayer)
			{
				budgetCosts.Clear();
			}

			if (updatePreview)
			{
				UpdatePreviewCost(budgetCosts);
			}

			canPlace = canPlace && CanPlace(budgetCosts, blockingBudget);
		}
		return CanPlaceResultCampaignBuilding(canPlace, showNotification, blockingBudget);
	}

	protected bool CanPlaceResultCampaignBuilding(bool canPlace, bool showNotification, EEditableEntityBudget blockingBudget)
	{
		if (showNotification)
		{
			Rpc(CanPlaceOwnerCampaignBuilding, canPlace, blockingBudget);
		}
		return canPlace;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void CanPlaceOwnerCampaignBuilding(bool canPlace, EEditableEntityBudget blockingBudget)
	{
		if (!canPlace)
		{
			switch (blockingBudget)
			{
				case EEditableEntityBudget.CAMPAIGN:
				{
					GetManager().SendNotification(ENotification.EDITOR_PLACING_NO_ENOUGH_SUPPLIES);
					break;
				};
				case EEditableEntityBudget.RANK_CORPORAL:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_CORPORAL_NEEDED, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.CORPORAL);
					break;
				};
				case EEditableEntityBudget.RANK_SERGEANT:
				{
					SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PLACING_RANK_SERGEANT_NEEDED, SCR_PlayerController.GetLocalPlayerId(), SCR_ECharacterRank.SERGEANT);
					break;
				};
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			if (maxBudget.GetBudgetType() == type)
			{
				int newMaxBudget;
				switch (type)
				{
					case EEditableEntityBudget.CAMPAIGN:
					{
						newMaxBudget = m_SuppliesComponent.GetSupplies();
						break;
					};
					case EEditableEntityBudget.RANK_CORPORAL:
					{
						newMaxBudget = GetUserRank();
						break;
					};
					case EEditableEntityBudget.RANK_SERGEANT:
					{
						newMaxBudget = GetUserRank();
						break;
					};
				}

				maxBudget.SetBudgetValue(newMaxBudget);
				budget = maxBudget;
				return true;
			}
		}
		return false;
	}
};
