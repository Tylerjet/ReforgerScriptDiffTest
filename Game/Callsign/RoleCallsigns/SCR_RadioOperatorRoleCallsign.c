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
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(character.GetControlledEntity().FindComponent(EquipedLoadoutStorageComponent));
		if (!loadoutStorage)
			return false;
		
		IEntity backpack = loadoutStorage.GetClothFromArea(LoadoutBackpackArea);
		return (backpack && backpack.FindComponent(SCR_RadioComponent));
	}
	
	
	
	override int GetRoleIndex()
	{
		return ERoleCallsign.RADIO_OPERATOR;
	}
};