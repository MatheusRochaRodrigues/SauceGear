#pragma once

class Shader; 

namespace UniformLights {
	//--------------------  utilidades para enviar dados aos shaders   
	void SetLightsToShader(Shader* shader);
	void SetPointsToShader(Shader* shader, int i = 0);
	int  SetSunToShader   (Shader* shader);
	void set_uShadowData  (Shader* lightingShader, int unit = 10); 
}
