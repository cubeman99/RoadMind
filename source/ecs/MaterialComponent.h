#pragma once

#include <cmgGraphics/cmg_graphics.h>
#include <cmgMath/cmg_math.h>
#include <map>
#include <vector>
#include "CommonTypes.h"


struct MaterialComponent : public ECSComponent<MaterialComponent>
{
	Material::sptr material;
};


