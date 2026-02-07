class SCR_CampaignMilitaryBaseMapDescriptorComponentClass : SCR_MilitaryBaseMapDescriptorComponentClass
{
}

class SCR_CampaignMilitaryBaseMapDescriptorComponent : SCR_MilitaryBaseMapDescriptorComponent
{	
	SCR_CampaignMilitaryBaseComponent m_CampaignBase;

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void SetParentBase(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		m_CampaignBase = base;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	void MapSetup(notnull Faction faction)
	{
		MapItem item = Item();
		Faction baseFaction = m_CampaignBase.GetFaction();
		
		if (m_CampaignBase.IsHQ() && faction != baseFaction)
		{
			item.SetVisible(false);
			return;
		}
		else
		{
			item.SetVisible(true);
			item.SetDisplayName(m_CampaignBase.GetBaseName());
			
			MapDescriptorProps props = item.GetProps();
			props.SetDetail(96);
			
			Color rangeColor;
			
			if (baseFaction)
				rangeColor = Color.FromInt(baseFaction.GetFactionColor().PackToInt());
			else
				rangeColor = Color.FromInt(Color.WHITE);
			
			props.SetOutlineColor(rangeColor);
			rangeColor.SetA(0.1);
			props.SetBackgroundColor(rangeColor);
			
			item.SetProps(props);
			item.SetRange(0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows info about the base in the map
	//! \param[in] playerFactionCampaign
	void HandleMapInfo(SCR_CampaignFaction playerFactionCampaign = null)
	{
		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.GetInstance();

		if (!campaignGameMode)
			return;
		
		if (!playerFactionCampaign)
			playerFactionCampaign = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		
		if (!playerFactionCampaign)
			playerFactionCampaign = campaignGameMode.GetBaseManager().GetLocalPlayerFaction();

		if (!playerFactionCampaign)
			return;

		if (m_CampaignBase.IsHQ() && m_CampaignBase.GetFaction() != playerFactionCampaign)
			return;
		
		// Set callsign based on player's faction
		if (m_CampaignBase.GetType() != SCR_ECampaignBaseType.RELAY && m_CampaignBase.GetCallsignDisplayName().IsEmpty())
			m_CampaignBase.SetCallsign(playerFactionCampaign);
		
		SCR_CampaignMapUIBase mapUI = m_CampaignBase.GetMapUI();
		
		if (mapUI)
			mapUI.SetIconInfoText();

		// Update base icon color
		EFactionMapID factionMapID = EFactionMapID.UNKNOWN;
		bool isInRange = m_CampaignBase.IsHQRadioTrafficPossible(playerFactionCampaign);

		// Show proper faction color only for HQs or bases within radio signal
		if (m_CampaignBase.GetFaction() && (m_CampaignBase.IsHQ() || isInRange))
		{
			switch (m_CampaignBase.GetFaction().GetFactionKey())
			{
				case campaignGameMode.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR): {factionMapID = EFactionMapID.EAST; break;};
				case campaignGameMode.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR): {factionMapID = EFactionMapID.WEST; break;};
				case campaignGameMode.GetFactionKeyByEnum(SCR_ECampaignFaction.INDFOR): {factionMapID = EFactionMapID.FIA; break;};
			}
		}
		
		Item().SetFactionIndex(factionMapID);

		array<SCR_ServicePointDelegateComponent> delegates = {};
		m_CampaignBase.GetServiceDelegates(delegates);
		
		foreach (SCR_ServicePointDelegateComponent delegate: delegates)
		{
			IEntity owner = delegate.GetOwner();
			
			if (!owner)
				continue;
			
			SCR_ServicePointMapDescriptorComponent comp = SCR_ServicePointMapDescriptorComponent.Cast(owner.FindComponent(SCR_ServicePointMapDescriptorComponent));

			if (comp)
			{
				if (isInRange)
					comp.SetServiceMarker(m_CampaignBase.GetCampaignFaction());
				else
					comp.SetServiceMarker(visible: false);
			}
		}

		if (mapUI)
			mapUI.UpdateBaseIcon(factionMapID);
	}
}
