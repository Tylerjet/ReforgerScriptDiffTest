class SCR_CampaignBuildingPreviewEntityEditorComponentClass: SCR_PreviewEntityEditorComponentClass
{
};

class SCR_CampaignBuildingPreviewEntityEditorComponent : SCR_PreviewEntityEditorComponent
{
	//! Prefab of trigger used to detect a blocking entity.
	[Attribute("", UIWidgets.ResourceNamePicker, "Trigger prefab", "et")]
	protected ResourceName m_EntityAreaTriggerPrefab;
	
	protected SCR_CampaignBuildingEntityAreaTrigger m_EntityAreaTrigger;	
	static const int BOUNDING_BOX_DIVIDER = 4;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BasePreviewEntity CreatePreview(ResourceName prefab, notnull array<vector> offsets)
	{
		super.CreatePreview(prefab, offsets);
		CreatePreviewTrigger();
		
		return m_PreviewEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DeletePreview()
	{
		super.DeletePreview();
		if (m_EntityAreaTriggerPrefab)
			SCR_EntityHelper.DeleteEntityAndChildren(m_EntityAreaTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreatePreviewTrigger()
	{
		if (!m_PreviewEntity)
			return;
		
		Resource resource = Resource.Load(m_EntityAreaTriggerPrefab);
		if (!resource.IsValid())
			return;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.Parent = m_PreviewEntity;
	
		IEntity trigger = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!trigger)
			return;
		
		m_EntityAreaTrigger = SCR_CampaignBuildingEntityAreaTrigger.Cast(trigger);	
		if (!m_EntityAreaTrigger) 
			return;
			
		vector vectorMin, vectorMax;
		m_PreviewEntity.GetPreviewBounds(vectorMin, vectorMax);
		float dist = vector.DistanceXZ(vectorMin, vectorMax);
		m_EntityAreaTrigger.SetSphereRadius(dist / BOUNDING_BOX_DIVIDER);
	}
}