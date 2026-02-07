[ComponentEditorProps(category: "GameScripted/AIComponent", description: "Scripted Player Mailbox component", icon: HYBRID_COMPONENT_ICON)]
class SCR_PlayerMailboxComponentClass: SCR_MailboxComponentClass
{
};

//------------------------------------------------------------------------------------------
class SCR_PlayerMailboxComponent : SCR_MailboxComponent
{
	[Attribute("", UIWidgets.Object)]
	protected ref SCR_RadialMenu m_pCommandingRadialMenu;
	
	[Attribute("CommandingContext", UIWidgets.EditBox)]
	protected string m_sInputContextName;
	
	[Attribute("OpenCommandingMenu", UIWidgets.EditBox)]
	protected string m_sInputActionName;
	
	/*
	UNDEFINED = 0,
	TARGET = 1,
	TARGET_LOST = 2,
	MOVE = 3,
	NO_AMMO = 4,
	WOUNDED = 5,
	FOUND_CORPSE = 6,
	UNDER_FIRE = 7,
	REQ_RETREAT = 8,
	REQ_AMMO = 9,
	COMMAND_DONE = 10,
	COMMAND_FAILED = 11,
	COMMAND_CONFIRMED = 12,
	*/
	SCR_AIMessageSelectionMenuEntry CreateEntry(AIMessage mess, AIAgent receiver = null)
	{
		ref auto pDummy = new ref SCR_AIMessageSelectionMenuEntry(mess);
		pDummy.SetParentComms(this, receiver);
		return pDummy;
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateReportUnderFire(vector pos, AIAgent receiver = null)
	{
		SCR_AIMessage_UnderFire mess = SCR_AIMessage_UnderFire.Cast(CreateMessage(SCR_AIMessage_UnderFire));		
		mess.SetText("Under fire!");
		mess.m_MessageType = 7;
		mess.m_vPosition = pos;
		return CreateEntry(mess, receiver);
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateReportDeadFriendly(vector pos, IEntity target, AIAgent receiver = null)
	{
		SCR_AIMessage_FoundCorpse mess = SCR_AIMessage_FoundCorpse.Cast(CreateMessage(SCR_AIMessage_FoundCorpse));
		if (target && !target.IsDeleted())
		{
			SCR_AIMessage_Wounded mess1 = SCR_AIMessage_Wounded.Cast(CreateMessage(SCR_AIMessage_Wounded));
			mess1.m_WoundedEntity = target;
			mess1.SetText("Wounded!");
			mess1.m_MessageType = 5;
			mess = mess1;
		}
		else
		{
			mess.SetText("Casualty!");			
			mess.m_MessageType = 6;			
		}
		mess.m_vPosition = pos;
		return CreateEntry(mess, receiver);
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateDangerEvent(EAIDangerEventType type, string text, vector pos, IEntity target, AIAgent receiver = null)
	{
		AIDangerEvent mess = AIDangerEvent.Cast(CreateMessage(AIDangerEvent));
		mess.SetPosition(pos);
		mess.SetObject(target);
		mess.SetDangerType(type);
		mess.SetText(text);
		return CreateEntry(mess, receiver);
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateReportEnemy(vector pos, IEntity target, AIAgent receiver = null)
	{
		SCR_AIMessage_Contact mess = SCR_AIMessage_Contact.Cast(CreateMessage(SCR_AIMessage_Contact));		
		mess.SetText("Enemy!");
		mess.m_LastSeenPosition = pos;
		mess.m_MessageType = 1;
		mess.m_Target = target;
		return CreateEntry(mess, receiver);
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateReportNoAmmo(vector pos, IEntity target, AIAgent receiver = null)
	{
		SCR_AIMessage_NoAmmo mess = SCR_AIMessage_NoAmmo.Cast(CreateMessage(SCR_AIMessage_NoAmmo));		
		mess.SetText("No ammo");
		//mess.m_Vector = pos; <- not needed???
		mess.m_MessageType = 4;
		//mess.m_Target = target; <- not needed??? sender?
		return CreateEntry(mess, receiver);
	}
	
	
	ref SCR_AIMessageSelectionMenuEntry CreateMoveOrder(vector pos, AIAgent receiver = null)
	{
		SCR_AIMessage_Move mess = SCR_AIMessage_Move.Cast(CreateMessage(SCR_AIMessage_Move));		
		mess.SetText("Move");
		mess.m_MovePosition = pos;
		mess.m_MessageType = 3;
		return CreateEntry(mess, receiver);
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateStopOrder(AIAgent receiver = null)
	{
		SCR_AIMessageGoal mess = SCR_AIMessageGoal.Cast(CreateMessage(SCR_AIMessageGoal));		
		mess.SetText("Stop");
		mess.m_MessageType = 0;
		return CreateEntry(mess, receiver);
	}
	
	ref SCR_AIMessageSelectionMenuEntry CreateEngageOrder(vector pos, IEntity target, AIAgent receiver = null)
	{
		SCR_AIMessage_Attack mess = SCR_AIMessage_Attack.Cast(CreateMessage(SCR_AIMessage_Attack));		
		mess.SetText("Engage");
		mess.m_LastSeenPosition = pos;
		mess.m_MessageType = 1;
		mess.m_Target = target;
		return CreateEntry(mess, receiver);
	}
	
	void UpdateEntries()
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		vector posInWorld = vector.Zero;
		IEntity cursorTarget = null;
		AIAgent myAgent = GetAIAgent();
		
		if (!myAgent)
			return;
		
		AIGroup parentGroup = myAgent.GetParentGroup();
		bool hasGroup = parentGroup != null;
		bool isLeader = hasGroup && parentGroup.GetLeaderAgent() == myAgent;
		
		if (GetGame().GetCameraManager())
		{			
			CameraBase cam = GetGame().GetCameraManager().CurrentCamera();
			if (cam)
			{
				cursorTarget = cam.GetCursorTargetWithPosition(posInWorld);
			}
		}		
		vector entityPosInWorld = posInWorld;
		if (myAgent.GetControlledEntity())
		{
			entityPosInWorld = myAgent.GetControlledEntity().GetOrigin();
		}
					
		auto aDummies = new array<ref AIMessageSelectionMenuEntry>();
		if (!hasGroup)
		{
			aDummies.Insert(CreateReportEnemy(posInWorld, cursorTarget, null));
			aDummies.Insert(CreateReportUnderFire(entityPosInWorld, null));
			aDummies.Insert(CreateReportDeadFriendly(posInWorld,cursorTarget, null));
		}
		else
		{
			if (isLeader)
			{
				aDummies.Insert(CreateMoveOrder(posInWorld, parentGroup));
				aDummies.Insert(CreateStopOrder(parentGroup));
				aDummies.Insert(CreateEngageOrder(posInWorld,cursorTarget, parentGroup));
			}
			else
			{
				aDummies.Insert(CreateReportNoAmmo(entityPosInWorld, myAgent.GetControlledEntity(), parentGroup));
			}
			aDummies.Insert(CreateReportUnderFire(entityPosInWorld, parentGroup));
			aDummies.Insert(CreateReportDeadFriendly(posInWorld,cursorTarget, parentGroup));	
			aDummies.Insert(CreateDangerEvent(EAIDangerEventType.Danger_ProjectileHit, "Under fire", entityPosInWorld, myAgent.GetControlledEntity(), parentGroup));
		}
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		// Clear radial menu entries
		m_pCommandingRadialMenu.ClearEntries();
		
		// Add your entries
		foreach (auto entry : aDummies)
		{
			if (entry == null)
				continue;
			
			m_pCommandingRadialMenu.AddEntry(entry);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		auto pGame = GetGame();
		if (!pGame)
			return;
		
		auto pInputManager = pGame.GetInputManager();
		if (!pInputManager)
			return;
		
		if (!m_pCommandingRadialMenu)
			return;
		
		// Is radial menu open?
		bool hasChanged = false;
		bool bIsOpen = m_pCommandingRadialMenu.IsOpen();
		
		// Get desired input
		pInputManager.ActivateContext(m_sInputContextName);
		float fOpen = pInputManager.GetActionValue(m_sInputActionName);
		
		// Open radial menu
		if (!bIsOpen && fOpen > 0.0)
		{
			hasChanged = true;
			m_pCommandingRadialMenu.Open(owner);			
		}
		
		// Close radial menu
		if (bIsOpen && fOpen <= 0.0)
		{
			m_pCommandingRadialMenu.Close(owner);
		}
		
		// If radial menu is open, clear current entries and fill with new ones
		if (hasChanged)
		{
			UpdateEntries();
		}
		
		// Update radial menu
		m_pCommandingRadialMenu.Update(owner, timeSlice);
	}
};