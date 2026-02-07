//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BaseGroupCommand : SCR_BaseRadialCommand
{	
	[Attribute(defvalue: "1", desc: "Will the command be shown to leader?")]
	protected bool m_bShowToLeader;
	
	[Attribute(defvalue: "0", desc: "Will the command be shown to member?")]
	protected bool m_bShowToMember;
	
	[Attribute(defvalue: "0", desc: "Will the command be shown while commanding from map?")]
	protected bool m_bShowOnMap;
	
	[Attribute(defvalue: "0", desc: "Is the command available while player is dead?")]
	protected bool m_bUsableWhileDead;
	
	[Attribute(defvalue: "0", desc: "Is the command available while player is unconscious?")]
	protected bool m_bUsableWhileUncouscious;
	
	[Attribute(defvalue: "0", desc: "ID of the gesture in gesture state machine to be played when command is given out. 0 = no gesture")]
	protected int m_iGestureID;
	
	[Attribute("", UIWidgets.EditBox, "Name of the sound event to be played, leave empty for no event")]
	protected string m_sSoundEventName;
	
	//------------------------------------------------------------------------------------------------
	override bool CanRoleShow()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;
		
		bool isPlayerLeader = groupController.IsPlayerLeaderOwnGroup();
		if (isPlayerLeader && !m_bShowToLeader)
			return false;
		
		if (!isPlayerLeader && !m_bShowToMember)
			return false;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	void PlayAIResponse(IEntity slaveGroup)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(slaveGroup);
		if (!slaveGroup)
			return;
		
		AIAgent agent = aiGroup.GetLeaderAgent();
		if (!agent)
			return;
		
		IEntity owner = agent.GetControlledEntity();
		if (!owner)
			return;
		
		SCR_CommunicationSoundComponent soundComponent = SCR_CommunicationSoundComponent.Cast(owner.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComponent)
			return;
		
		soundComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_POSITIVEFEEDBACK, 50);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSoundEventName()
	{
		return m_sSoundEventName;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanShowOnMap()
	{
		return m_bShowOnMap;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCommandGestureID()
	{
		return m_iGestureID;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownInCurrentLifeState()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		
		//returns true because if there is no controlled entity then player is not dead nor uncoscious.
		ChimeraCharacter character = ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!character)
			return true;
		
		//Return true in case of component not found to prevent VME
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return true;
		
		ECharacterLifeState lifeState = controller.GetLifeState();
		if ((!m_bUsableWhileDead && lifeState == ECharacterLifeState.DEAD) || (!m_bUsableWhileUncouscious && lifeState == ECharacterLifeState.INCAPACITATED))
			return false;
		
		return true;
	}
}