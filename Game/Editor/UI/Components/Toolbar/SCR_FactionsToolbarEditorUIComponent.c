/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_FactionsToolbarEditorUIComponent: SCR_EntitiesToolbarEditorUIComponent
{
	[Attribute("NoPlayerFactionsActive")]
	protected string m_sNoPlayerFactionsActiveName;
	
	[Attribute("ButtonHint")]
	protected string m_sButtonHintName;
	
	[Attribute("Header")]
	protected string m_sHeaderName;
	
	protected Widget m_wNoPlayerFactionsActive;
	protected Widget m_wButtonHint;
	protected Widget m_wHeader;
	
	protected override bool CanOpenDialog()
	{		
		//Are there any playable factions
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegateFactionManager || delegateFactionManager.GetPlayableFactionDelegateCount() > 0)
			return super.CanOpenDialog();
		
		//Has attribute been changed yet
		SCR_AttributesManagerEditorComponent attributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent, true));
		if (!attributeManager || attributeManager.GetChangedAttributesOnce())
			return super.CanOpenDialog();
		
		//Local player has spawned yet
		SCR_PlayersManagerEditorComponent editorPlayerManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		if (!editorPlayerManager || editorPlayerManager.HasLocalPlayerSpawnedOnce())
			return super.CanOpenDialog();
		
		return false;
	}
	
	protected void FactionPlayabilityChanged(Faction faction, bool playable)
	{
		Refresh();
	}
	
	override protected void HandlerAttachedScripted(Widget w)
	{
		m_State = -1; //--- Reset the value so no entities are added by the parent class
		
		super.HandlerAttachedScripted(w);
		
		//Subscribe to Playable changed
		SCR_DelegateFactionManagerComponent delegatesManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegatesManager)
			return;
		
		map<Faction, SCR_EditableFactionComponent> delegates = new map<Faction, SCR_EditableFactionComponent>;
		delegatesManager.GetFactionDelegates(delegates);
		
		foreach(Faction faction, SCR_EditableFactionComponent delegate: delegates)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);
			
			if (scrFaction)
				scrFaction.GetOnFactionPlayableChanged().Insert(FactionPlayabilityChanged);
		}
		
		Widget parent = w.GetParent();
		
		m_wNoPlayerFactionsActive = parent.FindAnyWidget(m_sNoPlayerFactionsActiveName);
		m_wButtonHint = parent.FindAnyWidget(m_sButtonHintName);
		m_wHeader = parent.FindAnyWidget(m_sHeaderName);
	}
	override protected void Refresh()
	{
		SCR_DelegateFactionManagerComponent delegatesManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (delegatesManager)
		{
			SCR_SortedArray<SCR_EditableFactionComponent> delegates = new SCR_SortedArray<SCR_EditableFactionComponent>();
			int count = delegatesManager.GetSortedFactionDelegates(delegates);
			bool playerFactionsActive = false;
			
			m_Entities.Clear();
			for (int i; i < count; i++)
			{
				SCR_Faction scrFaction = SCR_Faction.Cast(delegates[i].GetFaction());
				
				if (scrFaction && scrFaction.IsPlayable())
				{
					m_Entities.Insert(delegates.GetOrder(i), delegates[i]);
					playerFactionsActive = true;
				}
					
			}
			
			if (m_wNoPlayerFactionsActive && m_wButtonHint && m_wHeader)
			{
				m_wNoPlayerFactionsActive.SetVisible(!playerFactionsActive);
				m_wButtonHint.SetVisible(playerFactionsActive);
				m_wHeader.SetVisible(playerFactionsActive);
			}
			
			if (m_Pagination)
				m_Pagination.SetEntryCount(m_Entities.Count());
		}
		super.Refresh();
	}
	

	override protected void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		//Remove subscription to Playable changed
		SCR_DelegateFactionManagerComponent delegatesManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (!delegatesManager)
			return;
		
		map<Faction, SCR_EditableFactionComponent> delegates = new map<Faction, SCR_EditableFactionComponent>;
		delegatesManager.GetFactionDelegates(delegates);
		
		foreach(Faction faction, SCR_EditableFactionComponent delegate: delegates)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);
			
			if (scrFaction)
				scrFaction.GetOnFactionPlayableChanged().Remove(FactionPlayabilityChanged);
		}
	}
};