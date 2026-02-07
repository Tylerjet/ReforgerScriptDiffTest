/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Entities
* @{
*/

class RadioManagerEntityClass: GenericEntityClass
{
};

class RadioManagerEntity: GenericEntity
{
	/*!
	Query all other radio components in the range. Doesn't involve frequency, encryption or power of the radio.
	\param[in] radio            Source radio component.
	\param[out] radiosInRange   List of other RadioComponents in range.
	*/
	proto external int GetRadiosInRange(BaseRadioComponent radio, out notnull array<BaseRadioComponent> radiosInRange);
};

/** @}*/
