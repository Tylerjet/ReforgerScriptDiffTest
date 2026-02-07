[ComponentEditorProps(category: "GameScripted/KickHintComponent", description: "")]
class SCR_KickHintComponentClass: ScriptComponentClass
{
};
class SCR_KickHintComponent: ScriptComponent
{
	
	protected ref ScriptInvoker m_OnCriminalScoreIncreased = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoIncreaseCriminalScore(float points)
	{
		m_OnCriminalScoreIncreased.Invoke(points);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowUI(float points)
	{
		SCR_HintManagerComponent.ShowCustomHint("Friendly fire relates to any accidental attack on friendly troops. It may occur as the result of a misidentification, or a miscalculation when coordinating fire support, or by getting caught in a cross-fire. In short, mistakes can – and often do – happen in the field. \nFrequent incidents of friendly fire however, are a cause for concern. Consequently, repeated cases of fratricide will be considered deliberate, and will result in a kick from a session or a temporary ban.", "FF (temp string). SCORE: "+points, 15);
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyClientCriminalScoreIncreased(SCR_ECrimeNotification crime, float points)
	{
		Rpc(RPC_DoIncreaseCriminalScore, points);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_OnCriminalScoreIncreased.Insert(ShowUI);
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_ECrimeNotification
{
	TEAMKILL
}