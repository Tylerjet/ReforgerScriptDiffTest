class SCR_ResourceSystemHelper
{
	//~ How many decimals are displayed when showing supplies. Use SCR_ResourceSystemHelper.SuppliesToString(supplies) to ignore any decimals ending in 0
	static const int DECIMALS_SUPPLIES = 1;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceType Resource type
	//! \return If supplies are enabled or not
	static bool IsGlobalResourceTypeEnabled(EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return true;
		
		return baseGameMode.IsResourceTypeEnabled(resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the amount of resources stored in the resource component
	\param resourceComponent ResourceComponent to get stored resources from
	\param[out] storedRescources How many resources were found
	\param resourceType Type of resources to find
	\return Returns true if resource storage were found (even if stored resources is 0) returns false if no valid ResourceComponent or ResourceComponent setup
	*/
	static bool GetStoredResources(notnull SCR_ResourceComponent resourceComponent, out float storedRescources, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer; 
		storedRescources = 0;
		
		if (resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType, consumer))
		{
			storedRescources = consumer.GetAggregatedResourceValue();
			return true;
		}
			
		
		if (resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType, consumer))
		{
			storedRescources = consumer.GetAggregatedResourceValue();
			return true;
		}
		
		SCR_ResourceGenerator generator;
		if (resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, resourceType, generator))
		{
			storedRescources = generator.GetAggregatedResourceValue();
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Function to unify the way supplies are displayed in UI
	\param supplies Supplies float to be converted to string
	\return Float converted to supplies with the correct decimals
	*/
	static string SuppliesToString(float supplies)
	{
		return SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(supplies, DECIMALS_SUPPLIES);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the amount of resources stored as well as the max amount that can be stored in the resource component
	\param resourceComponent ResourceComponent to get stored resources from
	\param[out] Current stored resources (0 if no valid resources are found)
	\param[out] Max stored resources (0 if no valid resources are found)
	\param resourceType Type of resources to find
	\return Returns true if resource storage were found (even if stored resources is 0) returns false if no valid ResourceComponent or ResourceComponent setup or if max resource amount is 0
	*/
	static bool GetStoredAndMaxResources(notnull SCR_ResourceComponent resourceComponent, out float totalResources, out float maxResources, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer; 
		
		totalResources = 0;
		maxResources = 0;
		
		if (resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType, consumer))
		{
			maxResources = consumer.GetAggregatedMaxResourceValue();
			totalResources = consumer.GetAggregatedResourceValue();
			
			if (maxResources > 0)
				return true;
		}
			
		if (resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType, consumer))
		{
			maxResources = consumer.GetAggregatedMaxResourceValue();
			totalResources = consumer.GetAggregatedResourceValue();
		
			if (maxResources > 0)
				return true;
		}
		
		SCR_ResourceGenerator generator;
		if (resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, resourceType, generator))
		{
			maxResources = generator.GetAggregatedMaxResourceValue();
			totalResources = generator.GetAggregatedResourceValue();
			
			if (maxResources > 0)
				return true;
		}
		
		//~ Reset again
		totalResources = 0;
		maxResources = 0;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the amount of resources that are available to use for the given resource component
	\param resourceComponent ResourceComponent to get available resources from
	\param[out] availableResources How many resources are available
	\param resourceID Type of resources it checks. By default this is DEFAULT
	\param resourceType Type of resources to find
	\return Returns true if availabe resources were found (even if available resources is 0) returns false if no valid ResourceComponent or ResourceComponent setup
	*/
	static bool GetAvailableResources(notnull SCR_ResourceComponent resourceComponent, out float availableResources, EResourceGeneratorID resourceID = EResourceGeneratorID.DEFAULT, EResourceType resourceType = EResourceType.SUPPLIES)
	{		
		SCR_ResourceConsumer consumer;
		availableResources = 0;
		
		if (resourceComponent.GetConsumer(resourceID, resourceType, consumer))
		{
			availableResources = consumer.GetAggregatedResourceValue();
			return true;
		}
			
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the storage consumer from the given ResourceCompoment
	\param resourceComponent ResourceComponent to get consumer from
	\param resourceType Type of resources the consumer has
	\return Found consumer
	*/
	static SCR_ResourceConsumer GetStorageConsumer(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		if (consumer)
			return consumer;
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		if (consumer)
			return consumer;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the consumer for that will grant you access to the available resources
	\param resourceComponent ResourceComponent to get consumer from
	\param resourceType Type of resources the consumer has
	\return Found consumer
	*/
	static SCR_ResourceConsumer GetAvailableResourceConsumer(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		return resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the first valid consumer that could be found
	\param resourceComponent ResourceComponent to get consumer from
	\param resourceType Type of resources the consumer has
	\return Found consumer
	*/
	static SCR_ResourceConsumer GetFirstValidConsumer(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		if (consumer)
			return consumer;
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		if (consumer)
			return consumer;
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType);
		if (consumer)
			return consumer;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the first valid generator that could be found
	\param resourceComponent ResourceComponent to get generator from
	\param resourceType Type of resources the generator has
	\return Found generator
	*/
	static SCR_ResourceGenerator GetFirstValidGenerator(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceGenerator generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		if (generator)
			return generator;
		
		generator = resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, resourceType);
		if (generator)
			return generator;
		
		generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, resourceType);
		if (generator)
			return generator;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static function to make sure that all refunds are treated the same way
	//! return Correctly rounded refund amount
	static float RoundRefundSupplyAmount(float refundCost)
	{
		return Math.Ceil(refundCost);
	}
}
