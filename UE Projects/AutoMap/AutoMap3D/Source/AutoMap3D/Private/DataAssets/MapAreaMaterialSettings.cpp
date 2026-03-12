#include "DataAssets/MapAreaMaterialSettings.h"

// Public Functions --------------------------------------------------------------------------------------------

UMaterialInterface* UMapAreaMaterialSettings::GetUnwrapMaterial()
{
	return UnwrapMaterial;
}

UMaterialInterface* UMapAreaMaterialSettings::GetTerrainMaterial()
{
	return TerrainMaterial;
}

UMaterialInterface* UMapAreaMaterialSettings::GetAccentMaterial()
{
	return AccentMaterial;
}

UMaterialInterface* UMapAreaMaterialSettings::GetMapProxyMaterial()
{
	return MapProxyMaterial;
}

//--------------------------------------------------------------------------------------------