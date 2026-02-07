
// *************************************************************************************
// ! CharacterCamera1stPersonVehicle - 1st person camera when character is in vehicle
// *************************************************************************************
class CharacterCamera1stPersonVehicle extends CharacterCamera1stPerson
{
	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonVehicle(CameraHandlerComponent pCameraHandler)
	{
		m_ApplyHeadBob = false;
	}
		
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		if (pPrevCamera)
		{
			m_fLeftRightAngle = 0.0;
			m_fUpDownAngle = 0.0;
		}
		
		CharacterCamera3rdPersonVehicle characterCamera3rdPersonVehicle  = CharacterCamera3rdPersonVehicle.Cast(pPrevCamera);
		if (characterCamera3rdPersonVehicle)
		{
			m_fRollSmooth = characterCamera3rdPersonVehicle.m_fRollSmooth;
			m_fRollSmoothVel = characterCamera3rdPersonVehicle.m_fRollSmoothVel;
			m_fPitchSmooth = characterCamera3rdPersonVehicle.m_fPitchSmooth;
			m_fPitchSmoothVel = characterCamera3rdPersonVehicle.m_fPitchSmoothVel;
		}

		if (m_pCompartmentAccess)
		{
			BaseCompartmentSlot compartment = m_pCompartmentAccess.GetCompartment();
			if (compartment)
			{
				ForceFreelook(compartment.GetForceFreeLook());
				
				m_OwnerVehicle = compartment.GetOwner();
				
				if (m_OwnerVehicle)
				{
					SCR_VehicleCameraDataComponent vehicleCamData = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
					if( vehicleCamData )
					{
						m_fRollFactor = vehicleCamData.m_fRollFactor;
						m_fPitchFactor = vehicleCamData.m_fPitchFactor;
					}
				}
			}
		}
		
		m_bCameraTransition = false;
	}

	//-----------------------------------------------------------------------------
	
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);
		pOutResult.m_fUseHeading = 0.0;
		
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, pOutResult.m_CameraTM);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
	}
	
	//-----------------------------------------------------------------------------	
	override float GetBaseFOV()
	{
		return GetGame().GetCameraManager().GetVehicleFOV();
	}
	
	private IEntity m_OwnerVehicle;
};
// *************************************************************************************
// ! CharacterCamera1stPersonVehicleTransition - 1st person camera when character is getting in/out vehicle
// ************************************************************************************
class CharacterCamera1stPersonVehicleTransition extends CharacterCamera1stPersonVehicle
{
		//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		m_bCameraTransition = true;
	}

};
