[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_LightningContextAction : SCR_BaseContextAction
{
	[Attribute("{A8FFCD3F3697D77F}Prefabs/Editor/Lightning/LightningStrike.et")]
	protected ResourceName m_sLightningPrefab;
		
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		vector positionCheck;
		//~ Only allow lightning strike if entity has a position
		if (hoveredEntity && !hoveredEntity.GetPos(positionCheck))
			return false;
		
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		if (hoveredEntity)
			hoveredEntity.GetPos(cursorWorldPosition);
		
		Resource resource = Resource.Load(m_sLightningPrefab);
		if (!resource)
			return;
		
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
			return;
		
		//~ Spawn at ground or sea position
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.Transform[3] = cursorWorldPosition;
		
		SCR_Global.SnapToTerrain(spawnParams.Transform, GetGame().GetWorld(), true);
		
		//~ Apply scale from prefab
		float scale;
		entitySource.Get("scale", scale);
		if (scale != 1)
			SCR_Math3D.ScaleMatrix(spawnParams.Transform, scale);
		
		//~ Random rotation
		vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
		angles[0] = Math.RandomFloat(0, 360);	
		Math3D.AnglesToMatrix(angles, spawnParams.Transform);
		
		GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
	}
};