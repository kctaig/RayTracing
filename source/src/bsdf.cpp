#include "bsdf.hpp"

float LambertianDiffuseBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	return abs(dot(n, wi)) / M_PI;
}

vec3 LambertianDiffuseBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	return radiance / M_PI;
}

vec3 LambertianDiffuseBRDF::sampleDir(const vec3& wo, const vec3& n) const
{
	//float u = genRandomFloat();
	//float v = genRandomFloat();
	//// float z = std::fabs(1.0f - 2.0f * u);
	//float z = std::sqrt(1- u * u);
	//float r = std::sqrt(1.0f - z * z);
	//float phi = 2.0f * M_PI * v;
	//vec3 localDir = vec3(r * cos(phi), r * sin(phi), z);
	//return toWorld(localDir, n);

	vec2 disk;
	vec2 offset = 2.f * vec2(genRandomFloat(), genRandomFloat()) - vec2(1, 1);
	if (offset.x == 0 && offset.y == 0)  disk = vec2(0);
	float theta, r;
	if (abs(offset.x) > abs(offset.y)) {
		r = offset.x;
		theta = M_PI / 4.f * (offset.y / offset.x);
	}
	else {
		r = offset.y;
		theta = M_PI / 2.f - M_PI / 4.f * (offset.x / offset.y);
	}
	disk =  r * vec2(cos(theta), sin(theta));
	float z = sqrt(1 - disk.x * disk.x - disk.y * disk.y);
	if (z < 0.f) z = -z;
	vec3 localDir = vec3(disk.x, disk.y, z);
	return toWorld(localDir, n);
}

float PhongSpecularBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	vec3 h = normalize(wi - wo);
	float nh = dot(n, h);
	return (alpha + 1.0f) * pow(nh, alpha) / (2.0f * M_PI);
}

vec3 PhongSpecularBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	vec3 h = normalize(wi - wo);
	float nh = dot(h, n);
	float spec = pow(nh, alpha);
	float normFactor = (alpha + 2.0f) / (2.0f * M_PI);
	return radiance * spec * normFactor;
}

vec3 PhongSpecularBRDF::sampleDir(const vec3& wo, const vec3& n) const
{
	float u = genRandomFloat();
	float v = genRandomFloat();

	float phi = 2.0f * M_PI * u;
	float cosTheta = pow(v, 1.0f / (alpha + 1.0f));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	vec3 h = vec3(
		sinTheta * cos(phi),
		sinTheta * sin(phi),
		cosTheta
	);

	h = toWorld(h, n);

	vec3 wi = wo - 2.0f * dot(wo, h) * h;

	return wi;
}

float MirrorSpecularBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	return 1.0f;
}

vec3 MirrorSpecularBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	float cosTheta = dot(wi, n);
	vec3 fresnelSchlick = radiance + (vec3(1.0f) - radiance) * pow(1.0f - cosTheta, 5.0f);
	return fresnelSchlick / cosTheta;
}

vec3 MirrorSpecularBRDF::sampleDir(const vec3& wo, const vec3& n) const
{
	return normalize(wo - 2.0f * dot(wo, n) * n);
}

void BSDF::generateBSDFWeight()
{
	vec3 cie1931Weights(0.212671f, 0.715160f, 0.072169f);
	float sum = 0.f;
	vector<float> luminances;
	for (shared_ptr<BxDF>& bxdfPtr : bxdfPtrs) {
		float luminance = dot(bxdfPtr->getRadiance(), cie1931Weights);
		sum += luminance;
		luminances.push_back(luminance);
	}
	if (sum == 0.f) return;
	for (int i = 0; i < luminances.size(); i++) {
		bxdfPtrs[i]->setWeight(luminances[i] / sum);
	}
}

bool BSDF::sampleBSDF(const vec3& wo, const vec3& n)
{
	if (bxdfPtrs.empty()) return false;
	wi = sampelDir(wo, n);
	// judge the wi_dir is valid or not
	if (dot(wi, n) < EPSILON) {
		return false;
	}
	BSDFeval = eval(wo, wi, n);
	BSDFpdf = pdf(wo, wi, n);
	return true;
}

vec3 BSDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	vec3 res = vec3(0);
	for (shared_ptr<BxDF> bxdfPtr : bxdfPtrs) {
		res += bxdfPtr->eval(wo, wi, n);
	}
	return res;
}

float BSDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	float res = 0.f;
	for (shared_ptr<BxDF> bxdfPtr : bxdfPtrs) {
		res += bxdfPtr->pdf(wo, wi, n) * bxdfPtr->getWeight();
	}
	return res;
}

vec3 BSDF::sampelDir(const vec3& wo, const vec3& n) const
{
	std::vector<float> weiPreSum(bxdfPtrs.size());
	for (int i = 0; i < bxdfPtrs.size(); i++)
	{
		if (i == 0) weiPreSum[i] = bxdfPtrs[i]->getWeight();
		else weiPreSum[i] = weiPreSum[i - 1] + bxdfPtrs[i]->getWeight();
	}
	float randomValue = genRandomFloat() * weiPreSum.back();
	int bxdf_index = 0;
	while (bxdf_index < bxdfPtrs.size() && weiPreSum[bxdf_index] < randomValue) {
		bxdf_index++;
	}
	return bxdfPtrs[bxdf_index]->sampleDir(wo, n);
}
