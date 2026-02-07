// aproximation if character is in cone. We assume just one point(aiming position) not whole character
// steps explained here:
// https://stackoverflow.com/questions/12826117/how-can-i-detect-if-a-point-is-inside-a-cone-or-not-in-3d-space/
//------------------------------------------------------------------------------------------------
static bool SCR_AIIsCharacterInCone(ChimeraCharacter character, vector coneTip, vector coneAxis, float coneRadius = 4)
{
	if (!character)
		return false;
	vector aimingPos = character.AimingPosition();
	vector dirFromCharacter = vector.Direction(aimingPos, coneTip);
	
	float coneDist = vector.Dot(dirFromCharacter, coneAxis);
	
	// check if tested point is in same "half-plain" as cone is cast
	if (coneDist > 0)
		return false;

	float orthDistance = (dirFromCharacter - coneAxis * coneDist).Length();
	return orthDistance < coneRadius;
};

// gets a string from value given in port
// basically workaround that we can't cast void from GetVariableIn into type of that variable
//------------------------------------------------------------------------------------------------
static string SCR_AIGetStringFromPort(Node node, string port)
{
	if ( node.GetVariableType(true, port) == string )
	{
		string value;
		node.GetVariableIn(port, value);
		return value;		
	}
	else if ( node.GetVariableType(true, port) == int )
	{
		int value;
		node.GetVariableIn(port, value);
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port) == float )
	{
		float value;
		node.GetVariableIn(port, value);
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port) == bool )
	{
		bool value;
		node.GetVariableIn(port, value);
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port) == vector )
	{
		vector value;
		node.GetVariableIn(port, value);
		return value.ToString();
	}
	else if ( node.GetVariableType(true, port).IsInherited(Managed) )
	{
		Managed value;
		node.GetVariableIn(port, value);
		if (value)
			return value.ToString();
	}
	return "";
};

// use this for generating random numbers for AI, no need to create another instance
//------------------------------------------------------------------------------------------------------------------------------------------------------------
static ref RandomGenerator s_AIRandomGenerator = new RandomGenerator;

// use this for interpretting relevant character stance based on threat level
//------------------------------------------------------------------------------------------------
static ECharacterStance GetStanceFromThreat(EAIThreatState threatState)
{
	switch (threatState)
	{
		case EAIThreatState.THREATENED: return ECharacterStance.PRONE;
		case EAIThreatState.ALERTED: return ECharacterStance.CROUCH;
		case EAIThreatState.SAFE: return ECharacterStance.STAND;
		default: return ECharacterStance.STAND;
	}
	return ECharacterStance.STAND;
};

//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Weapon Handling
class SCR_AIWeaponHandling
{
	//--------------------------------------------------------------------------------------------
	static int GetCurrentMuzzleId(BaseWeaponManagerComponent weapMgr)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return -1;
				
		// Find muzzle ID
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		BaseMuzzleComponent currentMuzzle = weaponComp.GetCurrentMuzzle();
		
		if (!currentMuzzle || muzzles.IsEmpty())
			return -1;
		
		int currentMuzzleId = muzzles.Find(currentMuzzle);
		return currentMuzzleId;
	}
	
	//--------------------------------------------------------------------------------------------
	static BaseMagazineComponent GetCurrentMagazineComponent(BaseWeaponManagerComponent weapMgr)
	{
		BaseWeaponComponent weaponComp = weapMgr.GetCurrentWeapon();
		
		if (!weaponComp)
			return null;
		
		BaseMuzzleComponent	currentMuzzle = weaponComp.GetCurrentMuzzle();
		if (!currentMuzzle)
			return null;
		
		return currentMuzzle.GetMagazine();
	}
	
	//--------------------------------------------------------------------------------------------
	//! Although trivial, there are several ways to get current weapon, thus let's keep this function
	static BaseWeaponComponent GetCurrentWeaponComponent(BaseWeaponManagerComponent weapMgr)
	{
		return weapMgr.GetCurrentWeapon();
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMuzzleSwitch(CharacterControllerComponent controller, int newMuzzleId)
	{
		controller.SetMuzzle(newMuzzleId);
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMagazineSwitchCharacter(CharacterControllerComponent controller, BaseMagazineComponent newMagazineComp)
	{
		controller.ReloadWeaponWith(newMagazineComp.GetOwner());
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartMagazineSwitchTurret(TurretControllerComponent controller, BaseMagazineComponent newMagazineComp)
	{
		controller.DoReloadWeaponWith(newMagazineComp.GetOwner());
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartWeaponSwitchCharacter(CharacterControllerComponent controller, BaseWeaponComponent newWeaponComp)
	{
		controller.TryEquipRightHandItem(newWeaponComp.GetOwner(), EEquipItemType.EEquipTypeWeapon, false);
	}
	
	//--------------------------------------------------------------------------------------------
	static void StartWeaponSwitchTurret(TurretControllerComponent controller, BaseWeaponComponent newWeaponComp, IEntity turretOperator)
	{
		// Find weapon slot which has this weapon
		IEntity newWeaponEntity = newWeaponComp.GetOwner();
		BaseWeaponManagerComponent weaponMgr = controller.GetWeaponManager();
		if (!weaponMgr)
			return;
		
		WeaponSlotComponent newWeaponSlot;
		array<WeaponSlotComponent> slots = {};
		weaponMgr.GetWeaponsSlots(slots);
		foreach (WeaponSlotComponent slot : slots)
		{
			if (slot.GetWeaponEntity() == newWeaponEntity)
				newWeaponSlot = slot;
		}
		
		if (!newWeaponSlot)
		{
			// There is no slot which has this weapon
			return;
		}
		
		controller.SelectWeapon(turretOperator, newWeaponSlot);
	}
}