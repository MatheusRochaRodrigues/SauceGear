#pragma once 
#include "../SDF/SignedDistanceField.h"
#include "../SDF/Terrain.h"

class Map {
public:
	SignedDistanceField* sdf;

	Map() {
		sdf = new Terrain();
	};
	~Map () = default;
	 
};
