[BaseContainerProps()]
class SCR_InfoDisplayLayerHandler : SCR_InfoDisplayHandler
{
	[Attribute()]
	protected string m_sLayerName;
	
	protected override void OnStart(notnull SCR_InfoDisplay display)
	{
		if (display.GetHandler(SCR_InfoDisplaySlotHandler) != null)
		{
			Print("Info Display's can't have SCR_InfoDisplayLayerHandler & SCR_InfoDisplaySlotHandler co-exist as handlers!", LogLevel.ERROR);
			return;
		}

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return;

		Widget hudRoot = hudManager.GetHUDRootWidget();
		if (!hudRoot)
			return;

		Widget layerWidget = hudRoot.FindAnyWidget(m_sLayerName);
		if (!layerWidget)
			return;

		Widget rootWidget = workspace.CreateWidgets(display.m_LayoutPath, layerWidget);
	}
};
