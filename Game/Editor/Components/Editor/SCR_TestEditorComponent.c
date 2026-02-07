[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_TestEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/// @ingroup Editor_Components
/**

@example
*/
class SCR_TestEditorComponent : SCR_BaseEditorComponent
{
	override void EOnEditorInit()
	{
		PrintFormat("EOnEditorInit %1", this);
	}
	override void EOnEditorDelete()
	{
		PrintFormat("EOnEditorDelete %1", this);
	}
	override void EOnEditorOpen()
	{
		PrintFormat("EOnEditorOpen %1", this);
	}
	override void EOnEditorClose()
	{
		PrintFormat("EOnEditorClose %1", this);
	}
	override void EOnEditorOpenServer()
	{
		PrintFormat("EOnEditorOpenServer %1", this);
	}
	override void EOnEditorCloseServer()
	{
		PrintFormat("EOnEditorCloseServer %1", this);
	}
	override void EOnEditorPreActivate()
	{
		PrintFormat("EOnEditorPreActivate %1", this);
	}
	override void EOnEditorActivate()
	{
		PrintFormat("EOnEditorActivate %1", this);
	}
	override void EOnEditorPostActivate()
	{
		PrintFormat("EOnEditorPostActivate %1", this);
	}
	override void EOnEditorDeactivate()
	{
		PrintFormat("EOnEditorDeactivate %1", this);
	}
	override void EOnEditorDebug(array<string> debugTexts)
	{
		debugTexts.Insert(Type().ToString());
	}
};