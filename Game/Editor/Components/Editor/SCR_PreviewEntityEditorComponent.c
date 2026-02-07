[ComponentEditorProps(category: "GameScripted/Editor", description: "Manager of preview entity. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PreviewEntityEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/

/*!
Manager of 'ghost' preview shown while transforming or placing.
*/
class SCR_PreviewEntityEditorComponent : SCR_BaseEditorComponent
{
	[Attribute(category: "Preview", params: "et", desc: "Default ghost preview entity.")]
	private ResourceName m_PreviewEntityPrefab;
	
	[Attribute(category: "Preview", params: "et", desc: "Ghost preview used when waiting for server callback.")]
	private ResourceName m_WaitingEntityPrefab;
	
	[Attribute(category: "Preview", params: "emat", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Preview entity material when confirmation is allowed.")]
	private ResourceName m_PreviewMaterial;
	
	[Attribute(category: "Preview", params: "emat", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Preview entity material when confirmation is not allowed.")]
	private ResourceName m_DisabledPreviewMaterial;
	
	[Attribute(category: "Preview", params: "emat", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	private ResourceName m_WaitingPreviewMaterial;
	
	[Attribute(category: "Preview", defvalue: "0.02")]
	private float m_fPreviewTranslationInertia;
	
	[Attribute(category: "Preview", defvalue: "0.05")]
	private float m_fPreviewRotationInertia;
	
	[Attribute(category: "Preview", defvalue: "1 1 1 0.2", desc: "Color of height indicator helper object.")]
	protected ref Color m_HeightIndicatorColor;

	[Attribute(category: "Settings", defvalue: typename.EnumToString(EEditorTransformVertical, EEditorTransformVertical.TERRAIN), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorTransformVertical))]
	private EEditorTransformVertical m_VerticalMode;

	[Attribute(category: "Settings", defvalue: SCR_Enum.GetFlagValues(EEditorTransformVertical).ToString(), uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditorTransformVertical))]
	private EEditorTransformVertical m_AllowedVerticalModes;
	
	[Attribute(category: "Settings", defvalue: typename.EnumToString(EEditorTransformSnap, EEditorTransformSnap.TERRAIN), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorTransformSnap))]
	private EEditorTransformSnap m_VerticalSnap;
	
	[Attribute(category: "Settings", defvalue: "0.5")]
	private float m_fVerticalSnapLimit;
	
	[Attribute(category: "Settings", desc: "When true, interaction between entities passes when at least one edited entity meets the criteria.\nWhen false, all edited entities must be compatible.")]
	private bool m_bORInteraction;
	
	[Attribute(category: "Settings", defvalue: "EditorPreviewContext", desc: "Action context activated all the time.")]
	protected string m_sActionContext;
	
	private vector m_vTransform[4] = {vector.Right, vector.Up, vector.Forward, vector.Zero};
	//private vector m_vTransformBackup[4];
	private EEditorTransformVertical m_VerticalModeReal;
	private float m_fHeightTerrain;
	private float m_fHeightSea;
	private float m_fHeightGeometry;
	private bool m_bIsHeightSet;
	private bool m_bIsChange;
	private BaseWorld m_World;
	private bool m_bHasTerrain;
	private SCR_EditableEntityComponent m_Target;
	private bool m_bTargetSnap;
	private bool m_bTargetDelegate;
	private EEditableEntityInteraction m_TargetInteraction;
	private IEntity m_TargetHelper;
	private float m_fTargetRotationStep;
	private bool m_bInstantTransformation;
	private bool m_bIsFixedPosition;
	private bool m_bIsRotating;
	private bool m_bIsMovingVertically;
	private bool m_bHasSpecialInteraction;
	private bool m_bIsUnderwater;
	private int m_iHeightIndicatorColor;
	private bool m_bPreviewDisabled;
	protected bool m_bCanMoveInRoot;
	
	protected SCR_LayersEditorComponent m_LayerManager;
	protected SCR_StatesEditorComponent m_StateManager;
	protected SCR_BasePreviewEntity m_PreviewEntity;
	private ResourceName m_SlotPrefab;
	private vector m_vLocalOffset;
	private SCR_EditableEntityComponent m_Entity;
	private EEditableEntityType m_EntityType;
	protected ref SCR_EditableEntityInteraction m_Interaction;
	
	// OnPreviewCreate(SCR_BasePreviewEntity previewEntity)
	private ref ScriptInvoker Event_OnPreviewCreate = new ScriptInvoker;
	// OnPreviewDelete(SCR_BasePreviewEntity previewEntity)
	private ref ScriptInvoker Event_OnPreviewDelete = new ScriptInvoker;
	// OnPreviewChange(vector transform[4], bool isChange)
	private ref ScriptInvoker Event_OnPreviewChange = new ScriptInvoker;
	// OnTargetChange(SCR_EditableEntityComponent target)
	private ref ScriptInvoker Event_OnTargetChange = new ScriptInvoker;
	// OnWaitingPreviewCreate(SCR_BasePreviewEntity previewEntity)
	private ref ScriptInvoker Event_OnWaitingPreviewCreate = new ScriptInvoker;
	// OnVerticalModeChange(EEditorTransformVertical verticalMode)
	private ref ScriptInvoker Event_OnVerticalModeChange = new ScriptInvoker;
	// OnVerticalModeChange(EEditorTransformSnap verticalSnap)
	private ref ScriptInvoker Event_OnVerticalSnapChange = new ScriptInvoker;
	
	/*!
	Set transformation of the preview entity.
	When editing is confirmed, real entities are moved to where the preview is.
	\param transform Preview transformation
	*/
	void SetPreviewTransform(vector transform[4], float timeSlice = 1, bool instant = false, EEditorTransformVertical verticalMode = EEditorTransformVertical.TERRAIN)
	{
		if (!m_PreviewEntity || m_bIsFixedPosition)
			return;
		
		instant = true;
		
		vector cameraTransform[4];
		m_World.GetCurrentCamera(cameraTransform);
		vector cameraPos = cameraTransform[3];
		m_bIsUnderwater = cameraPos[1] < m_World.GetOceanBaseHeight();
		
		if (m_Target && m_bTargetSnap)
		{
			//--- Attached to a target, override input transformation
			vector targetTransform[4];
			if (m_Target.GetTransform(targetTransform))
			{
				switch (true)
				{
					//--- Disabled rotation
					case (m_fTargetRotationStep == -1):
					{
						transform = targetTransform;
						break;
					}
					//--- Stepped rotation
					case (m_fTargetRotationStep != 0):
					{
						vector targetAngles = Math3D.MatrixToAngles(targetTransform);
						vector angles = Math3D.MatrixToAngles(transform);
						angles[0] = targetAngles[0] + Math.Round((angles[0] - targetAngles[0]) / m_fTargetRotationStep) * m_fTargetRotationStep;
						if (!m_bInstantTransformation && !instant) LerpAngles(m_TargetHelper.GetAngles(), angles, timeSlice);
						Math3D.AnglesToMatrix(angles, transform);
						break;
					}
					//--- Free rotation (do not modify incoming dir)
					default:
					{
						if (!m_bInstantTransformation) 
						{
							vector angles = Math3D.MatrixToAngles(transform);
							if (!m_bInstantTransformation && !instant) LerpAngles(m_TargetHelper.GetAngles(), angles, timeSlice);
							Math3D.AnglesToMatrix(angles, transform);
						}
					}
				}
				transform[3] = targetTransform[3];
				if (!m_bInstantTransformation && !instant) transform[3] = LerpTranslation(m_TargetHelper.GetOrigin(), transform[3], timeSlice);
				m_TargetHelper.SetWorldTransform(transform);
			}
		}
		else
		{
			//--- Free transformation			
			if (!m_bInstantTransformation && !instant) 
			{
				vector transformCurrent[4];
				m_PreviewEntity.GetWorldTransform(transformCurrent);
				
				float quat[4], quatCurrent[4];
				Math3D.MatrixToQuat(transform, quat);
				Math3D.MatrixToQuat(transformCurrent, quatCurrent);
				Math3D.QuatLerp(quatCurrent, quat, quat, timeSlice);
				Math3D.QuatToMatrix(quat, transform);
				
				transform[3] = LerpTranslation(m_PreviewEntity.GetOrigin(), transform[3], timeSlice);
			}
			
			m_PreviewEntity.SetPreviewTransform(transform, verticalMode, m_fHeightTerrain, m_bIsUnderwater);
		}
		
		m_bIsChange = true;
		m_bInstantTransformation = false;
		m_VerticalModeReal = verticalMode;
		Event_OnPreviewChange.Invoke(transform, true);
		
		//--- Height not defined yet, set it now
		if (!m_bIsHeightSet) SetPreviewHeight(transform[3]);
		
		//--- Placeholder vertical indicator. ToDo: Replace
		Shape.CreateCylinder(m_iHeightIndicatorColor, ShapeFlags.TRANSP | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, transform[3] - Vector(0, 50, 0), 0.05, 100);
	}
	/*!
	Reset transformation of the preview entity back to its original coordinates.
	*/
	void ResetPreviewTransform()
	{
		if (!m_PreviewEntity)
			return;
		
		m_PreviewEntity.SetPreviewTransform(m_vTransform, EEditorTransformVertical.SEA, m_fHeightTerrain);
		
		bool isChange = m_bIsChange;
		m_bIsChange = false;
		if (isChange)
			Event_OnPreviewChange.Invoke(m_vTransform, false);
	}
	/*!
	Get transformation of the preview entity.
	\param transform Preview transformation
	*/
	bool GetPreviewTransform(out vector transform[4])
	{
		if (!m_PreviewEntity)
			return false;
		
		m_PreviewEntity.GetWorldTransform(transform);
		return true;
	}
	/*!
	Get transformation of the preview entity from when transformation started.
	\param transform Origin preview transformation
	*/
	bool GetPreviewTransformOrigin(out vector transform[4])
	{
		if (!m_PreviewEntity) return false;
		Math3D.MatrixCopy(m_vTransform, transform);
		return true;
	}
	/*!
	Set height in which the preview entity is.
	Height of the preview is maintained based on specific editing modes (e.g., above terrain or above sea).
	\param pos
	*/
	void SetPreviewHeight(vector pos)
	{
		if (!m_PreviewEntity || !m_World) return;
		m_fHeightSea = pos[1];
		if (m_bHasTerrain)
			m_fHeightTerrain = pos[1] - SCR_TerrainHelper.GetTerrainY(pos, m_World, !m_bIsUnderwater);
		else
			m_fHeightTerrain = m_fHeightSea;
		m_bIsHeightSet = true;
	}
	/*!
	Get preview's height above terrain.
	\return Height
	*/
	float GetPreviewHeightAboveTerrain()
	{
		return m_fHeightTerrain;
	}
	/*!
	Get preview's height above sea.
	\return Height
	*/
	float GetPreviewHeightAboveSea()
	{
		return m_fHeightSea;
	}
	/*!
	Get preview's height above geometry.
	\return Height
	*/
	float GetPreviewHeightAboveGeometry()
	{
		return m_fHeightGeometry;
	}
	/*!
	\return Offset of pivot on which the preview is centered (e.g., when dragging entity by its mesh, not by icon)
	*/
	vector GetLocalOffset()
	{
		return m_vLocalOffset;
	}
	/*!
	Set mode which defines which height the preview maintains.
	\param Mode
	*/
	void SetVerticalMode(EEditorTransformVertical mode)
	{
		if (mode == m_VerticalMode || !SCR_Enum.HasFlag(m_AllowedVerticalModes, mode))
			return;
		
		m_VerticalMode = mode;
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_MODE, m_VerticalMode);
		Event_OnVerticalModeChange.Invoke(m_VerticalMode);
	}
	/*!
	Get mode which defines which height the preview maintains.
	\return Mode
	*/
	EEditorTransformVertical GetVerticalMode()
	{
		return m_VerticalMode;
	}
	/*!
	Get actually used vertical mode
	(in certain circumstances, the mode defaults to SEA instead of using what user chose)
	\return Mode
	*/
	EEditorTransformVertical GetVerticalModeReal()
	{
		return m_VerticalModeReal;
	}
	/*!
	Set vertical snapping rules.
	\param Mode
	*/
	void SetVerticalSnap(EEditorTransformSnap snap)
	{
		if (snap == m_VerticalSnap)
			return;
		
		m_VerticalSnap = snap;
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_SNAP, m_VerticalSnap);
		Event_OnVerticalSnapChange.Invoke(m_VerticalSnap);
	}
	/*!
	Get vertical snapping rules.
	\return Mode
	*/
	EEditorTransformSnap GetVerticalSnap()
	{
		return m_VerticalSnap;
	}
	/*!
	Snap position according to currently active snapping rules
	\param pos Unmodified position
	\return Modified position
	*/
	vector SnapVertically(vector pos)
	{
		if (m_PreviewEntity && GetVerticalSnap() == EEditorTransformSnap.TERRAIN)
		{
			float snapOffsetHeight = m_vLocalOffset[1];
			float surfaceY = SCR_TerrainHelper.GetTerrainY(pos, m_World, !m_bIsUnderwater);
			
			//if (Math.AbsFloat(pos[1] - snapOffsetHeight - surfaceY) < m_fVerticalSnapLimit) //--- This allows to move the entity below terrain
			if (pos[1] - snapOffsetHeight - surfaceY < m_fVerticalSnapLimit)
				pos[1] = surfaceY + snapOffsetHeight;
		}
		return pos;
	}
	/*!
	Get height offset needed to achieve the suface snap
	\param pos Position
	\return Height offset
	*/
	float GetVerticalSnapDelta(vector pos)
	{
		if (!m_PreviewEntity)
			return 0;
		
		return -pos[1] + SCR_TerrainHelper.GetTerrainY(pos, m_World, !m_bIsUnderwater) + m_vLocalOffset[1];
	}
	/*!
	Checked if preview entity is snapped according to currently active snapping rules
	\return True when snapped
	*/
	bool IsSnappedVertically()
	{
		if (!m_PreviewEntity)
			return false;
		
		if (m_VerticalSnap == EEditorTransformSnap.TERRAIN)
		{
			//--- ToDo: One function to get snap offset used in here and in SnapVertically()
			vector pos = m_PreviewEntity.GetTransformAxis(3);
			float snapOffsetHeight = m_vLocalOffset[1];
			float surfaceY = SCR_TerrainHelper.GetTerrainY(pos, m_World, !m_bIsUnderwater);
			return Math.AbsFloat(pos[1] - snapOffsetHeight - surfaceY) < m_fVerticalSnapLimit;
		}
		return false;
	}
	/*!
	Checked if preview entity is snapped and can be unsnapped
	\param verticalDelta Intended vertical change
	\return True if the entity is snapped and can be unsnapped
	*/
	bool CanUnsnap(float verticalDelta)
	{
		if (!IsSnappedVertically()) return false;
		
		if (m_VerticalSnap == EEditorTransformSnap.TERRAIN)
		{
			return verticalDelta > 0; //--- Cannot go below terrain
		}
		return false;
	}
	/*!
	\return True if the preview entity can be moved to root
	*/
	bool CanMoveInRoot()
	{
		return m_bCanMoveInRoot;
	}
	/*!
	Show preview entity as disabled.
	\param disable True to show as disabled, false to show in normal state
	*/
	void ShowAsDisabled(bool disable = true)
	{
		if (disable == m_bPreviewDisabled)
			return;
		
		m_bPreviewDisabled = disable;
		
		ResourceName material = m_PreviewMaterial;
		if (disable)
			material = m_DisabledPreviewMaterial;
		
		SCR_Global.SetMaterial(m_PreviewEntity, material);
	}

	/*!
	Attach preview entity to a target.
	\param target Target
	\return True if the entity was attached
	*/
	bool SetTarget(SCR_EditableEntityComponent target, bool isDelegate = false)
	{
		if (!m_PreviewEntity)
			return false;
		
		if (target)
		{
			if (target == m_Target && isDelegate == m_bTargetDelegate)
				return false;
			
			//--- Check if the preview can interact with the target
			bool targetSnap;
			EEditableEntityInteraction targetInteraction;
			if (!CanInteractWith(target, targetSnap, targetInteraction, isDelegate))
			{
				if (m_Target) SetTarget(null);
				return true;
			}
			
			m_Target = target;
			m_bTargetSnap = targetSnap;
			m_TargetInteraction = targetInteraction;
			m_bTargetDelegate = isDelegate;
			
			if (m_bTargetSnap)
			{
				vector transform[4];
				m_PreviewEntity.GetWorldTransform(transform);
				
				//--- Move preview into helper entity which counters local offset, so rotation is around target's center
				m_TargetHelper = GetGame().SpawnEntity(GenericEntity, m_World);
				m_TargetHelper.SetTransform(transform);
				m_TargetHelper.AddChild(m_PreviewEntity, -1);
				vector localTransform[4] = {vector.Right, vector.Up, vector.Forward, m_vLocalOffset};
				m_PreviewEntity.SetLocalTransform(localTransform);
				
				SCR_SiteSlotEntity slotEntity = SCR_SiteSlotEntity.Cast(target.GetOwner());
				if (slotEntity)
					m_fTargetRotationStep = slotEntity.GetRotationStep();
				else
					m_fTargetRotationStep = -1;
			}
		}
		else
		{
			//--- Remove
			if (!m_Target) return false;
			
			vector transform[4];
			m_PreviewEntity.GetWorldTransform(transform);
			
			m_Target = null;
			m_TargetInteraction = 0;
			if (m_TargetHelper)
			{
				m_TargetHelper.RemoveChild(m_PreviewEntity);
				delete m_TargetHelper;
			}
			
			m_PreviewEntity.SetWorldTransform(transform);
			//SetPreviewTransform(transform);
		}
		Event_OnTargetChange.Invoke(target);
		return true;
	}
	protected bool CanInteractWith(SCR_EditableEntityComponent target, out bool snap, out EEditableEntityInteraction interaction, bool isDelegate)
	{
		//--- Cannot interact with itself
		if (target == m_Entity)
			return false;
		
		//--- Snap to compatible slot
		if (target.HasEntityState(EEditableEntityState.COMPATIBLE_SLOT))
		{
			interaction = EEditableEntityInteraction.SLOT;
			snap = true;
			return true;
		}
		
		EEditableEntityInteractionFlag defaultInteractionFlags;
		if (!m_LayerManager || m_LayerManager.IsEditingLayersEnabled())
			defaultInteractionFlags |= EEditableEntityInteractionFlag.LAYER_EDITING;

		//--- Check interaction of all edited entities
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		//SCR_EditablePreviewEntity editableChild;
		SCR_EditableEntityComponent editableChild;
		if (core)
		{
			if (m_bORInteraction)
			{
				//--- At least one entity must be compatible
				foreach (SCR_BasePreviewEntity child: m_PreviewEntity.GetPreviewChildren())
				{
					if (!m_Interaction)
						continue;
					
					EEditableEntityInteractionFlag interactionFlags = defaultInteractionFlags;
					
					editableChild = SCR_EditableEntityComponent.GetEditableEntity(child.GetSourceEntity());
					if (!editableChild || !editableChild.IsDestroyed())
						interactionFlags |= EEditableEntityInteractionFlag.ALIVE;
					
					if (isDelegate)
						interactionFlags |= EEditableEntityInteractionFlag.DELEGATE;
					
					if (!editableChild || editableChild.GetPlayerID() == 0)
						interactionFlags |= EEditableEntityInteractionFlag.NON_PLAYABLE;
					
					if (m_StateManager && m_StateManager.GetState() == EEditorState.PLACING)
						interactionFlags |= EEditableEntityInteractionFlag.PLACING;
					
					if (m_Interaction.CanSetParent(target, interactionFlags))
					{
						interaction = EEditableEntityInteraction.LAYER;
						return true;
					}
				}
			}
			else
			{
				//--- All entities must be compatible
				foreach (SCR_BasePreviewEntity child: m_PreviewEntity.GetPreviewChildren())
				{
					if (!m_Interaction)
						return false;
					
					EEditableEntityInteractionFlag interactionFlags = defaultInteractionFlags;
					
					editableChild = SCR_EditableEntityComponent.GetEditableEntity(child.GetSourceEntity());
					if (!editableChild || !editableChild.IsDestroyed())
						interactionFlags |= EEditableEntityInteractionFlag.ALIVE;
					
					if (isDelegate)
						interactionFlags |= EEditableEntityInteractionFlag.DELEGATE;
					
					if (!editableChild || editableChild.GetPlayerID() == 0)
						interactionFlags |= EEditableEntityInteractionFlag.NON_PLAYABLE;
					
					if (m_StateManager && m_StateManager.GetState() == EEditorState.PLACING)
						interactionFlags |= EEditableEntityInteractionFlag.PLACING;
					
					if (!m_Interaction.CanSetParent(target, interactionFlags))
						return false;
				}
				interaction = EEditableEntityInteraction.LAYER;
				return true;
			}
		}
		return false;
	}
	
	/*!
	Get target to which the preview is currently snapped to.
	\return Target editable entity
	*/
	SCR_EditableEntityComponent GetTarget()
	{
		return m_Target;
	}
	/*!
	Get target interaction type.
	\return Interaction type
	*/
	EEditableEntityInteraction GetTargetInteraction()
	{
		return m_TargetInteraction;
	}
	/*!
	Check if snapped to taregt.
	\return True if snapped
	*/
	bool IsSnappedToTarget()
	{
		return m_bTargetSnap;
	}
	/*!
	Get preview entity.
	\return Preview entity
	*/
	SCR_BasePreviewEntity GetPreviewEntity()
	{
		return m_PreviewEntity;
	}
	/*!
	Get action context activated every frame.
	\return Action context name
	*/
	string GetActionContext()
	{
		return m_sActionContext;
	}
	/*!
	Check if editing is currently on.
	\return True when editing
	*/
	bool IsEditing()
	{
		return m_PreviewEntity != null;
	}
	/*!
	Check if the preview changed while editing.
	\return True if changed
	*/
	bool IsChange()
	{
		return m_bIsChange || m_bIsFixedPosition;
	}
	/*!
	Is the entity being rotated in this frame?
	\return True if being rotated
	*/
	bool IsRotating()
	{
		return m_bIsRotating;
	}
	/*!
	Is the entity fixed to specific position?
	\return True if the position is fixed
	*/
	bool IsFixedPosition()
	{
		return m_bIsFixedPosition;
	}
	/*!
	Create slot prfab to which the preview can fit to.
	\return Prefab resource
	*/
	ResourceName GetSlotPrefab()
	{
		return m_SlotPrefab;
	}
	/*!
	Set if the entity is being moved vertically this frame.
	\param isRotating True if being moved vertically
	*/
	void SetIsMovingVertically(bool isMovingVertically)
	{
		m_bIsMovingVertically = isMovingVertically;
	}
	/*!
	Is the entity being moved vertically in this frame?
	\return True if being moved vertically
	*/
	bool IsMovingVertically()
	{
		return m_bIsMovingVertically;
	}
	/*!
	Set if the entity is being rotated this frame.
	\param isRotating True if being rotated
	*/
	void SetIsRotating(bool isRotating)
	{
		m_bIsRotating = isRotating;
	}
	/*!
	Is editing underwater?
	\return True when currently editing underwater
	*/
	bool IsUnderwater()
	{
		return m_bIsUnderwater;
	}
	/*!
	Get event called when a preview is created (e.g., transforming or placing starts).
	\return Script invoker
	*/
	ScriptInvoker GetOnPreviewCreate()
	{
		return Event_OnPreviewCreate;
	}
	/*!
	Get event called when a preview is deleted (e.g., transforming or placing ends).
	\return Script invoker
	*/
	ScriptInvoker GetOnPreviewDelete()
	{
		return Event_OnPreviewDelete;
	}
	/*!
	Get event called every time preview transformation is updated.
	\return Script invoker
	*/
	ScriptInvoker GetOnPreviewChange()
	{
		return Event_OnPreviewChange;
	}
	/*!
	Get event called every time preview is snapped or unsapped from a target.
	\return Script invoker
	*/
	ScriptInvoker GetOnTargetChange()
	{
		return Event_OnTargetChange;
	}
	/*!
	Get event called when waiting preview is created (e.g., after placing entity on client, while waiting for server callback).
	\return Script invoker
	*/
	ScriptInvoker GetOnWaitingPreviewCreate()
	{
		return Event_OnWaitingPreviewCreate;
	}
	/*!
	Get event called when vertical mode changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnVerticalModeChange()
	{
		return Event_OnVerticalModeChange;
	}
	/*!
	Get event called when vertical snapping changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnVerticalSnapChange()
	{
		return Event_OnVerticalSnapChange;
	}
	
	void InitTransform(vector transform[4])
	{
		m_vTransform = transform;
		SetPreviewHeight(transform[3]);
	}
	
	/*!
	Create preview from existing entities.
	\param pivot Directly edited entity
	\param entities List of edited entities
	\return Created preview entity
	*/
	SCR_BasePreviewEntity CreatePreview(SCR_EditableEntityComponent pivot, set<SCR_EditableEntityComponent> entities)
	{
		DeletePreview();
		
		EPreviewEntityFlag flags;
		if (m_bIsUnderwater)
			flags |= EPreviewEntityFlag.UNDERWATER;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		Math3D.MatrixCopy(m_vTransform, spawnParams.Transform);
		m_PreviewEntity = SCR_EditablePreviewEntity.SpawnPreviewFromEditableEntities(entities, m_PreviewEntityPrefab, GetOwner().GetWorld(), spawnParams, m_PreviewMaterial, flags);
		m_bInstantTransformation = true;
		m_Entity = pivot;
		m_EntityType = pivot.GetEntityType();
		m_Interaction = pivot.GetEntityInteraction();
		m_bCanMoveInRoot = !m_Interaction || m_Interaction.CanSetParent(SCR_EditableEntityInteraction.ROOT, 0);
		
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(pivot.GetInfo());
		if (info)
			m_SlotPrefab = info.GetSlotPrefab();
		else
			m_SlotPrefab = ResourceName.Empty;
		
		//--- Get local offset for slotting
		if (pivot.GetPos(m_vLocalOffset))
		{
			vector pivotTransform[4];
			pivot.GetOwner().GetWorldTransform(pivotTransform);
			Math3D.MatrixInvMultiply4(m_vTransform, pivotTransform, pivotTransform);
			m_vLocalOffset = -pivotTransform[3];// pivot.GetOwner().CoordToLocal(m_vTransform[3]); //--- Cannot use CoordToLocal, doesn't take m_vTransform rotation into effect
		}
		
		Event_OnPreviewCreate.Invoke(m_PreviewEntity);
		
		//--- Reset position. Without it, moving entity vertically on client (where there's a delay) would cause vertical offset
		SetPreviewHeight(m_vTransform[3]);
		ResetPreviewTransform();
		
		return m_PreviewEntity;
	}
	/*!
	Create preview from prefab.
	\param prefab IEntity prefab
	\param offsets Offsets when multiple instances of the preview entity are to be spawned
	\return Created preview entity
	*/
	SCR_BasePreviewEntity CreatePreview(ResourceName prefab, notnull array<vector> offsets)//, vector fixedTransform[4])
	{		
		DeletePreview();
		
		if (prefab)
		{
			Resource prefabResource = Resource.Load(prefab);
			EntitySpawnParams spawnParams = new EntitySpawnParams();;
			
			IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(prefabResource);
			IEntityComponentSource entityComponent = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_EditableEntityComponent);
			m_EntityType = SCR_EditableEntityComponentClass.GetEntityType(entityComponent);
			m_Interaction = SCR_EditableEntityComponentClass.GetEntityInteraction(entityComponent);
			m_SlotPrefab = SCR_EditableEntityComponentClass.GetSlotPrefab(entityComponent);
			m_bIsFixedPosition = SCR_EditableEntityComponentClass.HasFlag(entityComponent, EEditableEntityFlag.STATIC_POSITION);
			m_bCanMoveInRoot = !m_Interaction || m_Interaction.CanSetParent(SCR_EditableEntityInteraction.ROOT, 0);
			
			if (m_bIsFixedPosition)
			{
				//--- Get coordinates of the prefab itself when it's intended only for one position
				vector coords;
				entitySource.Get("coords", coords);
				if (coords.LengthSq() != 0)
				{
					spawnParams.Transform[3] = coords;
				}
				else
				{
					Print(string.Format("Prefab '%1' is flagged as STATIC_POSITION, but its position is <0,0,0>!", prefab.GetPath()), LogLevel.WARNING);
					m_bIsFixedPosition = false;
				}
			}
			else
			{
				//--- Restore previous yaw (don't apply full rotation, would cause problems on uneven terrain; ToDo: Use rotation relativer to terrain?)
				vector angles = Vector(Math3D.MatrixToAngles(m_vTransform)[0], 0, 0);
				Math3D.AnglesToMatrix(angles, m_vTransform);
				m_vTransform[3] = vector.Zero; //--- Reset position to prevent waypoint appearing in the air
				spawnParams.Transform = m_vTransform;
			}
			
			if (offsets.Count() > 1)
			{
				//--- Create as many previews as there are selected entities (e.g., for waypoins or tasks)
				array<ref SCR_BasePreviewEntry> entries = SCR_PrefabPreviewEntity.GetPreviewEntriesFromPrefab(prefabResource);
				SCR_BasePreviewEntry sourceEntry = entries[0];
				SCR_BasePreviewEntry entry;
				for (int i = 0, count = offsets.Count(); i < count; i++)
				{
					if (i == 0)
					{
						entry = entries[i];
					}
					else
					{
						entry = new SCR_BasePreviewEntry();
						entry.CopyFrom(sourceEntry);
						entries.Insert(entry);
					}
					entry.m_vPosition = offsets[i];
				}
				m_PreviewEntity = SCR_PrefabPreviewEntity.SpawnPreview(entries, m_PreviewEntityPrefab, spawnParams: spawnParams, material: m_PreviewMaterial);
			}
			else
			{
				//--- Create preview directly
				m_PreviewEntity = SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(prefabResource, m_PreviewEntityPrefab, spawnParams: spawnParams, material: m_PreviewMaterial);
			}
			if (m_PreviewEntity)
			{
				m_fHeightTerrain = 0;				
				Event_OnPreviewCreate.Invoke(m_PreviewEntity);
			}
		}
		return m_PreviewEntity;
	}
	/*!
	Delete current preview entity.
	*/
	void DeletePreview()
	{
		if (m_PreviewEntity)
		{
			SCR_BasePreviewEntity previewEntity = m_PreviewEntity;
			m_PreviewEntity = null;
			
			previewEntity.GetWorldTransform(m_vTransform);
			if (m_Target)
				Event_OnTargetChange.Invoke(null);
			
			Event_OnPreviewDelete.Invoke(previewEntity);
			delete previewEntity;
		}
		m_bIsChange = false;
		m_bIsHeightSet = false;
		m_bIsFixedPosition = false;
		m_bIsRotating = false;
		m_bPreviewDisabled = false;
		m_Target = null;
		m_TargetInteraction = 0;
		m_vLocalOffset = vector.Zero;
		m_Entity = null;
	}
	/*!
	Duplicate the preview to indicate waiting.
	*/
	SCR_BasePreviewEntity CreateWaitingPreview()
	{
		if (!m_PreviewEntity)
			return null;
		
		Resource previewResource = Resource.Load(m_WaitingEntityPrefab);
		if (!previewResource || !previewResource.IsValid()) 
			return null;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		m_PreviewEntity.GetWorldTransform(spawnParams.Transform);
		SCR_BasePreviewEntity previewEntity = SCR_EditablePreviewEntity.SpawnPreviewFromEditableOwner(m_PreviewEntity, m_WaitingEntityPrefab, m_PreviewEntity.GetWorld(), spawnParams, m_WaitingPreviewMaterial);
		Event_OnWaitingPreviewCreate.Invoke(previewEntity);
		return previewEntity;
	}
	
	protected vector LerpTranslation(vector currentPos, vector targetPos, float timeSlice)
	{
		return vector.Lerp(currentPos, targetPos, Math.Min(timeSlice * m_fPreviewTranslationInertia, 1));
	}
	protected void LerpAngles(vector currentAngles, out vector targetAngles, float timeSlice)
	{
		//--- currentAngles vector has different order than targetAngles vector, because it's filled by GetAngles()
		targetAngles[0] = SCR_Math.LerpAngle(currentAngles[1], targetAngles[0], Math.Min(timeSlice * m_fPreviewRotationInertia, 1));
	}
	
	protected void OnUserSettingsChanged()
	{
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (!editorSettings)
			return;
		
		int value;
		editorSettings.Get("m_PreviewVerticalSnap", value);
		SetVerticalSnap(value);
		editorSettings.Get("m_PreviewVerticleMode", value);
		SetVerticalMode(value);
	}
	
	
	override void EOnEditorDebug(array<string> debugTexts)
	{
		debugTexts.Insert("Vertical Mode: " + Type().EnumToString(EEditorTransformVertical, m_VerticalMode));
		debugTexts.Insert("Vertical Mode (Real): " + Type().EnumToString(EEditorTransformVertical, m_VerticalModeReal));
		
		if (m_PreviewEntity)
		{
			debugTexts.Insert(string.Format("Preview Target: %1", m_Target));
			debugTexts.Insert(string.Format("Preview Target Interaction: %1", typename.EnumToString(EEditableEntityInteraction, m_TargetInteraction)));
		}
	}
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_MODE) != m_VerticalMode)
		{
			SetVerticalMode(DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_MODE));
		}
		if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_SNAP) != m_VerticalSnap)
		{
			SetVerticalSnap(DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_SNAP));
		}
		#endif
	}
	override void EOnEditorActivate()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);

		ArmaReforgerScripted game = GetGame();
		if (!game) return;
		
		m_World = game.GetWorld();
		m_LayerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		m_StateManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
		
		#ifdef ENABLE_DIAG
		typename enumVerticalMode = EEditorTransformVertical;
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_MODE, "", "Vertical Mode", "Transforming", string.Format("0 %1 0 1", enumVerticalMode.GetVariableCount() - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_MODE, m_VerticalMode);
		
		typename enumVerticalSnap = EEditorTransformSnap;
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_SNAP, "", "Vertical Snap", "Transforming", string.Format("0 %1 0 1", enumVerticalSnap.GetVariableCount() - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_TRANSFORM_VERTICAL_SNAP, m_VerticalSnap);
		#endif
	}
	override void EOnEditorDeactivate()
	{
		DeletePreview();
	}
	override void EOnEditorInit()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game) 
			return;
		
		GenericWorldEntity worldEntity = game.GetWorldEntity();
		if (!worldEntity) 
			return;
		
		m_bHasTerrain = worldEntity.GetTerrain(0, 0) != null;
		
		m_fPreviewTranslationInertia = 1 / Math.Max(m_fPreviewTranslationInertia, 0.001);
		m_fPreviewRotationInertia = 1 / Math.Max(m_fPreviewRotationInertia, 0.001);
		
		BaseContainer editorSettings = GetGame().GetGameUserSettings().GetModule("SCR_EditorSettings");
		if (editorSettings)
		{
			//Init values from options
			OnUserSettingsChanged();
			GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
		}
		
		m_iHeightIndicatorColor = m_HeightIndicatorColor.PackToInt();
	}
	
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
	}
};