class SCR_BaseDestructionPhase: BaseDestructionPhase
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Model to use for the damage phase", "xob")]
	ResourceName m_sPhaseModel;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the phase")]
	ref array<ref SCR_BaseSpawnable> m_aPhaseDestroySpawnObjects;

	protected VObject m_PhaseModelCache;
	protected ref Resource m_ResourceCache;

	//------------------------------------------------------------------------------------------------
	VObject GetModel()
	{
		if (m_ResourceCache && m_PhaseModelCache)
			return m_PhaseModelCache;

		m_ResourceCache = Resource.Load(m_sPhaseModel);
		if (!m_ResourceCache)
			return null;
		
		BaseResourceObject resourceObject = m_ResourceCache.GetResource();
		if (!resourceObject)
			return null;
		
		m_PhaseModelCache = resourceObject.ToVObject();
		return m_PhaseModelCache;
	}
}