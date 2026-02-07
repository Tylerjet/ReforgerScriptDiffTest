/*
Class with misc functions for combat movement
*/

class SCR_AICombatMoveUtils
{
	// Threshold distance beyond which it is considered we are in long range fight
	static const float CLOSE_RANGE_COMBAT_DIST = 40.0;
	
	//! Decodes SCR_EAICombatMoveDirection enum
	static vector CalculateMoveDirection(SCR_EAICombatMoveDirection eDirection, vector myPos, vector movePos)
	{
		// Special cases
		if (eDirection == SCR_EAICombatMoveDirection.ANYWHERE)
		{
			// Random direction
			float bearing = Math.RandomFloat(0, Math.PI2);
			vector dirOut = Vector(Math.Cos(bearing), 0, Math.Sin(bearing));
			return dirOut;
		}
		else if (eDirection == SCR_EAICombatMoveDirection.CUSTOM_POS)
		{
			vector dirToTarget = vector.Direction(myPos, movePos);
			dirToTarget.Normalize();
			return dirToTarget;
		}
		
		// Generic directions
		
		vector dirToTarget = vector.Direction(myPos, movePos);
		dirToTarget.Normalize();
		vector dirSideways = (dirToTarget * Vector(0, 1, 0));
		
		switch (eDirection)
		{
			case SCR_EAICombatMoveDirection.FORWARD:	return dirToTarget;
			case SCR_EAICombatMoveDirection.BACKWARD:	return -dirToTarget;
			case SCR_EAICombatMoveDirection.RIGHT:		return -dirSideways;
			case SCR_EAICombatMoveDirection.LEFT:		return dirSideways;
		}
		return vector.Zero;
	}
	
	//! Returns true if aiming is possible in this stance and speed
	//! Some cases are unachievable by character controller
	static bool IsAimingAndMovementPossible(ECharacterStance stance, EMovementType moveType)
	{
		if (stance == ECharacterStance.PRONE)
			return false;
		
		if (moveType == EMovementType.SPRINT)
			return false;
		
		return true;
	}
}