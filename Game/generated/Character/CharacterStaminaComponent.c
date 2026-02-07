/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Character
* @{
*/

class CharacterStaminaComponentClass: BaseStaminaComponentClass
{
};

class CharacterStaminaComponent: BaseStaminaComponent
{
	/* EXAMPLE
	class SCR_CharacterStaminaComp : CharacterStaminaComponent
	{
		override event void OnStaminaDrain(float pDrain)
		{
			Print("stamina drained");
		}
	}
	*/
	
	/*!
	Returns breath progress as <0,1>.
	Progress goes from 0 to 1 then back to 0.
	\return Returns breath progress as a value <0,1> where 0 is breath out, 1 is breath in.
	*/
	proto external float GetBreathProgress();
	/*!
	Returns current breath frequency.
	\return Returns the frequency, i.e. amount of full breaths taken per second.
	*/
	proto external float GetBreathFrequency();
	/*!
	Returns current breath magnitude.
	\return Returns the current magnitude of breath.
	*/
	proto external float GetBreathMagnitude();
	
	// callbacks
	
	/*!
	Runs every time stamina value has changed.
	\param pDrain How much stamina has recovered/drained.
	*/
	event void OnStaminaDrain(float pDrain);
	/*!
	Called when breath progress reaches minimum or maximum value.
	(Lo = 0, Hi = 1)
	\param breathValue Value that breath progress has reached.
	*/
	event protected void OnBreathStateChanged(float breathValue);
};

/** @}*/
