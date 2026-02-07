[BaseContainerProps(), BaseContainerCustomStringTitleField("Radio Operator")]
class SCR_RadioOperatorRoleCallsign: SCR_BaseRoleCallsign
{	
	override bool IsValidRole(AIAgent character, SCR_AIGroup group, map<int, AIAgent> occupiedRoles, out int roleCallsignIndex)
	{
		if (!super.IsValidRole(character, group, occupiedRoles, roleCallsignIndex))
			return false;
			
		if (HasRadio(character))
		{
			roleCallsignIndex = GetRoleIndex();
			return true;
		}
		
		return false;
	}
	
	protected bool HasRadio(AIAgent character)
	{
		BaseLoadoutManagerComponent loadout = BaseLoadoutManagerComponent.Cast(character.GetControlledEntity().FindComponent(BaseLoadoutManagerComponent));
		
		if (!loadout)
			return false;
		
		IEntity backpack = loadout.GetClothByArea(LoadoutBackpackArea);
		return (backpack && backpack.FindComponent(SCR_RadioComponent));
	}
	
	
	
	override int GetRoleIndex()
	{
		return ERoleCallsign.RADIO_OPERATOR;
	}
};