class SCR_RadialMenuVisualsCommands : SCR_RadialMenuVisuals
{
	protected const int METERS_TO_KM_THRESHOLD = 1000;
	
	protected ref SCR_PhysicsHelper m_PhysicsHelper;
	
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
		m_PhysicsHelper = new SCR_PhysicsHelper();
		m_PhysicsHelper.GetOnTraceFinished().Insert(UpdateTargetDistanceCallback);
		
		PlayerController controller = GetGame().GetPlayerController();
		PlayerCamera camera = controller.GetPlayerCamera();
		if (!camera)
			return;
		
		IEntity controlledEntity = controller.GetControlledEntity();
		
		vector mat[4];
		camera.GetTransform(mat);
		vector end = mat[3] + mat[2] * 20000;
	
		m_PhysicsHelper.TraceSegmented(mat[3], end, TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.ANY_CONTACT, EPhysicsLayerDefs.Projectile, controlledEntity);
	
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTargetDistanceCallback(vector position, IEntity entity)
	{
		TextWidget distanceText = TextWidget.Cast(m_wRoot.FindAnyWidget("RichTxtDistance"));
		if (!distanceText)
			return;
		
		PlayerController playerControl = GetGame().GetPlayerController();
		if (!playerControl)
			return;
		
		if (position == vector.Zero)
		{
			distanceText.SetText("#AR-Commanding_Distance_Empty_Meters");
			return;
		}
		
		vector playerPosition = playerControl.GetControlledEntity().GetOrigin();
		float distance = vector.Distance(position, playerPosition);
		
		if (distance < METERS_TO_KM_THRESHOLD)
		{
			distanceText.SetTextFormat("#AR-ValueUnit_Short_Meters", Math.Round(distance));
			return;
		}
		
		//we are converting meters to kilometers with one decimal
		float kilometers = Math.Round(distance / 100) / 10;
		
		distanceText.SetTextFormat("#AR-CareerProfile_KMs", kilometers);
	}
}