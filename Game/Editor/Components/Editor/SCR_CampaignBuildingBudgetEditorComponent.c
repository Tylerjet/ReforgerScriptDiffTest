[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CampaignBuildingBudgetEditorComponentClass : SCR_BudgetEditorComponentClass
{
};

class SCR_CampaignBuildingBudgetEditorComponent : SCR_BudgetEditorComponent
{
	SCR_CampaignBuildingEditorComponent m_CampaignBuildingComponent;
	SCR_CampaignSuppliesComponent m_SuppliesComponent;

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorActivateServer()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_CampaignBuildingComponent = SCR_CampaignBuildingEditorComponent.Cast(FindEditorComponent(SCR_CampaignBuildingEditorComponent, true, true));
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
	protected EEditableEntityBudget GetMainBudgetType()
	{
		return EEditableEntityBudget.CAMPAIGN;
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
		if (m_SuppliesComponent)
			maxBudget = m_SuppliesComponent.GetSuppliesMax();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int GetCurrentBudgetValue(EEditableEntityBudget type)
	{
		return m_SuppliesComponent.GetSuppliesMax() - m_SuppliesComponent.GetSupplies();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected override void OnEntityCoreBudgetUpdatedOwner(EEditableEntityBudget entityBudget, int budgetValue, int budgetChange, bool sendBudgetMaxEvent, bool budgetMaxReached)
	{		
		if (entityBudget != EEditableEntityBudget.CAMPAIGN)
			return;
			
		if (m_SuppliesComponent && m_SuppliesComponent.GetSupplies() - budgetChange <= 0)
			Event_OnBudgetMaxReached.Invoke( entityBudget, true);
	}

	//------------------------------------------------------------------------------------------------
	protected override void RefreshBudgetSettings()
	{
		EEditableEntityBudget budgetType = GetMainBudgetType();
		m_BudgetSettingsMap.Clear();

		int maxBudgetValue;
		SCR_EditableEntityCoreBudgetSetting budget;
		if (m_EntityCore.GetBudget(budgetType, budget) && GetMaxBudgetValue(budgetType, maxBudgetValue))
		{
			m_BudgetSettingsMap.Insert(budgetType, budget);
		}

		RefreshSuppliesComponent();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	override void CanPlaceOwner(bool canPlace)
	{
		if (!canPlace)
			GetManager().SendNotification(ENotification.EDITOR_PLACING_NO_ENOUGH_SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetMaxBudget(EEditableEntityBudget type, out SCR_EntityBudgetValue budget)
	{
		foreach	(SCR_EntityBudgetValue maxBudget : m_MaxBudgets)
		{
			if (maxBudget.GetBudgetType() == type)
			{
				int newMaxBudget;
				if (m_SuppliesComponent)
					newMaxBudget = m_SuppliesComponent.GetSupplies();

				maxBudget.SetBudgetValue(newMaxBudget);
				budget = maxBudget;
				return true;
			}
		}
		return false;
	}
};
