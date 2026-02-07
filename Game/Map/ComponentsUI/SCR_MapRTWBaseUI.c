//------------------------------------------------------------------------------------------------
class SCR_MapRTWBaseUI : SCR_MapUIBaseComponent
{
	const string BASE_WORLD_TYPE = "Preview";
	
	// configuration
	protected string WIDGET_NAME;
	protected string RT_WIDGET_NAME;
	protected string WORLD_RESOURCE = "{88ABCDC0EEC969DF}Prefabs/World/PreviewWorld/MapCompassWorld.et";
	protected string WORLD_NAME;
	protected vector m_vPrefabPos;
	protected vector m_vCameraPos;
	protected vector m_vCameraAngle;
	
	
	protected bool m_bIsVisible;				// visibility flag
	protected bool m_bWantedVisible;			// holds wanted visiblity state after close
	protected bool m_bIsDragged = false;		// widget is being dragged
	protected float m_fPosX, m_fPosY;			// widget position
	
	protected GenericEntity m_RTEntity;
	protected ref SharedItemRef m_RTWorld;
	protected SCR_MapToolEntry m_ToolMenuEntry;	// tool menu entry
	
	// Widgets
	protected Widget m_wFrame;						// parent frame
	protected RenderTargetWidget m_wRenderTarget;	// RenderTargetWidget
	
	//------------------------------------------------------------------------------------------------
	//! Set widget names
	protected void SetWidgetNames()
	{
		WIDGET_NAME = "";
		RT_WIDGET_NAME = "";
		WORLD_NAME = "MapUIWorld";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize RT camera and prefab positon
	protected void InitPositionVectors()
	{
		m_vPrefabPos = "0 0 0";
		m_vCameraPos = "0 0.3 0";
		m_vCameraAngle = "0 -90 0";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get prefab resource for display
	//! \return prefab resource
	protected string GetPrefabResource()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Visibility toggle
	protected void ToggleVisible()
	{
		if (!m_bIsVisible)
			SetVisible(true);
		else
		{
			SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set visibility
	//! \param visible is true/false switch
	protected void SetVisible(bool visible)
	{
		if (visible)
		{			
			// RTW preview world
			if (!SetupRTWorld())
				return;
			
			if (!SpawnPrefab())
				return;
						
			m_bIsVisible = true;
			m_wFrame.SetVisible(true);
			m_wRenderTarget.SetVisible(true);
			FrameSlot.SetPos(m_wFrame, m_fPosX, m_fPosY);
			
		}
		else 
		{
			m_bIsVisible = false;
			m_wRenderTarget.SetVisible(false);
				
			delete m_RTEntity; // proc anims dont work without this not being refreshed atm
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup preview world
	//! \return Returns true if successful
	protected bool SetupRTWorld()
	{
		// create new empty base world
		if (!m_RTWorld || !m_RTWorld.IsValid())
		{
			m_RTWorld = BaseWorld.CreateWorld(BASE_WORLD_TYPE, WORLD_NAME);
			if (!m_RTWorld)
				return false;
		}
		
		BaseWorld previewWorld = m_RTWorld.GetRef();
		if (!previewWorld)
			return false;
			
		m_wRenderTarget.SetWorld( previewWorld, 0 );
			
		// spawn preview GenericWorldEntity
		Resource rsc = Resource.Load(WORLD_RESOURCE);
		if (rsc.IsValid())
			GetGame().SpawnEntityPrefabLocal(rsc, previewWorld);
			
		// default cam settings
		previewWorld.SetCameraType(0, CameraType.PERSPECTIVE);
		previewWorld.SetCameraNearPlane(0, 0.001);
		previewWorld.SetCameraFarPlane(0, 50);
		previewWorld.SetCameraVerticalFOV(0, 30);
		
						
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Spawn prefab in the preview world
	//! \return Returns true if successful
	protected bool SpawnPrefab()
	{
		string rscStr = GetPrefabResource();
		if (rscStr == string.Empty)
			return false;
		
		Resource rscItem = Resource.Load(rscStr);
		if (rscItem.IsValid())
		{
			BaseWorld world = m_RTWorld.GetRef();
			if (!world)
				return false;
			
			if (!m_RTEntity)
			{			
				m_RTEntity = GenericEntity.Cast( GetGame().SpawnEntityPrefabLocal(rscItem, world) );
				if (!m_RTEntity) 
					return false;
				
				InitPositionVectors();
				
				m_RTEntity.SetOrigin(m_vPrefabPos);
				
				BaseWorld previewWorld = m_RTWorld.GetRef();
				if (!previewWorld)
					return false;
				
				previewWorld.SetCamera(0, m_vCameraPos, m_vCameraAngle);
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Drag compass event
	protected void OnDragWidget(Widget widget)
	{
		if (widget == m_wFrame)
			m_bIsDragged = true;
		else 
			m_bIsDragged = false;
	}
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{		
		super.OnMapOpen(config);
		
		SetWidgetNames();
		
		// refresh widgets
		m_wFrame = m_RootWidget.FindAnyWidget(WIDGET_NAME);
		m_wRenderTarget = RenderTargetWidget.Cast(m_RootWidget.FindAnyWidget(RT_WIDGET_NAME));
				
		// if dragging available, add callback
		if ( SCR_MapDragComponent.Cast(m_MapEntity.GetMapUIComponent(SCR_MapDragComponent)) )
			SCR_MapDragComponent.GetOnDragWidgetInvoker().Insert(OnDragWidget);
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{				
		delete m_RTEntity;
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapRTWBaseUI()
	{
		m_bHookToRoot = true;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapRTWBaseUI()
	{
		m_RTWorld = null;
		m_bIsDragged = false;
	}

};
