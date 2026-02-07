//--- For gamepad only
enum EPreviewEntityEditorOperation
{
	MOVE_HORIZONTAL,
	MOVE_VERTICAL,
	ROTATE,
};
/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_PreviewEntityEditorUIComponent: SCR_BaseEditorUIComponent
{
	const int MIN_CURSOR_DIS_TO_TRANSFORM = 10; //--- Upon clicking, how far must cursor move to initiate
	const float TRACE_DIS = 2000;
	
	[Attribute(defvalue: "30")]
	protected float m_fMoveVerticalCoef;
	
	[Attribute(defvalue: "-500")]
	protected float m_fRotationCoef;
	
	[Attribute(defvalue: "0.05")]
	protected float m_fRotationInertia;
	
	[Attribute(defvalue: "0.25")]
	protected float m_fUnsnapDuration;
	
	[Attribute(defvalue: "1", desc: "When adjusting entity height, camera will move together with the entity when above this vertical offset.\nWhen below, the camera will rotate towards the entity instead.")]
	protected float m_fMinCameraVerticalOffset;
	
	protected InputManager m_InputManagerBase;
	protected SCR_PreviewEntityEditorComponent m_PreviewEntityManager;
	protected SCR_CameraEditorComponent m_CameraManagerBase;
	protected SCR_CursorEditorUIComponent m_CursorComponentBase;
	protected SCR_HoverEditableEntityFilter m_HoverFilter;
	protected BaseWorld m_World;
	protected vector m_vClickTransformBase[4];
	protected vector m_vClickPosBase;
	protected vector m_vClickPosWorldBase;
	protected bool m_bMouseMoved;
	protected bool m_bIsRotatingTowardsCursor;
	protected SCR_EditableEntityComponent m_Target;
	protected float m_fTargetYaw;
	protected float m_fUnsnapProgress;
	protected vector m_vTransformOrigin[4];
	protected vector m_vAnglesOrigin;
	protected vector m_vAnimatedTransform[4];
	protected bool m_bIsAnimated;
	protected vector m_vTerrainNormal;
	protected EPreviewEntityEditorOperation m_Operation;
	
	protected void OnHoverChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (!m_PreviewEntityManager || !m_PreviewEntityManager.IsEditing() || m_PreviewEntityManager.IsFixedPosition() || m_bIsRotatingTowardsCursor) return;
		
		if (entitiesRemove && entitiesRemove.Count() == 1)
		{
			if (m_Target && !m_bIsRotatingTowardsCursor)
			{
				m_bIsAnimated = m_PreviewEntityManager.SetTarget(null);
			}
			m_Target = null;
		}
		if (entitiesInsert && entitiesInsert.Count() == 1)
		{
			SCR_EditableEntityComponent entity = entitiesInsert[0];
			bool isDelegate = true;
			if (m_HoverFilter) isDelegate = m_HoverFilter.IsDelegate();
			if (m_PreviewEntityManager.SetTarget(entity, isDelegate))
			{
				m_Target = entity;
				m_PreviewEntityManager.GetPreviewTransform(m_vAnimatedTransform);
			}
		}
	}
	protected void OnPreviewCreate()
	{
		m_PreviewEntityManager.GetPreviewTransformOrigin(m_vTransformOrigin);
		m_vAnglesOrigin = m_vTransformOrigin[2].VectorToAngles();
		m_bIsAnimated = false;
		SetRotationPivot();
		
		//--- Hotfix: OnEditorTransformRotationModifierUp is not called when not editing, this will reset it. ToDo: Remove
		if (!m_InputManagerBase.GetActionTriggered("EditorTransformRotateYawModifier"))
			OnEditorTransformRotationModifierUp(0, 0);
		
		//--- Activate currently hovered entity.
		//--- For example, on client, dragging a soldier on a vehicle before server callback about transformation arrives would not recognize the hover.
		set<SCR_EditableEntityComponent> hoverEntities = new set<SCR_EditableEntityComponent>();
		m_HoverFilter.GetEntities(hoverEntities);
		OnHoverChange(EEditableEntityState.HOVER, hoverEntities, null);
	}
	protected void OnPreviewDelete()
	{
		m_bIsRotatingTowardsCursor = false;
	}
	protected void OnEditorTransformRotationModifierDown(float value, EActionTrigger reason)
	{
		if (!m_CursorComponentBase) return;
		
		SetRotationPivot();
		SetClickPos(m_CursorComponentBase.GetCursorPos());		
		m_bIsRotatingTowardsCursor = true;
	}
	protected void OnEditorTransformRotationModifierUp(float value, EActionTrigger reason)
	{
		if (!m_PreviewEntityManager.IsEditing() || !m_bIsRotatingTowardsCursor) return;
		
		//m_PreviewEntityManager.SetTarget(null); //--- Don't reset, messes up restoring pre-snap transformation
		m_bIsRotatingTowardsCursor = false;
		m_PreviewEntityManager.SetIsRotating(false);
		
		//--- Return cursor back
		if (m_CursorComponentBase)
		{
			m_CursorComponentBase.SetCursorPos(m_vClickPosBase, true);
			m_vClickPosCancel = m_vClickPosBase;
		}
	}
	protected void SetRotationPivot()
	{
		vector previewTransform[4];
		if (m_PreviewEntityManager.GetPreviewTransform(previewTransform))
		{
			m_vClickTransformBase = previewTransform;
		}
		else
		{
			vector worldPos;
			if (!m_CursorComponentBase.GetCursorWorldPos(worldPos)) return;
			m_vClickTransformBase[3] = worldPos;
		}
		
		vector pos = m_vClickTransformBase[3];
		m_vTerrainNormal = SCR_Global.GetTerrainNormal(pos, m_World, !m_PreviewEntityManager.IsUnderwater());
	}
	protected void SetClickPos(vector clickPos)
	{
		m_vClickPosBase = clickPos;
		m_bMouseMoved = false;
		
		ArmaReforgerScripted game = GetGame();
		if (!game || !m_World) return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace) return;
		
		vector clickDir;
		vector cameraPos = workspace.ProjScreenToWorld(clickPos[0], clickPos[1], clickDir, m_World, -1);
		m_vClickPosWorldBase = cameraPos + clickDir * 10;
	}
	protected bool HasMouseMoved()
	{
		if (m_bMouseMoved || !m_InputManagerBase.IsUsingMouseAndKeyboard()) return true;
		
		ArmaReforgerScripted game = GetGame();
		if (!game || !m_World) return false;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace) return false;
		
		//--- Check not only if cursor moved, but also if camera moved, resulting in cursor movement in the world
		vector clickPos = workspace.ProjWorldToScreen(m_vClickPosWorldBase, m_World);
		
		m_bMouseMoved = vector.Distance(m_CursorComponentBase.GetCursorPos(), clickPos/*m_vClickPosBase*/) > MIN_CURSOR_DIS_TO_TRANSFORM;
		return m_bMouseMoved;
	}
	protected void GetCursorPos(out vector cameraPos, out vector cursorDir)
	{
		ArmaReforgerScripted game = GetGame();
		if (!game || !m_World) return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace) return;
		
		vector cursorPos = m_CursorComponentBase.GetCursorPos();
		cameraPos = workspace.ProjScreenToWorld(cursorPos[0], cursorPos[1], cursorDir, m_World, -1);
	}
	protected float GetTraceDis(vector pos, vector dir, float cameraHeight)
	{
		autoptr TraceParam trace = new TraceParam();
		trace.Start = pos;
		trace.End = trace.Start + dir;
		if (cameraHeight >= m_World.GetOceanBaseHeight())
			trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN;
		else
			trace.Flags = TraceFlags.WORLD; //--- Don't check for water intersection when under water
		trace.LayerMask = TRACE_LAYER_CAMERA;
		
		return m_World.TraceMove(trace, null);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Transformation functions
	protected void MoveHorizontalTowardsCursor(float tDelta, out vector transform[4], out bool canTransform, out EEditorTransformVertical verticalMode)
	{
		vector cameraPos, cursorDir;
		GetCursorPos(cameraPos, cursorDir);
		cursorDir *= TRACE_DIS;
		vector pos = transform[3];
		
		switch (m_PreviewEntityManager.GetVerticalMode())
		{
			case EEditorTransformVertical.TERRAIN:
			{
				canTransform = GetPreviewPosAboveTerrain(cameraPos, cursorDir, pos, verticalMode);
				break;
			}
			case EEditorTransformVertical.SEA:
			{
				canTransform = GetPreviewPosAboveSea(cameraPos, cursorDir, pos, verticalMode);
				break;
			}
			case EEditorTransformVertical.GEOMETRY:
			{
				break;
			}
		}
		transform[3] = pos;
		m_fUnsnapProgress = 0;
		m_PreviewEntityManager.SetIsMovingVertically(false);
	}
	protected bool MoveVertical(float tDelta, out vector transform[4], float moveVertical)
	{		
		//--- Gamepad move vertical
		if (m_PreviewEntityManager.CanUnsnap(moveVertical))
		{
			m_fUnsnapProgress += tDelta;
			if (m_fUnsnapProgress < m_fUnsnapDuration) return false;
		}
		else
		{
			m_fUnsnapProgress = 0;
		}
		
		//--- Apply changes
		vector pos = transform[3];
		transform[3][1] = pos[1] + moveVertical;// * tDelta;
		
		//--- Snap vertically (not when unsnapped recently)
		if (m_fUnsnapProgress == 0)
			transform[3] = m_PreviewEntityManager.SnapVertically(transform[3]);
		
		//-- Update height
		m_PreviewEntityManager.SetPreviewHeight(transform[3]);
		
		//--- Adjust camera to keep looking at preview
		SCR_ManualCamera camera;
		if (m_CameraManagerBase && m_CameraManagerBase.GetCamera(camera))
		{
			vector cameraPos = camera.GetWorldTransformAxis(3) + transform[3] - pos;
			pos = transform[3];
			
			if (cameraPos[1] > pos[1] + m_fMinCameraVerticalOffset && m_PreviewEntityManager.GetPreviewHeightAboveTerrain() > 0)
			{
				//--- Move camera with the entity when it's above the entity and the entity is above ground
				camera.SetOrigin(cameraPos);
			}
			else
			{
				//--- Rotate camera towards the entity in all other cases
				vector cameraTransform[4] = {vector.Right, vector.Up, vector.Forward, camera.GetWorldTransformAxis(3)};
				Math3D.DirectionAndUpMatrix(transform[3] - cameraTransform[3], vector.Up, cameraTransform);
				camera.SetTransform(cameraTransform);
			}
			camera.SetDirty(true);
		}
		m_PreviewEntityManager.SetIsMovingVertically(true);
		return true;
	}
	protected void MoveVerticalTowardsCursor(float tDelta, out vector transform[4])
	{
		vector cameraPos, cursorDir;
		GetCursorPos(cameraPos, cursorDir);
		
		float dis = vector.DistanceXZ(cameraPos, transform[3]);
		float angle = cursorDir.VectorToAngles()[1];
		vector tracePos = cameraPos + cursorDir * (dis / Math.Cos(angle * Math.DEG2RAD));
		transform[3][1] = tracePos[1];
		transform[3] = m_PreviewEntityManager.SnapVertically(transform[3]);
		m_PreviewEntityManager.SetPreviewHeight(transform[3]);
		m_PreviewEntityManager.SetIsMovingVertically(true);
	}
	protected void Rotate(float tDelta, out vector transform[4], float rotationValue)
	{		
		vector angles = transform[2].VectorToAngles();
		bool freeRotation = true;
		SCR_EditableEntityComponent target = m_PreviewEntityManager.GetTarget();
		if (target)
		{
			if (!RotateInSlot(target, rotationValue, angles, freeRotation)) return;
		}
		if (freeRotation)
		{
			angles -= m_vAnglesOrigin;
			angles[0] = angles[0] + rotationValue;// * tDelta;
		}
		
		vector basis[4];
		Math3D.AnglesToMatrix(angles, basis);
		Math3D.MatrixMultiply3(basis, m_vTransformOrigin, transform);
	}
	protected bool RotateInSlot(SCR_EditableEntityComponent slot, float rotationValue, out vector angles, out bool freeRotation)
	{
		if (m_fTargetYaw)
		{
			angles[0] = m_fTargetYaw;
			return true;
		}
		
		SCR_SiteSlotEntity slotEntity = SCR_SiteSlotEntity.Cast(slot.GetOwner());
		if (!slotEntity) return false;
		
		float step = slotEntity.GetRotationStep();
		if (step == -1) return false;
		if (step == 0) return true;
		
		angles[0] = angles[0] + step * rotationValue.Sign();
		m_fTargetYaw = angles[0];
		return true;
	}
	protected void RotateTowardsCursor(float tDelta, out vector transform[4])
	{
		//--- Reset when camera starts moving
		if (m_CameraManagerBase && m_CameraManagerBase.GetCamera() && m_CameraManagerBase.GetCamera().IsManualInput())
		{
			OnEditorTransformRotationModifierUp(0, 0);
			return;
		}
		
		//--- Attached to a target - use its center instead of relative click position
		SCR_EditableEntityComponent target = m_PreviewEntityManager.GetTarget();
		vector pivotTransform[4] = m_vClickTransformBase;
		if (target) target.GetTransform(pivotTransform);
		
		//--- Get intersection with terrain plane
		vector cameraPos, cursorDir;
		GetCursorPos(cameraPos, cursorDir);
		vector worldPos = SCR_Math3D.IntersectPlane(cameraPos, cursorDir * TRACE_DIS, m_vClickTransformBase[3], m_vTerrainNormal);
		float dir = (worldPos - pivotTransform[3]).VectorToAngles()[0] - m_vAnglesOrigin[0];
		
		vector basis[4];
		Math3D.AnglesToMatrix(Vector(dir, 0, 0), basis);
		Math3D.MatrixMultiply3(basis, m_vTransformOrigin, transform);
		
		m_PreviewEntityManager.SetIsRotating(true);
		
		//--- Debug
		//Shape.CreateSphere(Color.White.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, m_vClickTransformBase[3], 1);
		//Shape.CreateSphere(Color.Red.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, worldPos, 1);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Intersections
	protected bool GetPreviewPosAboveTerrain(vector cameraPos, vector cursorDir, out vector worldPos, out EEditorTransformVertical verticalMode)
	{
		//float heightASL = m_PreviewEntityManager.GetPreviewHeightAboveSea();
		float heightATL = m_PreviewEntityManager.GetPreviewHeightAboveTerrain();
		
		//--- Cursor below horizon
		vector offsetPos = cameraPos;
		//if (cameraPos[1] > heightASL)
		if (cameraPos[1] > worldPos[1]) //--- When camera is above ground, offset tracing pos by entity height to keep preview under cursor
			offsetPos -= vector.Up * heightATL;
		
		float traceDis = GetTraceDis(offsetPos, cursorDir, cameraPos[1]);
		if (traceDis != 1)
		{
			//--- Cursor on the ground: Use intersection positon
			worldPos = offsetPos + cursorDir * traceDis;
			worldPos[1] = worldPos[1] + heightATL;
			return true;
		}
		else
		{
			if (cursorDir[1] > 0) return false;
		
			//--- Cursor above horizon: Use ASL
			return GetPreviewPosAboveSea(cameraPos, cursorDir, worldPos, verticalMode);
		}
	}
	protected bool GetPreviewPosAboveSea(vector cameraPos, vector cursorDir, out vector worldPos, out EEditorTransformVertical verticalMode)
	{
		float heightASL = m_PreviewEntityManager.GetPreviewHeightAboveSea();
		
		float traceDis = 1;
		if (verticalMode != EEditorTransformVertical.SEA && cursorDir[1] < 0)
		{
			//--- Cursor points below the camera: Use ground intersection
			traceDis = GetTraceDis(cameraPos, cursorDir, cameraPos[1]);
			if (traceDis == 1) return false;
		}
		
		//--- Force ASL mode to prevent snapping to terrain
		verticalMode = EEditorTransformVertical.SEA;

		//--- Check for intersection with horizontal plane in entity's ASL height
		if (traceDis == 1)
			traceDis = Math3D.IntersectRayBox(cameraPos, cameraPos + cursorDir, Vector(-float.MAX, heightASL, -float.MAX), Vector(float.MAX, heightASL, float.MAX));
		
		//--- No plane intersection: Ignore (e.g., when camera is above the entity and cursor points at the sky)
		if (traceDis == -1) return false;
		
		worldPos = cameraPos + cursorDir * traceDis;
		worldPos[1] = heightASL; //--- Make sure ASL height is maintained
		
		return true;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Main Loop
	protected void ProcessInput(float tDelta)
	{
		if (!m_InputManagerBase || !m_PreviewEntityManager || m_PreviewEntityManager.IsFixedPosition()) return;
		
		//--- Transforming
		bool canTransform = true;
		bool instant = false;
		vector previewTransform[4];
		m_PreviewEntityManager.GetPreviewTransform(previewTransform);
		EEditorTransformVertical verticalMode = m_PreviewEntityManager.GetVerticalMode();
		
		//--- Automatic animation when restoring transformation after unsnapping from a target
		if (m_bIsAnimated)
		{
			previewTransform[0] = m_vAnimatedTransform[0];
			previewTransform[1] = m_vAnimatedTransform[1];
			previewTransform[2] = m_vAnimatedTransform[2];
		}
		
		m_InputManagerBase.ActivateContext("EditorPreviewContext");
		float moveVertical = m_InputManagerBase.GetActionValue("EditorTransformMoveVertical") * tDelta;
		float rotateYaw = m_InputManagerBase.GetActionValue("EditorTransformRotateYaw") * tDelta;
		
		//--- Snap to surface
		if (m_InputManagerBase.GetActionTriggered("EditorTransformSnapToSurface"))
			moveVertical = m_PreviewEntityManager.GetVerticalSnapDelta(previewTransform[3]);
		
		if (rotateYaw != 0 && Math.AbsFloat(rotateYaw) > Math.AbsFloat(moveVertical) && (m_Operation == EPreviewEntityEditorOperation.MOVE_HORIZONTAL || m_Operation == EPreviewEntityEditorOperation.ROTATE))
		{
			//--- Gamepad rotation
			Rotate(tDelta, previewTransform, rotateYaw * m_fRotationCoef);
			m_bIsAnimated = false;
			moveVertical = 0;
			m_Operation = EPreviewEntityEditorOperation.ROTATE;
		}
		else
		{
			m_fTargetYaw = 0;
		}
		
		if (m_bIsRotatingTowardsCursor && HasMouseMoved())
		{
			//--- Rotation towards cursor
			RotateTowardsCursor(tDelta, previewTransform);
			m_bIsAnimated = false;
		}
		else if (m_InputManagerBase.GetActionTriggered("EditorTransformMoveVerticalModifier") && HasMouseMoved())
		{
			//--- Vertical movement towards cursor
			MoveVerticalTowardsCursor(tDelta, previewTransform);
		}
		else if (moveVertical != 0 && (m_Operation == EPreviewEntityEditorOperation.MOVE_HORIZONTAL || m_Operation == EPreviewEntityEditorOperation.MOVE_VERTICAL))
		{
			//--- Vertical movement
			instant = MoveVertical(tDelta, previewTransform, moveVertical * m_fMoveVerticalCoef);
			m_Operation = EPreviewEntityEditorOperation.MOVE_VERTICAL;
		}
		else
		{
			//--- Horizontal movement towards cursor
			MoveHorizontalTowardsCursor(tDelta, previewTransform, canTransform, verticalMode);
			m_Operation = EPreviewEntityEditorOperation.MOVE_HORIZONTAL;
		}
		
		//--- Apply
		if (canTransform)
			m_PreviewEntityManager.SetPreviewTransform(previewTransform, tDelta, instant, verticalMode);
		else
			m_PreviewEntityManager.ResetPreviewTransform();
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	override void HandlerAttachedScripted(Widget w)
	{
		MenuRootComponent root = GetRootComponent();
		if (root)
		{
			m_CursorComponentBase = SCR_CursorEditorUIComponent.Cast(root.FindComponent(SCR_CursorEditorUIComponent, true));
		}
		if (!m_CursorComponentBase) return;
		
		m_PreviewEntityManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, true));
		if (!m_PreviewEntityManager) return;
		
		m_PreviewEntityManager.GetOnPreviewCreate().Insert(OnPreviewCreate);
		m_PreviewEntityManager.GetOnPreviewDelete().Insert(OnPreviewDelete);
		
		m_CameraManagerBase = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent, true));
		if (!m_CameraManagerBase) return;
		
		m_HoverFilter = SCR_HoverEditableEntityFilter.Cast(SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.HOVER));
		if (m_HoverFilter)
			m_HoverFilter.GetOnChanged().Insert(OnHoverChange);
		
		ArmaReforgerScripted game = GetGame();
		if (game)
		{
			m_World = game.GetWorld();
			
			m_InputManagerBase = game.GetInputManager();
			if (m_InputManagerBase)
			{
				m_InputManagerBase.AddActionListener("EditorTransformRotateYawModifier", EActionTrigger.DOWN, OnEditorTransformRotationModifierDown);
				m_InputManagerBase.AddActionListener("EditorTransformRotateYawModifier", EActionTrigger.UP, OnEditorTransformRotationModifierUp);
			}
		}
				
		m_fRotationInertia = 1 / Math.Max(m_fRotationInertia, 0.001);
	}
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_PreviewEntityManager)
		{
			m_PreviewEntityManager.GetOnPreviewCreate().Remove(OnPreviewCreate);
			m_PreviewEntityManager.GetOnPreviewDelete().Remove(OnPreviewDelete);
		}
		
		if (m_HoverFilter)
			m_HoverFilter.GetOnChanged().Remove(OnHoverChange);
		
		if (m_InputManagerBase)
		{
			m_InputManagerBase.RemoveActionListener("EditorTransformRotateYawModifier", EActionTrigger.DOWN, OnEditorTransformRotationModifierDown);
			m_InputManagerBase.RemoveActionListener("EditorTransformRotateYawModifier", EActionTrigger.UP, OnEditorTransformRotationModifierUp);
		}
	}
};