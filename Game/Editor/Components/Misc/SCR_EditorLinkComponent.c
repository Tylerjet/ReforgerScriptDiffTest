[ComponentEditorProps(category: "GameScripted/Editor", description: "")]
class SCR_EditorLinkComponentClass: ScriptComponentClass
{
	[Attribute()]
	ref array<ref SCR_EditorLinkEntry> m_aEntries;
};

/** @ingroup Editable_Entities
*/

/*!
Link which creates replicated entity. To be use in composition prefabs which cannot contain nested replicated entities;
*/
class SCR_EditorLinkComponent : ScriptComponent
{
	protected bool m_bCanDeactivate;
	
	protected static bool s_bIgnoreSpawning;
	
	/*!
	Ignore spawning entity links in the next spawned prefab with this component.
	Used when some other system handles spawning independently.
	Has to be set before spawning the prefab, is reset to false afterwards.
	\param ignore True to ignore spawning
	*/
	static void IgnoreSpawning(bool ignore)
	{
		s_bIgnoreSpawning = ignore;
	}
	override void EOnInit(IEntity owner)
	{
		if (s_bIgnoreSpawning)
		{
			s_bIgnoreSpawning = false;
			return;
		}
		
		if ((Replication.IsClient() && !Replication.Loadtime()) || SCR_Global.IsEditMode(owner))
			return;
		
		SCR_EditorLinkComponentClass prefabData = SCR_EditorLinkComponentClass.Cast(GetComponentData(owner));
		if (!prefabData)
			return;
		
		Game game = GetGame();
		BaseWorld world = owner.GetWorld();
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.LOCAL;
		spawnParams.Parent = owner;
		
		foreach (SCR_EditorLinkEntry entry: prefabData.m_aEntries)
		{			
			entry.SetSpawnParams(spawnParams);
			if (!game.SpawnEntityPrefab(Resource.Load(entry.m_Prefab), world, spawnParams))
				Print(string.Format("Unable to spawn linked entity @\"%1\"!", entry.m_Prefab.GetPath()), LogLevel.WARNING);
		}
		
		//--- Deactivate after initialization to save resources
		if (m_bCanDeactivate)
			owner.ClearFlags(EntityFlags.ACTIVE, false);
	}
	override void OnPostInit(IEntity owner)
	{
		if ((Replication.IsClient() && !Replication.Loadtime()) || SCR_Global.IsEditMode(owner))
			return;
		
		SCR_EditableEntityComponent parent = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (!parent)
			return;
		
		m_bCanDeactivate = (owner.GetFlags() & EntityFlags.ACTIVE) == 0;
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Prefab", true)]
class SCR_EditorLinkEntry
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_Prefab;
	
	[Attribute()]
	vector m_vPosition;
	
	[Attribute()]
	vector m_vAngles;
	
	[Attribute(defvalue: "1")]
	float m_fScale;
	
	void SetSpawnParams(out EntitySpawnParams spawnParams)
	{
		Math3D.AnglesToMatrix(Vector(m_vAngles[1], m_vAngles[0], m_vAngles[2]), spawnParams.Transform);
		spawnParams.Scale = m_fScale;
		spawnParams.Transform[3] = m_vPosition;
	}
	
	void SCR_EditorLinkEntry(ResourceName prefab, vector position, vector angles, float scale)
	{
		if (!prefab.IsEmpty())
		{
			m_Prefab = prefab;
			m_vPosition = position;
			m_vAngles = angles;
			m_fScale = scale;
		}
	}
};