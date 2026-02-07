[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableSystemComponentClass: SCR_EditableEntityComponentClass
{
};

/** @ingroup Editable_Entities
*/

/*!
Editable entity which can contain location description.
*/
class SCR_EditableSystemComponent: SCR_EditableEntityComponent
{	
	protected SCR_FactionControlComponent m_FactionControlComponent;
	protected ref ScriptInvoker Event_OnUIRefresh = new ref ScriptInvoker;
	
	protected void OnFactionChanged()
	{
		Event_OnUIRefresh.Invoke();
	}
	
	/*!
	Get entity's faction.
	\return Faction
	*/
	override Faction GetFaction()
	{
		if (m_FactionControlComponent)
		{
			return m_FactionControlComponent.GetFaction();
		}
		return null;
	}
	
	/*!
	Get event called when GUI should refresh entity's GUI, i.e., update faction color and call events in GUI widgets.
	\return Script invoker
	*/
	override ScriptInvoker GetOnUIRefresh()
	{
		return Event_OnUIRefresh;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_FactionControlComponent = SCR_FactionControlComponent.Cast(owner.FindComponent(SCR_FactionControlComponent));
		if (m_FactionControlComponent)
		{
			m_FactionControlComponent.GetOnFactionChanged().Insert(OnFactionChanged);
		}
	}
	
	void ~SCR_EditableSystemComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (m_FactionControlComponent)
		{
			m_FactionControlComponent.GetOnFactionChanged().Insert(OnFactionChanged);
		}
	}
};