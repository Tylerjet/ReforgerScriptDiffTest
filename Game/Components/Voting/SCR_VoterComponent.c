[ComponentEditorProps(category: "GameScripted/Voting", description: "")]
class SCR_VoterComponentClass: ScriptComponentClass
{
};
class SCR_VoterComponent: ScriptComponent
{
	protected PlayerController m_PlayerController;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, anywhere
	/*!
	Get local instance of this component.
	\return Local voter component
	*/
	static SCR_VoterComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return SCR_VoterComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_VoterComponent));
		else
			return null;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, owner
	/*!
	Vote!
	\param type Type of the vote
	\param value Voted value, depends on the type (e.g., for KICK it's player ID)
	*/
	void Vote(EVotingType type, int value)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		Rpc(VoteServer, type, value);
		manager.VoteLocal(type, value);
	}
	/*!
	Remove previously cast vote.
	\param type Type of the vote
	\param value Voted value, depends on the type (e.g., for KICK it's player ID)
	*/
	void RemoveVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		Rpc(RemoveVoteServer, type, value);
		manager.RemoveVoteLocal(type, value);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected, server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void VoteServer(EVotingType type, int value)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (manager)
			manager.Vote(m_PlayerController.GetPlayerId(), type, value);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RemoveVoteServer(EVotingType type, int value)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (manager)
			manager.RemoveVote(m_PlayerController.GetPlayerId(), type, value);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Overrides
	override void OnPostInit(IEntity owner)
	{
		m_PlayerController = PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Debug.Error2("SCR_VoterComponent", "SCR_VoterComponent must be attached to PlayerController!");
			return;
		}
	}
};