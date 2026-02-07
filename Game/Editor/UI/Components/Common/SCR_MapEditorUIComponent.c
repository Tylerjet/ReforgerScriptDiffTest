class SCR_MapEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected SCR_MapEntity m_MapEntity;
	protected InputManager m_InputManager;
	
	protected SCR_ManualCamera m_EditorCamera;
	protected CanvasWidget m_CameraLineCanvas;
	
	protected SCR_EditorManagerCore m_EditorCore;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected SCR_MapEditorComponent m_EditorMapManager;
	
	protected ref Color m_CameraIconColor = Color.White;
	protected Widget m_MapWidget;
	
	protected ResourceName m_EditorMapConfigPrefab;
	protected ref MapConfiguration m_MapConfigEditor;
	protected ref MapItem m_CameraIcon;
	protected ref array<ref CanvasWidgetCommand> m_MapDrawCommands = {};
	protected bool m_bIsFirstTimeOpened = true;
	
	void ToggleMap(bool show, ResourceName mapConfigPrefab)
	{
		if (mapConfigPrefab.IsEmpty())
		{
			return;
		}
		
		if (show)
		{
			m_MapConfigEditor = m_MapEntity.SetupMapConfig(EMapEntityMode.EDITOR, mapConfigPrefab, m_MapWidget);
			m_MapEntity.OpenMap(m_MapConfigEditor);
		}
		else
		{
			m_MapEntity.CloseMap();
		}
		
		if (m_EditorCamera)
		{
			m_EditorCamera.SetInputEnabled(!show);
		}
	}
	
	protected void OnMapInit(MapConfiguration config)
	{
		if (IsConfigEditor(config))
		{
			m_MapWidget.SetVisible(true);
		}
	}
	
	protected void OnMapOpen(MapConfiguration config)
	{
		if (IsConfigEditor(config))
		{
			if (m_bIsFirstTimeOpened)
			{
				m_bIsFirstTimeOpened = false;
				m_MapEntity.ZoomOut();
			}
		}
	}
	
	protected void OnMapClose(MapConfiguration config)
	{
		if (IsConfigEditor(config))
		{
			m_MapWidget.SetVisible(false);
		}
	}
	
	bool IsEditorMapOpen()
	{
		return m_MapEntity && m_MapEntity.IsOpen() && IsConfigEditor(m_MapEntity.GetMapConfig());
	}
	
	protected bool IsConfigEditor(MapConfiguration config)
	{
		return config && config.MapEntityMode == EMapEntityMode.EDITOR;
	}
	
	protected void OnEditorModeChange(SCR_EditorModeEntity newModeEntity, SCR_EditorModeEntity oldModeEntity)
	{
		SCR_CameraEditorComponent cameraComponent;
		if (oldModeEntity)
		{
			cameraComponent = SCR_CameraEditorComponent.Cast(oldModeEntity.FindComponent(SCR_CameraEditorComponent));
			if (cameraComponent)
			{
				cameraComponent.GetOnCameraCreate().Remove(OnEditorCameraCreate);
			}
		}
		if (newModeEntity)
		{
			cameraComponent = SCR_CameraEditorComponent.Cast(newModeEntity.FindComponent(SCR_CameraEditorComponent));
			if (cameraComponent)
			{
				cameraComponent.GetOnCameraCreate().Insert(OnEditorCameraCreate);
				OnEditorCameraCreate(cameraComponent.GetCamera());
			}
			SetCameraIconColor(newModeEntity.GetModeType());
		}
	}
	
	protected void SetCameraIconColor(EEditorMode mode)
	{
		if (!m_EditorCore)
		{
			return;
		}
		
		SCR_EditorModeUIInfo modeUiInfo = m_EditorCore.GetDefaultModeInfo(mode);
		if (modeUiInfo)
		{
			m_CameraIconColor = modeUiInfo.GetModeColor();
		}
		else if (m_EditorCore.GetDefaultModeInfo(EEditorMode.EDIT))
		{
			m_CameraIconColor = m_EditorCore.GetDefaultModeInfo(EEditorMode.EDIT).GetModeColor();
		}
		
		if (m_CameraIcon)
		{
			MapDescriptorProps cameraIconProps = m_CameraIcon.GetProps();
			cameraIconProps.SetFrontColor(m_CameraIconColor);
			cameraIconProps.Activate(true);
			m_CameraIcon.SetProps(cameraIconProps);
		}
	}
	
	protected void OnEditorCameraCreate(SCR_ManualCamera editorCamera)
	{
		if (!m_MapEntity || !editorCamera)
		{
			return;
		}
		
		m_EditorCamera = editorCamera;		
		
		if (m_EditorCamera)
		{
			m_EditorCamera.SetInputEnabled(!IsEditorMapOpen());
		}
		
		//=== Hide editor camera in map until proper implementation
		/* 
		if (!m_CameraIcon)
		{
			m_CameraIcon = m_MapEntity.CreateCustomMapItem();
			m_CameraIcon.SetBaseType(EMapDescriptorType.MDT_ICON);
			m_CameraIcon.SetImageDef("editor-camera");
			m_CameraIcon.SetVisible(false);
			m_CameraIcon.SetFactionIndex(0);
			
			MapDescriptorProps props = m_CameraIcon.GetProps();
			props.SetFrontColor(m_CameraIconColor);
			props.SetIconSize(32, 0.5, 0.5);
			m_CameraIcon.SetProps(props);
			props.Activate(true);
			m_CameraIcon.SetProps(props);
		}
		*/
	}
	
	protected void OnMenuUpdate()
	{
		if (IsEditorMapOpen())
		{
			//! Camera icon
			if(m_EditorCamera && m_CameraIcon)
			{
				vector cameraMatrix[4];
				m_EditorCamera.GetTransform(cameraMatrix);
				m_CameraIcon.SetVisible(true);
				m_CameraIcon.SetPos(cameraMatrix[3][0], cameraMatrix[3][2]);
				vector angles = Math3D.MatrixToAngles(cameraMatrix);
				m_CameraIcon.SetAngle(angles[0]);
				m_CameraIcon.SetEditor(true);
				
				float fov = m_EditorCamera.GetVerticalFOV();
				DrawCameraLines(cameraMatrix, angles, fov);
			}
			else if (m_CameraIcon && m_CameraIcon.IsVisible())
			{
				m_CameraIcon.SetVisible(false);
			}
			
			m_InputManager.ActivateContext("MapContext");
		}
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager) return;
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity) return;
		
		m_MapEntity.GetOnMapInit().Insert(OnMapInit);
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (!m_EditorManager)
			return;
		
		m_EditorManager.GetOnModeChange().Insert(OnEditorModeChange);
		
		m_EditorMapManager = SCR_MapEditorComponent.Cast(SCR_MapEditorComponent.GetInstance(SCR_MapEditorComponent));
		if (!m_EditorMapManager)
			return;
		
		m_EditorMapManager.SetMapHandler(this);
		m_EditorMapConfigPrefab = m_EditorMapManager.GetMapConfigPrefab();
		
		m_EditorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));

		GetMenu().GetOnMenuUpdate().Insert(OnMenuUpdate);
		
		m_MapWidget = w;
		
		m_CameraLineCanvas = CanvasWidget.Cast(m_MapWidget.FindWidget("EditorCameraLines"));
		
		OnEditorModeChange(m_EditorManager.GetCurrentModeEntity(), null);
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		super.HandlerDeattached(w);
		
		if (IsEditorMapOpen())
		{
			ToggleMap(false, m_EditorMapConfigPrefab);
		}
		
		if (m_CameraIcon)
		{
			m_CameraIcon.Recycle();
		}
		
		if (GetMenu())
		{
			GetMenu().GetOnMenuUpdate().Remove(OnMenuUpdate);
		}
	}
	
	protected void DrawCameraLines(vector cameraMatrix[], vector angles, float fov)
	{
		if (!m_CameraLineCanvas)
		{
			return;
		}
		
		//TODO trace and draw lines based on screen edges
	}
};