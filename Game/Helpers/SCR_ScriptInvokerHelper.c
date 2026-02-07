//------------------------------------------------------------------------------------------------
//~ Generic Script invokers. To be used in any script that does not need specific invokers
void ScriptInvokerIntMethod(int i);
typedef func ScriptInvokerIntMethod;
typedef ScriptInvokerBase<ScriptInvokerIntMethod> ScriptInvokerInt;

void ScriptInvokerBoolMethod(bool b);
typedef func ScriptInvokerBoolMethod;
typedef ScriptInvokerBase<ScriptInvokerBoolMethod> ScriptInvokerBool;

void ScriptInvokerFloatMethod(float f);
typedef func ScriptInvokerFloatMethod;
typedef ScriptInvokerBase<ScriptInvokerFloatMethod> ScriptInvokerFloat;

void ScriptInvokerVectorMethod(vector v);
typedef func ScriptInvokerVectorMethod;
typedef ScriptInvokerBase<ScriptInvokerVectorMethod> ScriptInvokerVector;

void ScriptInvokerEntityMethod(IEntity e);
typedef func ScriptInvokerEntityMethod;
typedef ScriptInvokerBase<ScriptInvokerEntityMethod> ScriptInvokerEntity;

void ScriptInvokerWidgetMethod(Widget w);
typedef func ScriptInvokerWidgetMethod;
typedef ScriptInvokerBase<ScriptInvokerWidgetMethod> ScriptInvokerWidget;

//------------------------------------------------------------------------------------------------
/*
//~ This is an example class how to define and use script invokers
class SCR_ExampleClass
{
	//~ Define the event
	protected ref ScriptInvokerInt m_OnExampleEvent;
	
	//~ return Example script invoker
	ScriptInvokerInt GetOnExampleEvent()
	{
		if (!m_OnExampleEvent)
			m_OnExampleEvent = new ScriptInvokerInt();
		
		return m_OnExampleEvent;
	}
	
	protected void ExampleInvoke()
	{
		int exampleInt;
		m_OnExampleEvent.Invoke(exampleInt);
	}
	
	protected void MyExampleFunction(int exampleInt)
	{
		Print(string.Format("I was executed %1", exampleInt), LogLevel.VERBOSE);
	}
	
	protected void ExampleInsert()
	{
		SCR_ExampleClass exampleClass; //~ Get this from somewhere
		
		exampleClass.GetOnExampleEvent().Insert(MyExampleFunction);
	}
	
	protected void ExampleRemove()
	{
		SCR_ExampleClass exampleClass; //~ Get this from somewhere
		
		exampleClass.GetOnExampleEvent().Remove(MyExampleFunction);
	}
};
*/


