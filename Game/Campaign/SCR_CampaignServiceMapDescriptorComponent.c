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
	
	[Attribute("Default Name", desc: "Name of service to be shown on hover.")]
	protected string m_sMarkerName;
	
	[Attribute("Default Description", desc: "Description of service to be shown on hover.")]
	protected string m_sMarkerDesc;
	
	[Attribute("{94F1E2223D7E0588}UI/layouts/Campaign/ServiceHint.layout")]
	protected ResourceName m_sDescriptorServiceHint;
	
	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset")]
	protected ResourceName m_sImageSet;
	
	protected string m_sMarkerSmall = "Slot_Small";
	
	protected Widget m_wDescriptorServiceHint;
	
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
	
	//------------------------------------------------------------------------------
	void ShowServiceHint(MapItem show)
	{	
		MapItem item = Item();
		if (show != item)
			return;
		
		int mx;
		int my;
		WidgetManager.GetMousePos(mx, my);

		mx = GetGame().GetWorkspace().DPIUnscale(mx);
		my = GetGame().GetWorkspace().DPIUnscale(my);

		if (!m_wDescriptorServiceHint)
		{
			SCR_MapEntity entity = SCR_MapEntity.GetMapInstance();
			m_wDescriptorServiceHint = entity.GetMapMenuRoot().FindAnyWidget("ServiceHintRoot");
		}

		if (show)
		{
			FrameSlot.SetPos(m_wDescriptorServiceHint, mx, my);
			TextWidget.Cast(m_wDescriptorServiceHint.FindAnyWidget("ServiceName")).SetTextFormat(m_sMarkerName);
			Widget resupplyText = m_wDescriptorServiceHint.FindAnyWidget("ResupplyText");
			if (resupplyText)
				resupplyText.SetVisible(false);
			
			TextWidget serviceText = TextWidget.Cast(m_wDescriptorServiceHint.FindAnyWidget("ServiceText"));
			if (serviceText)
			{
				if (m_sMarkerDesc != "Default Description")
				{
					serviceText.SetTextFormat(m_sMarkerDesc);
					serviceText.SetVisible(true);
				}
				else
				{
					serviceText.SetVisible(false);
				}
			}
		}
		m_wDescriptorServiceHint.SetVisible(true)
	}
	
	//------------------------------------------------------------------------------------------------
	void HideServiceHint(MapItem show)
	{
		MapItem item = Item();
		if (show != item)
			return;
		
		if (m_wDescriptorServiceHint)
			m_wDescriptorServiceHint.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CampaignServiceMapDescriptorComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_MapEntity.GetOnLayerChanged().Insert(SetVisible);
		SCR_MapEntity.GetOnHoverItem().Insert(ShowServiceHint);
		SCR_MapEntity.GetOnHoverEnd().Insert(HideServiceHint);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignServiceMapDescriptorComponent()
	{
		SCR_MapEntity.GetOnLayerChanged().Remove(SetVisible);
		SCR_MapEntity.GetOnHoverItem().Remove(ShowServiceHint);
		SCR_MapEntity.GetOnHoverEnd().Remove(HideServiceHint);
	}

};
