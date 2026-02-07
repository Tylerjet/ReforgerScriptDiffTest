// TODO: delete
[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_BranchedJunctionPowerPoleClass : SCR_JunctionPowerPoleClass
{
}

class SCR_BranchedJunctionPowerPole : SCR_JunctionPowerPole
{
	[Attribute("3")]
	protected int m_iBranchSize;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_BranchedJunctionPowerPole(IEntitySource src, IEntity parent)
	{
	}
}
