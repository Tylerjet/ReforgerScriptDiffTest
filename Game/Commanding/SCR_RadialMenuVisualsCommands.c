class SCR_RadialMenuVisualsCommands : SCR_RadialMenuVisuals
{
	protected const int METERS_TO_KM_THRESHOLD = 1000;
	
	//------------------------------------------------------------------------------------------------
	override protected event void OnOpen(IEntity owner)
	{
		super.OnOpen(owner);
		UpdateTargetDistance();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected string SetEmptyIconByType(ScriptedSelectionMenuEntry entry)
	{
		if (!entry)
			return m_sEmptyIconName;
		
		SCR_CommandingMenuEntry commandEntry = SCR_CommandingMenuEntry.Cast(entry);
		if (!commandEntry)
			return m_sEmptyIconName;
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return m_sEmptyIconName;
		
		SCR_BaseGroupCommand command = commandingManager.FindCommand(commandEntry.GetCommandName());
		if (!command)
			return m_sEmptyIconName;
		
		string iconName = command.GetIconName();
		if (iconName.IsEmpty())
			iconName = m_sEmptyIconName;
		
		return iconName;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTargetDistance()
	{
		TextWidget distanceText = TextWidget.Cast(m_wRoot.FindAnyWidget("RichTxtDistance"));
		if (!distanceText)
			return;
		
		PlayerController playerControl = GetGame().GetPlayerController();
		
		
		vector targetPosition, playerPosition;
		int distance;
		playerControl.GetPlayerCamera().GetCursorTargetWithPosition(targetPosition);
		if (targetPosition == vector.Zero)
		{
			distanceText.SetText("#AR-Commanding_Distance_Empty_Meters");
			return;
		}
		
		playerPosition = playerControl.GetControlledEntity().GetOrigin();
		
		
		distance = vector.Distance(targetPosition, playerPosition);
		
		if (distance < METERS_TO_KM_THRESHOLD)
		{
			distanceText.SetTextFormat("#AR-ValueUnit_Short_Meters", distance);
			return;
		}
		
		//we are converting meters to kilometers with one decimal
		float kilometers = Math.Round(distance / 100) / 10;
		
		distanceText.SetTextFormat("#AR-CareerProfile_KMs", kilometers);
	}
}