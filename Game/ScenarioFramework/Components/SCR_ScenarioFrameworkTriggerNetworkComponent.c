//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTriggerNetworkComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTriggerNetworkComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//! Handles replication according to if owner of this component is in the trigger or just left it
	void ReplicateTriggerState(SCR_CharacterTriggerEntity trigger, bool left)
	{
		if (!left)
			Rpc(RpcDo_InvokeTriggerUpdated, trigger.GetActivationCountdownTimer(), trigger.GetActivationCountdownTimerTemp(), trigger.GetPlayersCountByFactionInsideTrigger(trigger.GetOwnerFaction()), trigger.GetPlayersCountByFaction(), trigger.GetPlayerActivationNotificationTitle(), trigger.GetTriggerConditionsStatus());
		else
			Rpc(RpcDo_InvokePlayerLeftTrigger);
	}

	//------------------------------------------------------------------------------------------------
	//! Invokes OnTriggerUpdated
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_InvokeTriggerUpdated(float activationCountdownTimer, float tempWaitTime, int playersCountByFactionInside, int playersCountByFaction, string playerActivationNotificationTitle, bool triggerConditionsStatus)
	{
		SCR_CharacterTriggerEntity.s_OnTriggerUpdated.Invoke(activationCountdownTimer, tempWaitTime, playersCountByFactionInside, playersCountByFaction, playerActivationNotificationTitle, triggerConditionsStatus);
	}

	//------------------------------------------------------------------------------------------------
	//! Invokes OnTriggerUpdatedPlayerNotPresent to the player who just left the trigger
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_InvokePlayerLeftTrigger()
	{
		SCR_CharacterTriggerEntity.s_OnTriggerUpdatedPlayerNotPresent.Invoke(0);
	}
};
