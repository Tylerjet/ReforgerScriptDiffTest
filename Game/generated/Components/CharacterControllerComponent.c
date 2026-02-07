/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Components
* @{
*/

class CharacterControllerComponentClass: PrimaryControllerComponentClass
{
};

class CharacterControllerComponent: PrimaryControllerComponent
{
	proto external CharacterAnimationComponent GetAnimationComponent();
	proto external BaseWeaponManagerComponent GetWeaponManagerComponent();
	proto external CameraHandlerComponent GetCameraHandlerComponent();
	proto external InventoryStorageManagerComponent GetInventoryStorageManager();
	proto external VoNComponent GetVONComponent();
	proto external CharacterInputContext GetInputContext();
	proto external float GetMovementType();
	//! Update animation about state of movement, define speed and direction in local space of character
	proto external void SetMovement(float type, vector movementDirLocal);
	//! set heading angle in radians
	proto external void SetHeadingAngle(float newHeadingAngle, bool adjustAimingYaw = false);
	proto external float GetHeadingAngle();
	//! set aiming angles in radians
	proto external void SetAimingAngles(float yaw, float pitch);
	//! Returns the current stance of the character.
	proto external ECharacterStance GetStance();
	//! Returns the current controlled character.
	proto external SCR_ChimeraCharacter GetCharacter();
	//! Set wanted input action values
	proto external void SetFireWeaponWanted(bool val);
	//! Set wanted input action values
	proto external void SetThrow(bool val, bool cancelThrow);
	//! Update simulation state with difference of world position
	proto external void SetMovementDirWorld(vector movementDirWorld);
	proto external vector GetMovementDirWorld();
	//! Set the current weapon-raised state.
	proto external void SetWeaponRaised(bool val);
	//! Set the current weapon ADS state.
	proto external void SetWeaponADS(bool val);
	//! Set the current play gesture state.
	proto external void SetPlayGesture(bool val);
	proto external void SetFreeLook(bool val);
	proto external void ResetPersistentStates(bool resetADSState = true, bool resetGadgetState = true);
	/*!
	Sets dynamic speed of this character.
	\param value Desired speed as percentage <0,1>.
	*/
	proto external void SetDynamicSpeed(float value);
	/*!
	Returns dynamic speed value.
	\return Dynamic speed value as <0, 1>.
	*/
	proto external float GetDynamicSpeed();
	/*!
	Sets dynamic stance of this character.
	\param value Desired stance height as percentage of full erect <0,1>.
	*/
	proto external void SetDynamicStance(float value);
	/*!
	Returns whether provided dynamic stance can be set for this character.
	\see CharacterControllerComponent::SetDynamicStance(float value)
	\param value Desired stance height as percentage of full erect <0,1>.
	*/
	proto external bool CanSetDynamicStance(float value);
	/*!
	Returns current dynamic stance value.
	\return Dynamic stance value as <0,1>.
	*/
	proto external float GetDynamicStance();
	/*!
	Enables or disables inspection mode.
	\param state Desired state true=enabled, disabled otherwise.
	*/
	proto external void SetInspectionMode(bool state);
	/*!
	Returns whether inspection mode can be set.
	\return True in case inspection mode can be set, false otherwise.
	*/
	proto external bool CanSetInspectionMode();
	/*!
	Returns whether character is in inspection mode.
	\return True in case character is in inspection mode, false otherwise.
	*/
	proto external bool GetIsInspectionMode();
	/*!
	Sets inspection state if inspection state is enabled.
	\param state Desired state.
	*/
	proto external void SetInspectionState(float state);
	/*!
	Returns inspection state if inspection state is enabled.
	\return Returns desired state.
	*/
	proto external float GetInspectionState();
	//! CharacterStanceChange STANCECHANGE_NONE = 0, STANCECHANGE_TOERECTED = 1, STANCECHANGE_TOCROUCH = 2, STANCECHANGE_TOPRONE = 3
	proto external void SetStanceChange(int stance);
	//! CharacterStanceChange STANCECHANGE_NONE = 0, STANCECHANGE_TOERECTED = 1, STANCECHANGE_TOCROUCH = 2, STANCECHANGE_TOPRONE = 3
	proto external bool CanChangeStance(int stance);
	proto external void ForceStance(int stance);
	proto external void ForceStanceUp(int stance);
	//! 2 - right, 1 - left
	proto external void SetRoll(int val);
	proto external void SetJump(float val);
	proto external void SetWantedLeaning(float val);
	proto external void SetBanking(float val);
	proto external void SetMeleeAttack(bool val);
	//! Either character wants to lean
	proto external float GetLeaning();
	proto external float GetWantedLeaning();
	proto external bool IsAdjustingLeaning();
	//! Returns current amount of leaning applied
	proto external float GetCurrentLeanAmount();
	proto external bool IsLeaning();
	proto external float GetADSTime();
	proto external bool IsWeaponRaised();
	proto external bool IsWeaponObstructed();
	proto external float GetObstructionAlpha();
	proto external bool IsClimbing();
	proto external bool IsSwimming();
	proto external bool IsSprinting();
	proto external bool IsChangingStance();
	proto external bool IsWeaponADS();
	proto external bool IsChangingFireMode();
	proto external bool IsPlayingGesture();
	proto external bool IsFreeLookEnabled();
	proto external bool IsTrackIREnabled();
	proto external bool IsFocusMode();
	proto external bool GetWeaponADSInput();
	proto external bool IsChangingItem();
	proto external bool IsFalling();
	proto external bool IsReloading();
	proto external bool CanFire();
	proto external bool IsDead();
	proto external bool IsUsingItem();
	proto external bool IsMeleeAttack();
	proto external bool CanEngageChangeItem();
	//! Set weapon on character with switching animations. If true, the request was successful
	proto external bool SelectWeapon(BaseWeaponComponent newWeapon);
	proto external bool SetMuzzle(int index);
	proto external bool SetFireMode(int index);
	proto external void SetSightsRange(int index);
	proto external void SetWeaponADSInput(bool val);
	/*!
	Set the safety of the current weapon.
	\param safety True to set the weapon safety.
	\param automatic True to set the automatic safety otherwise false to set manual safety.
	\return Returns true if the action has been successfull otherwise false.
	*/
	proto external bool SetSafety(bool safety, bool automatic);
	//Returns the current stamina value in <0, 1>. -1 if there is no stamina component attached to the current owner.
	proto external float GetStamina();
	//! Request weapon reload. If true, request was sucessful
	proto external bool ReloadWeapon();
	// mag or projectile
	proto external bool ReloadWeaponWith(IEntity ammunitionEntity);
	//! Dying
	proto external void Ragdoll();
	//! Kills the character. Skips invincibility checks.
	proto external void ForceDeath();
	/* @NOTE(Leo): Temp solution, eventually will be solved by setting respective gadget graph attachments,
	unfortunately current state of Enf animation system is not allowing it and will be solved not earlier then 10.07 by @Michal Žák
	from conversation with @Théo Escamez:
	so heres how it works :
	for now we have 4 items> compass adrianov, compass SY183, Radio ANPRC68 and Radio R148
	...where they are triggered respectively by integers 1 2 3 and 4...
	*/
	proto external void TakeGadgetInLeftHand(IEntity gadget, int gadgetType, bool autoFocus = false, bool skipAnimations = false);
	// Performs gadget equip validation
	proto external bool CanEquipGadget(IEntity gadget);
	proto external IEntity GetAttachedGadgetAtLeftHandSlot();
	/*!
	Try to use bandage on specified body part for a specific duration.
	\param bandage Bandage entity used for bandage action.
	\param bodyPart Body part where the bandage will be applied.
	\param duration Duration of the bandage in seconds.
	\param bandageSelf True if the player is bandaging himself.
	*/
	proto external bool TryUseBandage(IEntity bandage, int bodyPart, float duration = 4.0, bool bandageSelf = true);
	//! Remove held gadget
	proto external void RemoveGadgetFromHand(bool skipAnimations = false);
	//! Put held gadget on hold
	proto external void RecoverHiddenGadget(bool respectSettings, bool skipAnims);
	//! Returns true if there is a gadget in hands.
	proto external bool IsGadgetInHands();
	//! Returns true if focus will be changed to requested
	proto external bool SetGadgetRaisedModeWanted(bool newRaised);
	//! Returns true if character will be (or is) using gadget in raised mode
	proto external bool IsGadgetRaisedModeWanted();
	//! Generic item
	//! Equippes an item in right hand, if swap is true then action performed without animations, accepts optional callback that will be triggered when action is completed
	proto external bool TryEquipRightHandItem(IEntity item, EEquipItemType type, bool swap = false, BaseUserAction callbackAction = null);
	//! Returns generic item attached to right hand. Returns null if there's none (or if active item is weapon)
	proto external IEntity GetRightHandItem();
	/*!
	Try to use equipped item.
	\return Returns true if the equipped item has been used.
	*/
	proto external bool TryUseEquippedItem();
	/*!
	Try to use the specified item.
	\param item The item which should be used.
	\return Returns true if the item has been used.
	*/
	proto external bool TryUseItem(IEntity item);
	//! Returns true if the character can use an item.
	proto external bool CanUseItem();
	//! Starts character gesture with specified duration in milliseconds (if duration <= 0, it will be played until StopCharacterGesture is called)
	proto external bool TryStartCharacterGesture(int gesture, int durationMS = 0);
	proto external void StopCharacterGesture();
	/*!
	Try to play the specified gesture.
	\param gesture The gesture which should be played.
	\param callbackAction Optional UserAction as callback that will be called when target animation event is hit or gesture is complete
	\param confirmEvent
	\return Returns true if the gesture has been played.
	*/
	proto external bool TryPlayItemGesture(EItemGesture gesture, BaseUserAction callbackAction = null, string confirmEvent = "");
	//! Returns true if the character is playing a gesture.
	proto external bool IsPlayingItemGesture();
	//! Returns true if the character can play a gesture.
	proto external bool CanPlayItemGesture();
	//! Returns true if the character can use provided ladder.
	//! if optional maxTestDistance or maxEntryAngle is below 0, distance for test will be taken from character's ladder auto detection settings
	proto external bool CanUseLadder(LadderComponent pLadder, float maxTestDistance = -1.0, float maxEntryAngle = -1.0, bool performTraceCheck = false);
	//! Start climbing provided ladder.
	//! Returns true if request was successful.
	proto external bool TryUseLadder(LadderComponent pLadder, float maxTestDistance = -1.0, float maxEntryAngle = -1.0);
	// Script
	proto external void RequestActionByID(int actionID, float value);
	//! Returns true if the character is partially lowered.
	proto external bool IsPartiallyLowered();
	//! Returns the aiming angles in radians
	proto external vector GetAimingAngles();
	//! Returns the weapon angles in degrees
	proto external vector GetWeaponAngles();
	//! Returns the camera weapon angles in degrees
	proto external vector GetCameraWeaponAngles();
	//------------------------------------------------------------------------
	proto external vector GetCameraWeaponOffset();
	//! Returns the angular velocity in degrees/s
	proto external vector GetLookAtAngularVelocity();
	//------------------------------------------------------------------------
	proto external bool GetDisableMovementControls();
	//------------------------------------------------------------------------
	proto external void SetDisableMovementControls(bool other);
	//------------------------------------------------------------------------
	proto external bool GetDisableViewControls();
	//------------------------------------------------------------------------
	proto external void SetDisableViewControls(bool other);
	//------------------------------------------------------------------------
	proto external void SetAimingSensitivity(float mouse, float gamepad, float ads);
	// Sets gadget handling into persistent mode
	proto external void SetStickyGadget(bool enable);
	// Sets ads and gadget focus into persistent mode
	proto external void SetStickyADS(bool enable);
	//------------------------------------------------------------------------
	proto external void SetMaxZoomInADS(bool enable);
	//------------------------------------------------------------------------
	proto external bool GetMaxZoomInADS();
	//------------------------------------------------------------------------
	proto external bool GetDisableWeaponControls();
	//------------------------------------------------------------------------
	proto external void SetDisableWeaponControls(bool other);
	//------------------------------------------------------------------------
	proto external vector GetMovementVelocity();
	//------------------------------------------------------------------------
	proto external vector GetMovementInput();
	//------------------------------------------------------------------------
	proto external vector GetVelocity();
	//------------------------------------------------------------------------
	proto external bool GetIsSprintingToggle();
	//------------------------------------------------------------------------
	proto external float GetLastStanceChangeDelay();
	//------------------------------------------------------------------------
	proto external int GetCurrentMovementPhase();
	//------------------------------------------------------------------------
	proto external bool GetCanFireWeapon();
	//------------------------------------------------------------------------
	proto external bool GetCanThrow();
	//------------------------------------------------------------------------
	proto external void SetWeaponNoFireTime(float t);
	//------------------------------------------------------------------------
	proto external bool IsInThirdPersonView();
	// Caching third person view since profiling showed minor impact on cpu time when calling in to scripted method
	proto external void SetInThirdPersonView(bool state);
	proto external float GetStanceChangeDelayTime();
	proto external float GetJumpSpeed();
	proto external bool GetMeleeAttackInput();
	/*! Returns whether a position is in the character's view
	\param pos World Position to check is within view
	\param angMax Maximum(exclusive) angular offset in Degrees to consider the position within view
	*/
	proto external bool GetPositionInView(vector pos, float angMax);
	/*! Returns modifiers for weapon sway/recoil based on movement, stance, etc
	Fills scaleA and scaleB with following modifier values:
	scaleA.x - modifier scaleX
	scaleA.y - modifier scaleY
	scaleB.x - speed scale
	scaleB.y - translation scale
	*/
	[Obsolete("This method will be removed soon!")]
	proto external void GetWeaponModifiers(float baseScale, float moveScale, out vector scaleA, out vector scaleB);
	/*!
	Returns locally cached stamina component or null if none.
	*/
	proto external CharacterStaminaComponent GetStaminaComponent();
	// Check if character is not moving and not any other locomotion related action is being performed
	proto external bool IsCharacterStationary();
	
	// callbacks
	
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\poaram timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeslice);
	event protected void OnReloaded(IEntity owner, BaseWeaponComponent weapon);
	event protected void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player);
	event protected void OnApplyControls(IEntity owner, float timeSlice);
	//! Handling of melee events. Sends true if melee started, false, when melee ends
	event protected void OnMeleeDamage(bool started);
	event bool GetCanMeleeAttack() { return true; };
	//! Override to handle what happens after pressing F button, return false to use default cpp behavior
	event protected bool OnPerformAction() { return false; };
	//! Override to handle whether character can get out of vehicle via GetOut input action
	event bool CanGetOutVehicleScript() { return true; };
	//! Handling of death
	event protected void OnDeath(IEntity instigator);
	//! Will be called when gadget taken/removed from hand
	event protected void OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround);
	//! Will be called when gadget fully transitioned to or canceled focus mode
	event protected void OnGadgetFocusStateChanged(IEntity gadget, bool isFocused);
	//! Will be called when item use action is started
	event protected void OnItemUseBegan(IEntity item);
	//! Will be called when item use action is complete
	event protected void OnItemUseComplete(IEntity item);
	event protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd);
	//! Called when a player has been assigned to this controller
	event protected void OnControlledByPlayer(IEntity owner, bool controlled);
};

/** @}*/
