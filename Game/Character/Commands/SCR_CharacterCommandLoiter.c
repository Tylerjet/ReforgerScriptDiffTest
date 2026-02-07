enum ELoiterCommandState // TODO: SCR_
{
	LOITERING = 0,
	EXITING = 1,
	DONE = 2
}

class SCR_CharacterCommandLoiter : CharacterCommandScripted
{
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_CharacterCommandLoiter(BaseAnimPhysComponent pAnimPhysComponent, ChimeraCharacter pCharacter, CharacterInputContext pBaseInputCtx, SCR_ScriptedCharacterInputContext pScrInputCtx, SCR_ScriptedCommandsStaticTable pStaticTable, SCR_CharacterCommandHandlerComponent pScrCommandHandler)
	{
		m_pCharAnimComponent = CharacterAnimationComponent.Cast(pAnimPhysComponent);
		m_pCharacter = pCharacter;
		m_pBaseInputCtx = pBaseInputCtx;
		m_pScrInputCtx = pScrInputCtx;
		m_pStaticTable = pStaticTable;
		m_pCommandHandler = pScrCommandHandler;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_bWasTag = false;
		
		if (m_pScrInputCtx.m_iLoiteringType < 0)
		{
			SetFlagFinished(true);
			return;
		}
		
		SwitchState(ELoiterCommandState.LOITERING);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeactivate()
	{
		m_pScrInputCtx.m_iLoiteringType = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPossess()
	{
		StopLoitering(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsRootMotionControlled()
	{
		return m_pScrInputCtx.m_bLoiteringRootMotion;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PrePhysUpdate(float pDt)
	{
		switch (m_eState)
		{
			case ELoiterCommandState.LOITERING:
			{
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag);
				m_bWasTag = isTag || m_bWasTag;
				
				bool cancelLoiterInputs = false;
				
				PlayerController playerController = GetGame().GetPlayerController();
				if (playerController && playerController.GetControlledEntity() == m_pCharacter)
				{
					InputManager iManager = GetGame().GetInputManager();
					cancelLoiterInputs = iManager.GetActionValue("CharacterFire") || iManager.GetActionValue("CharacterSprint") || iManager.GetActionValue("CharacterRaiseWeapon") || iManager.GetActionValue("CharacterWeaponADS") || iManager.GetActionValue("CharacterReload");
				}
				
				if ((m_bWasTag && !isTag) || (cancelLoiterInputs && !m_pScrInputCtx.m_bLoiteringDisablePlayerInput))
				{
					m_pCommandHandler.StopLoitering(false);
				}
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				m_pCharAnimComponent.CallCommand(m_pStaticTable.m_CommandGesture, -1, 0.0); // -1 is soft exit.
				// @TODO(ivanickyjak) We cannot use tags on server.
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag) || m_pCharAnimComponent.IsSecondaryTag(m_pStaticTable.m_IsLoiteringTag);
				if (!isTag)
				{
					FinishLoiter();
					m_pCommandHandler.BroadCastLoiterFinish();
				}
			}
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param newState
	void SwitchState(ELoiterCommandState newState)
	{
		switch (newState)
		{
			case ELoiterCommandState.LOITERING:
			{
				m_bWasTag = false;
				m_pCharAnimComponent.CallCommand4I(m_pStaticTable.m_CommandGesture, 0, m_pScrInputCtx.m_iLoiteringType, 0, 0, 0.0);
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				m_pCharAnimComponent.CallCommand(m_pStaticTable.m_CommandGesture, -1, 0.0); // -1 is soft exit.
			}
			break;
		}

		m_eState = newState;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param terminateFast should be true when going into alerted or combat state.
	void StopLoitering(bool terminateFast)
	{
		SwitchState(ELoiterCommandState.EXITING);
	}

	//------------------------------------------------------------------------------------------------
	void FinishLoiter()
	{
		SetFlagFinished(true);
		m_pScrInputCtx.m_iLoiteringType = -1;
	}

	protected CharacterAnimationComponent m_pCharAnimComponent;
	protected ChimeraCharacter m_pCharacter;
	protected CharacterInputContext m_pBaseInputCtx;
	protected SCR_ScriptedCharacterInputContext m_pScrInputCtx;
	protected SCR_ScriptedCommandsStaticTable m_pStaticTable;
	protected SCR_CharacterCommandHandlerComponent m_pCommandHandler;
	
	protected ELoiterCommandState m_eState;
	protected bool m_bWasTag;
}
