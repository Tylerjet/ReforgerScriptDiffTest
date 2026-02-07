//------------------------------------------------------------------------------------------------
class SCR_CareerEndScreen: ScriptedWidgetComponent 
{
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		Widget rootWidget = w;
		Widget BodyLayout = rootWidget.FindAnyWidget("BodyLayout");
		
		Widget FirstVerticalSection = BodyLayout.FindAnyWidget("FirstVerticalSection");
		Widget SecondtVerticalSection = BodyLayout.FindAnyWidget("SecondVerticalSection");
		Widget ThirdVerticalSection = BodyLayout.FindAnyWidget("ThirdVerticalSection");
		
		Widget PlayerHud = FirstVerticalSection.FindAnyWidget("PlayerHud");
		
		if (!PlayerHud)
			return;
		
		SCR_CareerProfileHUD PlayerHudHandler = SCR_CareerProfileHUD.Cast(PlayerHud.FindHandler(SCR_CareerProfileHUD));
		if (!PlayerHudHandler)
			return;
		
		PlayerHudHandler.PrepareHUD("", "", "", "", "", "CareerEndRankProgressBar", "");
		
		PlayerHudHandler.SetPlayerRank(3);
		PlayerHudHandler.SetProgressBarValue(100);
	}	
};