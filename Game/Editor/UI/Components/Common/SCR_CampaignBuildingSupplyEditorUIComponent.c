class SCR_CampaignBuildingSupplyEditorUIComponent : SCR_BaseEditorUIComponent
{
	SCR_CampaignSuppliesComponent m_SupplyComponent;
	SCR_FactionAffiliationComponent m_FactionComponent;
	TextWidget m_ProviderName
	TextWidget m_ProviderCallsign
	TextWidget m_ProviderSupplyCurrent
	TextWidget m_ProviderSupplyMax
	OverlayWidget m_ProviderIcon;
	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_ProviderName = TextWidget.Cast(w.FindAnyWidget("Provider_Name"));
		m_ProviderCallsign = TextWidget.Cast(w.FindAnyWidget("Provider_Callsign"));
		m_ProviderSupplyCurrent = TextWidget.Cast(w.FindAnyWidget("Supply_Value_Current"));
		m_ProviderSupplyMax = TextWidget.Cast(w.FindAnyWidget("Supply_Value_Max"));
		m_ProviderIcon = OverlayWidget.Cast(w.FindAnyWidget("Provider_Icon_Overlay"));
		
		SCR_CampaignBuildingEditorComponent buildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!buildingEditorComponent)
			return;
		
		IEntity targetEntity = buildingEditorComponent.GetProviderEntity();
		if (!targetEntity)
			return;		
		
		if (!buildingEditorComponent.GetProviderSuppliesComponent(m_SupplyComponent))
			return;
		
		m_FactionComponent = buildingEditorComponent.GetProviderFactionComponent();
		if (!m_FactionComponent)
			return;
		
		SetSourceIcon(targetEntity);
		SetProviderName(targetEntity);
		
		UpdateSupply(m_SupplyComponent.GetSupplies(), m_SupplyComponent.GetSuppliesMax());
		m_SupplyComponent.m_OnSuppliesChanged.Insert(UpdateSupply);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void SetSourceIcon(IEntity targetEntity)
	{
		if (!m_ProviderIcon)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		Color factionColor;
		// We need to check both here, as vehicle has not set AffiliatedFaction if it is empty, to prevent AI to shoot at the empty vehicle. 
		Faction faction = m_FactionComponent.GetAffiliatedFaction();
		if (!faction)
			faction = m_FactionComponent.GetDefaultAffiliatedFaction();
		
		if (faction)
			factionColor = GetColorForFaction(faction.GetFactionKey());

		//This marker thing should be converted into more sandbox solution later.
		SCR_MilitarySymbolUIComponent m_SymbolUI = SCR_MilitarySymbolUIComponent.Cast(m_ProviderIcon.FindHandler(SCR_MilitarySymbolUIComponent));
		SCR_MilitarySymbol baseIcon = new SCR_MilitarySymbol();
		
		switch(faction.GetFactionKey())
		{
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.INDFOR);
				break;
			}
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.OPFOR);
				break;
			}
			case campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR):
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.BLUFOR);				
				break;
			}
			case "Unknown":
			{
				baseIcon.SetIdentity(EMilitarySymbolIdentity.UNKNOWN);
				break;
			}
		}
		
		if (Vehicle.Cast(m_FactionComponent.GetOwner()))
			baseIcon.SetIcons(EMilitarySymbolIcon.MOTORIZED);

		baseIcon.SetDimension(2);
		m_ProviderIcon.SetColor(factionColor);
		m_SymbolUI.Update(baseIcon);
	}
	
	//------------------------------------------------------------------------------
	protected Color GetColorForFaction(string factionKey)
	{
		FactionManager fm = GetGame().GetFactionManager();
		if (!fm)
			return null;

		Faction faction = fm.GetFactionByKey(factionKey);
		if (!faction)
			return null;

		return faction.GetFactionColor();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSupply(int currentSupplies, int maxSupplies)
	{
		if (m_SupplyComponent)
		{
			if (m_ProviderSupplyCurrent)
				m_ProviderSupplyCurrent.SetText(currentSupplies.ToString());
			if (m_ProviderSupplyMax)
				m_ProviderSupplyMax.SetText(maxSupplies.ToString());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetProviderName(IEntity targetEntity)
	{
		SCR_CampaignMilitaryBaseComponent targetBase = SCR_CampaignMilitaryBaseComponent.Cast(targetEntity.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (targetBase)
		{
			m_ProviderName.SetText(targetBase.GetBaseNameUpperCase());
			m_ProviderCallsign.SetText(targetBase.GetCallsignDisplayName());
			return;
		}

		m_ProviderName.SetText("");
		m_ProviderCallsign.SetText("#AR-Vehicle_SupplyTruck_Name-UC");
	}
};
