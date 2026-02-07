
class SCR_CharacterVicinityComponentClass: CharacterVicinityComponentClass
{
};

class SCR_CharacterVicinityComponent: CharacterVicinityComponent
{

	ref ScriptInvoker OnVicinityUpdateInvoker = new ref ScriptInvoker();
	private void OnUpdate()
	{
		OnVicinityUpdateInvoker.Invoke();
	}
};