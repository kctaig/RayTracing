#include "payload.hpp"
#include "material.hpp"
#include "Model.hpp"

void PayLoad::initBxDFs()
{
	bsdfPtr = make_shared<BSDF>();
	shared_ptr<Material> matPtr = meshPtr->matPtr;
	vec3 specular = matPtr->specular;
	float shininess = matPtr->shininess;
	vec3 diffuse = matPtr->diffuse;
	vec2 texCoord = meshPtr->getTexCoord(uv);
	vec3 texRadiance = matPtr->getDiffuse(texCoord);

	if (shininess >= 10000) {
		bsdfPtr->perfectSpecular = true;
		bsdfPtr->bxdfPtrs.push_back(make_shared<MirrorSpecularBRDF>(specular));
	}
	else {
		if (glm::length(diffuse) > EPSILON)
			bsdfPtr->bxdfPtrs.push_back(make_shared<LambertianDiffuseBRDF>(diffuse));
		if (glm::length(specular) > EPSILON)
			bsdfPtr->bxdfPtrs.push_back(make_shared<PhongSpecularBRDF>(specular, shininess));
		if (matPtr->useTexture) {
			if (shininess > 1.0f)
				bsdfPtr->bxdfPtrs.push_back(make_shared<PhongSpecularBRDF>(texRadiance, shininess));
			else
				bsdfPtr->bxdfPtrs.push_back(make_shared<LambertianDiffuseBRDF>(texRadiance));
		}
	}
	bsdfPtr->generateBSDFWeight();
}