[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_CampaignServiceMapDescriptorComponentClass : SCR_MapDescriptorComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignServiceMapDescriptorComponent : SCR_MapDescriptorComponent
{

	[Attribute("", desc: "Name of marker (service is active).")]
	protected string m_sMarkerActive;
	
	[Attribute("", desc: "Name of marker (service is inctive).")]
	protected string m_sMarkerInactive;
	
	protected string m_sMarkerSmall = "Slot_Small";
	
	protected SCR_CampaignBase m_Base;
	
	//------------------------------------------------------------------------------------------------
	void SetParentBase(SCR_CampaignBase base)
	{
		m_Base = base;
		
		if (!m_Base)
			return;
		
		if (m_Base.GetOwningFaction() != SCR_RespawnSystemComponent.GetLocalPlayerFaction())
			SetServiceMarker(visible: false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetServiceMarker(SCR_CampaignFaction faction = null, bool visible = true)
	{
		MapItem item = Item();
		
		if (!item)
			return; 
		
		if (visible)
			item.SetVisible(true);
		else
		{
			item.SetVisible(false);
			return;
		}
		
		Color col = Color.White;
		
		if (faction)
			col = faction.GetFactionColor();
		else
		{
			SCR_CampaignFactionManager man = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
			
			if (man)
			{
				SCR_CampaignFaction f = man.GetCampaignFactionByKey(SCR_GameModeCampaignMP.FACTION_INDFOR);
				col = f.GetFactionColor();
			}
		}
		
		MapDescriptorProps props = item.GetProps();
		props.SetFrontColor(col);
		props.SetTextVisible(false);
		props.Activate(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetServiceMarkerActive(bool active)
	{
		MapItem item = Item();
		
		if (!item)
			return; 
		
		string marker;
		
		if (active)
			marker = m_sMarkerActive;
		else
			marker = m_sMarkerInactive;
		
		item.SetImageDef(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(int layer)
	{
		if (!m_Base)
			return;
		
		MapItem item = Item();
		MapDescriptorProps props = item.GetProps();
		
		if(layer == 0)
		{
			item.SetImageDef(m_sMarkerActive);
			props.SetIconSize(32, 0.3, 0.3);
		}
		else
		{
			item.SetImageDef(m_sMarkerSmall);
			props.SetIconSize(32, 0.1, 0.1);
		}
		
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignFaction f = SCR_CampaignFaction.Cast(campaign.GetLastPlayerFaction());
		
		if (!f)
			return;
		
		if (m_Base.IsBaseInFactionRadioSignal(f))
			Item().GetProps().SetVisible(layer < 3);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignServiceMapDescriptorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_MapEntity.GetOnLayerChanged().Insert(SetVisible);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignServiceMapDescriptorComponent()
	{
		SCR_MapEntity.GetOnLayerChanged().Remove(SetVisible);
	}

};
