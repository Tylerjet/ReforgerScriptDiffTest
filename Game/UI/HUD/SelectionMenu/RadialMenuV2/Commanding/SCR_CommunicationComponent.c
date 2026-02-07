[ComponentEditorProps(category: "GameScripted/HUD", description: "Commading functionality component", color: "0 0 255 255")]
class SCR_CommunicationComponentClass: SCR_MailboxComponentClass
{
};

enum EPingType
{
	NONE,
	INFO,
	ENEMY,
	MOVE,
};

enum ECommandType
{
	NONE,
	MOVE_COMMAND,
	ATTACK_COMMAND,
	STOP_COMMAND,
	FOLLOW_COMMAND,
};

[BaseContainerProps()]
class SCR_CommunicationContent
{
	[Attribute()]
	string name;
	
	[Attribute()]
	string description;
	
	[Attribute()]
	int page;
	
	[Attribute()]
	ResourceName imageSet;
	
	[Attribute()]
	string imageName;
	
	[Attribute("{121C45A1F59DC1AF}UI/layouts/Common/RadialMenu/RadialEntryElement.layout")]
	ResourceName layout;
	
	[Attribute()]
	bool showIn3D;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EPingType))]	
	EPingType pingType;
	
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECommandType))]	
	ECommandType commandType;
};

// Prototype of communication component
// It can be created outside SCR_MailboxComponent with separate ping system
class SCR_CommunicationComponent : SCR_MailboxComponent
{
	[Attribute("PlayerGroup")]
	protected string m_sGroupName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_MoveWP;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_AttackWP;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_FollowWP;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "et")]
	protected ResourceName m_StopWP;
	
	[Attribute()]
	protected ref array<ref SCR_CommunicationContent> m_aCommands;
	
	[Attribute("50")]
	protected float m_fPingBroadcastDistance;
	
	[Attribute("5")]
	protected float m_fPingLifetime;
	
	[Attribute("0.2")]
	protected float m_fFadeInTime;

	[Attribute("1")]
	protected float m_fFadeOutTime;

	[Attribute("{A1A4D9A0D9219C92}UI/layouts/HUD/Commanding/BasicPing.layout", UIWidgets.ResourceNamePicker, "Ping layout", "layout")]
	protected ResourceName m_PingLayout;

	[Attribute("{1F0A6C9C19E131C6}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_PingImageSet;

	protected SCR_RadialMenuHandler m_RadialMenu;
	protected float m_fFadeInRate;
	protected IEntity m_Owner;
	protected bool m_bIsMenuOpen;
	protected ref array<ref Tuple3<Widget, float, vector>> m_aActivePings = {};
	protected vector m_vPingPosition;
	protected BaseWorld m_World;
	protected CameraManager m_CameraManager;
	protected Widget m_wRoot;
	protected WorkspaceWidget m_Workspace;
	protected SCR_AIGroup m_Group;
	protected AIWaypoint m_CurrentWaypoint;
	
	protected ref array<AIAgent> m_aTargets = {};

	protected const ResourceName m_DummyRoot = "{F70ADE861966D34F}UI/layouts/Common/DummyRoot.layout";
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		m_Owner = owner;

		if (m_fFadeInTime <= 0)
			m_fFadeInRate = 1000;
		else
			m_fFadeInRate = 1 / m_fFadeInTime;
		
		// Convert fadeout time to ms
		m_fFadeOutTime *= 1000;
		m_fPingLifetime *= 1000;
		
		m_World = GetGame().GetWorld();
		m_CameraManager = GetGame().GetCameraManager();
		m_Workspace = GetGame().GetWorkspace();
		m_wRoot = GetRootWidget();
		
		GetGame().GetInputManager().AddActionListener("TacticalPing", EActionTrigger.DOWN, Action_BasicPing);
		SetupRadialMenu();
	}
	
	//------------------------------------------------------------------------------------------
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;
		
		UpdatePings(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------
	private void UpdatePings(float timeSlice)
	{
		// Process active pings
		if (m_aActivePings.Count() == 0)
			return;
		
		Widget w;
		float timeLeft;
		float worldTime = m_World.GetWorldTime();
		float targetOpacity;
		vector pos;
		vector mat[4];
		vector direction;
		
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		if (!camera)
			return;
		
		camera.GetWorldTransform(mat);
		vector cameraDirection = mat[2].Normalized();
		
		for (int i = m_aActivePings.Count() -1; i >= 0; i--)
		{
			w = m_aActivePings[i].param1;
			timeLeft = m_aActivePings[i].param2 - worldTime;
			pos = m_aActivePings[i].param3;
			direction = vector.Direction(pos, camera.GetOrigin());
			float dotProduct = vector.Dot(direction, cameraDirection);
			
			if (timeLeft < m_fFadeOutTime && timeLeft > 0)
			{
				targetOpacity = timeLeft / m_fFadeOutTime;
				w.SetOpacity(targetOpacity);
			}
			
			// Direction
			if (dotProduct > 0)
			{
				w.SetOpacity(0);
			}
			else if (timeLeft > 0)
			{
				pos = GetGame().GetWorkspace().ProjWorldToScreen(pos, m_World);
				FrameSlot.SetPos(w, pos[0], pos[1]);
			}
			else
			{
				w.RemoveFromHierarchy();
				m_aActivePings.Remove(i);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupRadialMenu()
	{
		m_RadialMenu = GetRadialMenuHandler();
		if (!m_RadialMenu)
		{
			// UNHACK: Backup - if menu is not created in time
			GetGame().GetCallqueue().CallLater(SetupRadialMenu, 100);
			return;
		}
		
		// Setup radial menu
		foreach (SCR_CommunicationContent content : m_aCommands)
		{
			RegisterEntry(content);
		}
		
		m_RadialMenu.m_OnActionPerformed.Insert(OnRadialMenuPerformed);
		m_RadialMenu.m_OnCreated.Remove(SetupRadialMenu);
		ConnectToGroup(m_sGroupName);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetPlayerGroup()
	{
		if (!m_Group && m_World)
			m_Group = SCR_AIGroup.Cast(m_World.FindEntityByName(m_sGroupName));

		return m_Group;
	}
	
	//------------------------------------------------------------------------------------------------
	void ConnectToGroup(string name)
	{
		if (!m_World)
		{
			m_World = GetGame().GetWorld();
			if (!m_World)
				return;
		}
		
		m_Group = SCR_AIGroup.Cast(m_World.FindEntityByName(name));
		EnableRadialMenu(m_Group != null);
	}
	
	//------------------------------------------------------------------------------------------------
	void ConnectToGroup(SCR_AIGroup group)
	{
		m_Group = group;
		EnableRadialMenu(m_Group != null);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableRadialMenu(bool enable)
	{
		if (!m_RadialMenu)
			return;
		
		SCR_RadialMenuInteractions interactions = m_RadialMenu.GetRadialMenuInteraction();
		if (!interactions)
			return;

		interactions.SetCanOpenMenu(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRadialMenuPerformed(BaseSelectionMenuEntry entry, int i)
	{
		if (i < 0 || i >= m_aCommands.Count())
			return;
		
		SCR_CommunicationContent item = m_aCommands[i];
		vector pos, dummyPos;
		bool positionFound = GetAimPosition(pos);
		IEntity target = m_CameraManager.CurrentCamera().GetCursorTargetWithPosition(dummyPos);
		
		if (item.showIn3D && item.pingType != EPingType.NONE && positionFound)
			Action_Ping(item.pingType);

		if (GetPlayerGroup())
			CreateOrder(item.commandType, pos, target);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateOrder(ECommandType type, vector aimPosition, IEntity target)
	{
		if (type == ECommandType.NONE)
			return;
		
		if (type == ECommandType.MOVE_COMMAND)
			SendMoveCommand(aimPosition);
		else if (type == ECommandType.ATTACK_COMMAND)
			SendAttackCommand(target, aimPosition);
		else if (type == ECommandType.STOP_COMMAND)
			SendStopCommand();
		else if (type == ECommandType.FOLLOW_COMMAND)
			SendFollowCommand();
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearCurrentWaypoint()
	{
		if (m_CurrentWaypoint)
			m_Group.RemoveWaypoint(m_CurrentWaypoint);
	}	
	
	//------------------------------------------------------------------------------------------------
	AIWaypoint CreateWaypoint(ResourceName path)
	{
		Resource resource = Resource.Load(path);
		if (!resource)
			return null;
		
		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource));
		if (!wp)
			return null;
		
		return wp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SendMoveCommand(vector aimPosition)
	{
		if (!m_Group || aimPosition == vector.Zero)
			return;
		
		AIWaypoint wp = CreateWaypoint(m_MoveWP);
		if (!wp)
			return;
		
		ClearCurrentWaypoint();
		wp.SetOrigin(aimPosition);
		m_Group.AddWaypointToGroup(wp);
		m_CurrentWaypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SendAttackCommand(IEntity target, vector aimPosition)
	{
		if (!m_Group || !target)
			return;
		
		string waypointType = m_MoveWP;
		if (target)
			waypointType = m_AttackWP;
		
		AIWaypoint wp = CreateWaypoint(waypointType);
		if (!wp)
			return;
		
		SCR_EntityWaypoint entityWaypoint = SCR_EntityWaypoint.Cast(wp);
		if (entityWaypoint)
			entityWaypoint.SetEntity(target);
		
		ClearCurrentWaypoint();
		wp.SetOrigin(aimPosition);
		m_Group.AddWaypointToGroup(wp);
		m_CurrentWaypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SendStopCommand()
	{
		if (!m_Group)
			return;
		
		AIWaypoint wp = CreateWaypoint(m_StopWP);
		if (!wp)
			return;

		// Get group leader position
		IEntity ent = m_Group.GetLeaderEntity();
		if (!ent)
			return;
		
		ClearCurrentWaypoint();
		wp.SetOrigin(ent.GetOrigin());
		m_Group.AddWaypointToGroup(wp);
		m_CurrentWaypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SendFollowCommand()
	{
		AIWaypoint wp = CreateWaypoint(m_FollowWP);
		if (!wp)
			return;
	
		SCR_EntityWaypoint entityWaypoint = SCR_EntityWaypoint.Cast(wp);
		if (!entityWaypoint)
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		entityWaypoint.SetEntity(pc.GetControlledEntity());
		
		ClearCurrentWaypoint();
		m_Group.AddWaypointToGroup(wp);
		m_CurrentWaypoint = wp;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetPlayerPosition()
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return vector.Zero;
		
		IEntity ent = pc.GetControlledEntity();
		if (!ent)
			return vector.Zero;
		
		return ent.GetOrigin();
	}
	
	// Setup radial menu
	//------------------------------------------------------------------------------------------------
	void RegisterEntry(SCR_CommunicationContent content)
	{
		ScriptedSelectionMenuEntry entry = new ScriptedSelectionMenuEntry;
		entry.SetName(content.name);
		m_RadialMenu.AddEntry(entry, content.page);
	
		SetupEntryLayout(entry, content);
	}
	
	// Set properties of entry layout 
	//------------------------------------------------------------------------------------------------
	protected void SetupEntryLayout(ScriptedSelectionMenuEntry entry, SCR_CommunicationContent content)
	{
		if (!entry)
			return;
		
		SCR_SelectionEntryWidgetComponent entryWidgetComp = new SCR_SelectionEntryWidgetComponent;
		if (!entryWidgetComp)
			return;
		
		// Set component properties 
		entryWidgetComp.SetTextureData(content.imageSet, content.imageName);
		SCR_RadialMenuVisuals visuals = m_RadialMenu.GetRadialMenuVisuals();
		if (visuals)
			visuals.InsertEntryWidget(entryWidgetComp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_RadialMenuHandler GetRadialMenuHandler()
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return null;

		array<Managed> ents = {};
		controller.FindComponents(SCR_RadialMenuComponent, ents);
		foreach (Managed ent : ents)
		{
			SCR_RadialMenuComponent comp = SCR_RadialMenuComponent.Cast(ent);
			if (comp.m_sInput_Toggle == "OpenCommandMenu")
				return comp.m_pRadialMenu;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------
	void SetPingTexture(Widget w, EPingType type)
	{
		string texture;
		switch (type)
		{
			case EPingType.INFO:
				texture = "cancel";
				break;
			case EPingType.ENEMY:
				texture = "kill";
				break;
			case EPingType.MOVE:
				texture = "down";
				break;
		}

		if (texture == string.Empty)
			return;

		ImageWidget img = ImageWidget.Cast(w);
		if (!img)
			return;

		img.LoadImageFromSet(0, m_PingImageSet, texture);
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget GetRootWidget()
	{
		if (m_wRoot)
			return m_wRoot;
		
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (hudManager)
			m_wRoot = hudManager.CreateLayout(m_DummyRoot, EHudLayers.BACKGROUND, -1); // This should be even covered by blood
		else
			GetGame().GetCallqueue().CallLater(GetRootWidget);
		
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	bool GetAimPosition(out vector outPos = vector.Zero)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
		
		ChimeraCharacter ent = ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!ent)
			return false;
		
		vector outDir;
		float w = m_Workspace.DPIUnscale(m_Workspace.GetWidth() * 0.5);
		float h = m_Workspace.DPIUnscale(m_Workspace.GetHeight() * 0.5);
		vector startPos = m_Workspace.ProjScreenToWorld(w, h, outDir, m_World);
		outDir *= 10000;
	
		autoptr TraceParam trace = new TraceParam();
		trace.LayerMask = EPhysicsLayerDefs.Camera;
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;

		if (ent.IsInVehicle())
		{
			ref array<IEntity> excludeArray = {};
			// TEMP: Exclude list of vehicle entities does not work 
			// Exclude just character and move trace few meters forward from camera
			// GetVehicleEntities(ent, excludeArray);
			excludeArray.Insert(ent);
			trace.ExcludeArray = excludeArray;
			
			// Trace 2 meters from the camera, so encounter 
			trace.Start = startPos + outDir.Normalized() * 2;
		}
		else
		{
			trace.Exclude = ent;
		}
		
		float traceDis = m_World.TraceMove(trace, null);
		if (traceDis == 1) 
			return false;
		outPos = startPos + outDir * traceDis;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_BasicPing()
	{
		Action_Ping(EPingType.INFO);
	}
	
	//------------------------------------------------------------------------------------------------
	void Action_Ping(EPingType type)
	{
		if (!m_CameraManager || !m_World)
			return;
		
		vector pos;
		bool positionFound = GetAimPosition(pos);
		if (!positionFound)
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		SCR_ChimeraCharacter ent = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!ent)
			return;

		// Remove your old ping
		if (m_vPingPosition != vector.Zero)
			Rpc(AskRemovePing, m_vPingPosition);

		m_vPingPosition = pos;
		Rpc(AskShowPing, type, ent.GetOrigin(), pos, ent.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void AskShowPing(EPingType type, vector pingOrigin, vector position, string factionKey)
	{
		DoShowPing(type, pingOrigin, position, factionKey);
		Rpc(DoShowPing, type, pingOrigin, position, factionKey);
	}
	
	//------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoShowPing(EPingType type, vector pingOrigin, vector position, string factionKey)
	{
		if (!m_World || !m_aActivePings || !m_wRoot)
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		SCR_ChimeraCharacter ent = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!ent)
			return;
		
		if (ent.GetFactionKey() != factionKey)
			return;
		
		float distanceSQ = vector.DistanceSq(ent.GetOrigin(), pingOrigin);
		if (distanceSQ > m_fPingBroadcastDistance * m_fPingBroadcastDistance)
			return;
		
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_PingLayout, m_wRoot);
		if (!w)
			return;
		
		SetPingTexture(w, type);
		w.SetOpacity(0);
		WidgetAnimator.PlayAnimation(w, WidgetAnimationType.Opacity, 1, m_fFadeInRate);

		Tuple3<Widget, float, vector> data = new Tuple3<Widget, float, vector>(w, m_World.GetWorldTime() + m_fPingLifetime, position);
		m_aActivePings.Insert(data);
	}
	
	//------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void AskRemovePing(vector position)
	{
		DoRemovePing(position);
		Rpc(DoRemovePing, position);
	}
	
	//------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoRemovePing(vector position)
	{
		foreach (Tuple3<Widget, float, vector> tuple : m_aActivePings)
		{
			if (tuple.param3 != position)
				continue;
			
			tuple.param1.RemoveFromHierarchy();
			m_aActivePings.RemoveItem(tuple);
			return;
		}
	}
};