//------------------------------------------------------------------------------------------------
// all AI re-gistered animations, must specify In and Out to be possible to use in "AnimationWaypoint"

[BaseContainerProps()]
class SCR_AIAnimation_Base
{
	bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4]);
	bool StopAnimation(IEntity pUserEntity, bool performFast);
}

//----------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimation_Sitting : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StartLoitering(ELoiteringType.SIT, false, true, true, vAnimationTransform);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}	
}

[BaseContainerProps()]
class SCR_AIAnimation_Pushups : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StartLoitering(ELoiteringType.PUSHUPS, true, true, true, vAnimationTransform);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}
};

//----------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimation_Loitering : SCR_AIAnimation_Base
{
	//------------------------------------------------------------------------------------------------
	override bool StartAnimation(IEntity pUserEntity, vector vAnimationTransform[4])
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StartLoitering(ELoiteringType.LOITERING, true, true, true, vAnimationTransform);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool StopAnimation(IEntity pUserEntity, bool performFast)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return false;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return false;
		
		controller.StopLoitering(performFast);
		return true;
	}
}
