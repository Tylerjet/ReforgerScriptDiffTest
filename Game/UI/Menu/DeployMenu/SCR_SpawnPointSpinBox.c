class SCR_SpawnPointSpinBox : SCR_SpinBoxComponent
{
	protected ref array<RplId> m_aSpawnPointIds = {};

	int AddItem(string item, RplId id)
	{
		if (!id.IsValid() || m_aSpawnPointIds.Find(id) > -1)
			return -1;
		int i = AddItem(item);
		m_aSpawnPointIds.Insert(id);
		SetInitialState();

		return i;
	}

	override void RemoveItem(int item)
	{
		super.RemoveItem(item);
		
		if (m_aSpawnPointIds.IsIndexValid(item))
			m_aSpawnPointIds.Remove(item);
	}

	override void ClearAll()
	{
		super.ClearAll();
		m_aSpawnPointIds.Clear();
	}

	RplId GetSpawnPointId(int itemId)
	{
		if (m_aSpawnPointIds.IsEmpty())
			return RplId.Invalid();
		return m_aSpawnPointIds.Get(itemId);
	}

	int GetItemId(RplId id)
	{
		return m_aSpawnPointIds.Find(id);
	}

	bool IsEmpty()
	{
		return m_aSpawnPointIds.IsEmpty();
	}
};