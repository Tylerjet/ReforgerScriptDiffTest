class SCR_SpawnPointRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	[Attribute("Selector")]
	protected string m_sSpawnPointSelector;
	protected Widget m_wSpawnPointSelector;

	protected RplId m_SelectedSpawnPointId = RplId.Invalid();
	protected SCR_SpawnPointSpinBox m_SpawnPointSelector;

	protected ref ScriptInvoker<RplId> m_OnSpawnPointSelected;
	static ref ScriptInvoker<RplId> s_OnSpawnPointSelected;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wSpawnPointSelector = w.FindAnyWidget(m_sSpawnPointSelector);
		if (m_wSpawnPointSelector)
			m_SpawnPointSelector = SCR_SpawnPointSpinBox.Cast(m_wSpawnPointSelector.FindHandler(SCR_SpawnPointSpinBox));

		if (!m_SpawnPointSelector)
			return;

		m_SpawnPointSelector.m_OnChanged.Insert(SelectSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(RemoveSpawnPoint);		
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_SpawnPointSelector)
			m_SpawnPointSelector.m_OnChanged.Remove(SelectSpawnPoint);
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(RemoveSpawnPoint);		
	}
	
	protected void SelectSpawnPoint(SCR_SpawnPointSpinBox spinbox, int itemId)
	{
		if (spinbox.IsEmpty())
			return;

		if (spinbox.GetSpawnPointId(itemId) != m_SelectedSpawnPointId)
		{
			m_SelectedSpawnPointId = spinbox.GetSpawnPointId(itemId);
			GetOnSpawnPointSelected().Invoke(m_SelectedSpawnPointId);
			SGetOnSpawnPointSelected().Invoke(m_SelectedSpawnPointId);
		}
	}

	protected void SetSpawnPoint(RplId spawnPointId)
	{
		m_SelectedSpawnPointId = spawnPointId;
		GetOnSpawnPointSelected().Invoke(spawnPointId);
	}

	void CycleSpawnPoints(bool next = true) // false for previous spawn
	{
		int currentIndex = m_SpawnPointSelector.GetCurrentIndex();
		int nextIndex = currentIndex + 1;
		if (!next)
			nextIndex = currentIndex - 1;

		int itemCount = m_SpawnPointSelector.m_aElementNames.Count();
		if (nextIndex >= itemCount)
			nextIndex = 0;
		else if (nextIndex < 0)
			nextIndex = itemCount - 1;
		
		m_SpawnPointSelector.SetCurrentItem(nextIndex);
	}

	protected void AddSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		if (spawnPoint)
		{
			RplId currentSpawnPointId = m_SelectedSpawnPointId;
			string name = spawnPoint.GetSpawnPointName();
			m_SpawnPointSelector.AddItem(name, spawnPoint.GetRplId());
			
			if (spawnPoint.GetRplId() != currentSpawnPointId)
			{
				int itemId = m_SpawnPointSelector.GetItemId(currentSpawnPointId);
				if (itemId > -1)
					m_SpawnPointSelector.SetCurrentItem(itemId); // make sure currently selected spawn point stays selected after new point is added
			}
		}
	}

	protected void RemoveSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		RplId currentSpawnPointId = m_SelectedSpawnPointId;
		int itemId = m_SpawnPointSelector.GetItemId(spawnPoint.GetRplId());
		if (itemId == -1)
			return;

		m_SpawnPointSelector.RemoveItem(itemId);
		
		if (spawnPoint.GetRplId() != currentSpawnPointId)
		{
			itemId = m_SpawnPointSelector.GetItemId(currentSpawnPointId);
			m_SpawnPointSelector.SetCurrentItem(itemId);
			m_SelectedSpawnPointId = GetCurrentRplId();
		}
	}

	protected void OnSpawnPointFactionChange(SCR_SpawnPoint spawnPoint)
	{
		PlayerController pc = GetGame().GetPlayerController();
		SCR_PlayerFactionAffiliationComponent plyFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!plyFactionComp || !plyFactionComp.GetAffiliatedFaction())
			return;
		
		if (spawnPoint.GetFactionKey() == plyFactionComp.GetAffiliatedFaction().GetFactionKey())
			AddSpawnPoint(spawnPoint);
		else
			RemoveSpawnPoint(spawnPoint);
	}
	
	protected void ClearSpawnPoints()
	{
		m_SpawnPointSelector.ClearAll();
		// todo@lk: show warning
	}

	void ShowAvailableSpawnPoints(Faction faction)
	{
		if (!faction)
			return;

		ClearSpawnPoints();

		array<SCR_SpawnPoint> infos = SCR_SpawnPoint.GetSpawnPointsForFaction(faction.GetFactionKey());
		if (infos.IsEmpty())
		{
			SetSpawnPoint(RplId.Invalid());
			return;
		}

		foreach (SCR_SpawnPoint info : infos)
		{
			AddSpawnPoint(info);
		}
	}

	void SelectSpawnPointExt(RplId id)
	{
		int itemId = m_SpawnPointSelector.GetItemId(id);
		if (itemId > -1)
			m_SpawnPointSelector.SetCurrentItem(itemId);
	}

	ScriptInvoker GetOnSpawnPointSelected()
	{
		if (!m_OnSpawnPointSelected)
			m_OnSpawnPointSelected = new ScriptInvoker();

		return m_OnSpawnPointSelected;
	}
	
	static ScriptInvoker SGetOnSpawnPointSelected()
	{
		if (!s_OnSpawnPointSelected)
			s_OnSpawnPointSelected = new ScriptInvoker();
		
		return s_OnSpawnPointSelected;
	}

	RplId GetCurrentRplId()
	{
		return m_SpawnPointSelector.GetSpawnPointId(m_SpawnPointSelector.GetCurrentIndex());
	}
};