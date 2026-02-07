enum ELoiterCommandState
{
	LOITERING = 0,
	EXITING = 1,
	DONE = 2
};

class SCR_CharacterCommandLoiter : CharacterCommandScripted
{
	void SCR_CharacterCommandLoiter(BaseAnimPhysComponent pAnimPhysComponent, ChimeraCharacter pCharacter, CharacterInputContext pBaseInputCtx, SCR_ScriptedCharacterInputContext pScrInputCtx, SCR_ScriptedCommandsStaticTable pStaticTable)
	{
		m_pCharAnimComponent = CharacterAnimationComponent.Cast(pAnimPhysComponent);
		m_pCharacter = pCharacter;
		m_pBaseInputCtx = pBaseInputCtx;
		m_pScrInputCtx = pScrInputCtx;
		m_pStaticTable = pStaticTable;
	}
	
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

	override void OnDeactivate()
	{
		m_pScrInputCtx.m_iLoiteringType = -1;
	}

	override bool IsRootMotionControlled()
	{
		return m_pScrInputCtx.m_bLoiteringRootMotion;
	}
	
	override void PrePhysUpdate(float pDt)
	{
		switch (m_eState)
		{
			case ELoiterCommandState.LOITERING:
			{
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag);
				m_bWasTag = isTag || m_bWasTag;
				
				InputManager iManager = GetGame().GetInputManager();
				bool cancelLoiterInputs = iManager.GetActionValue("CharacterFire") || iManager.GetActionValue("CharacterSprint") || iManager.GetActionValue("CharacterRaiseWeapon") || iManager.GetActionValue("CharacterWeaponADS") || iManager.GetActionValue("CharacterReload");
				
				if ((m_bWasTag && !isTag) || cancelLoiterInputs)
				{
					SwitchState(ELoiterCommandState.EXITING);
				}
			}
			break;
			case ELoiterCommandState.EXITING:
			{
				bool isTag = m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringTag) || m_pCharAnimComponent.IsSecondaryTag(m_pStaticTable.m_IsLoiteringTag);
				if (!isTag)
				{
					FinishLoiter();
				}
			}
			break;
		}
	}
	
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
				m_pCharAnimComponent.CallCommand(m_pStaticTable.m_CommandGesture, 0, 0.0);
			}
			break;
		}

		m_eState = newState;
	}
	
	//terminateFast should be true when we are going into alerted or combat state.
	void StopLoitering(bool terminateFast)
	{
		if (m_pCharAnimComponent.IsPrimaryTag(m_pStaticTable.m_IsLoiteringStartingTag))
			return;

		SwitchState(ELoiterCommandState.EXITING);
	}

	protected void FinishLoiter()
	{
		SetFlagFinished(true);
		m_pScrInputCtx.m_iLoiteringType = -1;
	}

	protected CharacterAnimationComponent m_pCharAnimComponent;
	protected ChimeraCharacter m_pCharacter;
	protected CharacterInputContext m_pBaseInputCtx;
	protected SCR_ScriptedCharacterInputContext m_pScrInputCtx;
	protected SCR_ScriptedCommandsStaticTable m_pStaticTable;
	
	protected ELoiterCommandState m_eState;
	protected bool m_bWasTag;
};