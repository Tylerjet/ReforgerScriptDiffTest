/*!
Component which opens download manager when button is activated
*/
class SCR_DownloadManager_OpenDownloadManagerComponent : ScriptedWidgetComponent
{
	override void HandlerAttached(Widget w)
	{
		SCR_NavigationButtonComponent n = SCR_NavigationButtonComponent.Cast(w.FindHandler(SCR_NavigationButtonComponent));
		n.m_OnActivated.Insert(SCR_DownloadManager_Dialog.Create);
	}
};