class LightUserAction : BaseLightUserAction
{
	//! Will only be shown if user is in vehicle?
	[Attribute( uiwidget: UIWidgets.CheckBox)]
	protected bool m_bInteriorOnly;
	
	//! Will action be available for entities seated in pilot compartment only?
	[Attribute( uiwidget: UIWidgets.CheckBox, defvalue: "1")]
	protected bool m_bPilotOnly;
	
	//! Stores Contec name for sound position offset
	[Attribute( uiwidget: UIWidgets.EditBox)]
	protected string  m_sParentCtx;
	
	protected IEntity m_pOwner;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_pOwner = pOwnerEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		// See if character is in vehicle
		if (character && character.IsInVehicle())
		{
			// See if character is in "this" (owner) vehicle
			CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(character.FindComponent(CompartmentAccessComponent));
			if (compartmentAccess)
			{
				// Character is in compartment
				// that belongs to owner of this action
				BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
				if (slot)
				{
					// Check pilot only condition
					if (m_bPilotOnly && !PilotCompartmentSlot.Cast(slot))
						return false;
					
					// Check interior only condition
					if (m_bInteriorOnly && slot.GetOwner() != m_pOwner)
						return false;
					
					return true;
				}
			}
			
			return false;
		}
		// We cannot be pilot nor interior, if we are not seated in vehicle at all.
		else if (m_bInteriorOnly || m_bPilotOnly)
			return false;
		
		BaseLightManagerComponent lightManager = GetLightManager();
		if (!lightManager)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{		
		BaseLightManagerComponent lightManager = GetLightManager();
		if (!lightManager)
			return false;
				
		UIInfo actionInfo = GetUIInfo();
		if (!actionInfo)
			return false;
		
		string selfName = actionInfo.GetName();
		outName = selfName + "%CTX_HACK%";
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetLightAudioPos(out vector pos)
	{		
		if (!m_sParentCtx.IsEmpty())
		{
			ActionsManagerComponent actionManagerComponent = ActionsManagerComponent.Cast(m_pOwner.FindComponent(ActionsManagerComponent));				
			if (actionManagerComponent)
			{
				UserActionContext context = actionManagerComponent.GetContext(m_sParentCtx);
				
				if (context != null)
				{
					vector mat[4]
					context.GetTransformationModel(mat);
					pos = mat[3];
					return true;
				}
			}
		}
		
		return false;
	}
	
	void PlaySound(IEntity ownerEntity, bool lightsState)
	{
		// Sound		
		GenericEntity genericOwnerEntity = GenericEntity.Cast(ownerEntity);
		SoundComponent soundComponent = SoundComponent.Cast(genericOwnerEntity.FindComponent(SoundComponent));
		if (soundComponent)
		{

			vector offset;
			bool haveOffset = GetLightAudioPos(offset);
			
			if (haveOffset)
			{				
				if (lightsState)
				{
					soundComponent.SoundEventOffset("SOUND_VEHICLE_CLOSE_LIGHT_ON", offset);				
				}
				else
				{
					soundComponent.SoundEventOffset("SOUND_VEHICLE_CLOSE_LIGHT_OFF", offset);
				}
			}
			else
			{
				if (lightsState)
				{
					soundComponent.SoundEvent("SOUND_VEHICLE_CLOSE_LIGHT_ON");				
				}
				else
				{
					soundComponent.SoundEvent("SOUND_VEHICLE_CLOSE_LIGHT_OFF");
				}
			}			
		}
	}
};