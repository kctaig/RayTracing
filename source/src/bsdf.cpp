#include "bsdf.hpp"

float LambertianBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	float cos_theta = std::max(EPSILON, dot(n, wi));
	return cos_theta / M_PI;
}

vec3 LambertianBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	return radiance / M_PI;
}

vec3 LambertianBRDF::sampleDir(const vec3& wo, const vec3& n) const
{
	//float u = genRandomFloat();
	//float v = genRandomFloat();
	//float z = std::fabs(1.0f - 2.0f * u);
	//float r = std::sqrt(1.0f - z * z);
	//float phi = 2.0f * M_PI * v;
	//vec3 localDir = vec3(r * cos(phi), r * sin(phi), z);
	//return toWorld(localDir, n);

	float x_1 = genRandomFloat();
	float x_2 = genRandomFloat();
	float z = std::fabs(1.0f - 2.0f * x_1);
	float r = std::sqrt(1.0f - z * z);
	float phi = 2 * M_PI * x_2;
	vec3 local_dir(r * std::cos(phi), r * std::sin(phi), z);
	vec3 tangent1, tangent2;
	// 选择一个与 N 不平行的向量，这里使用 (1, 0, 0)，假设 N 不与 (1, 0, 0) 重合
	vec3 arbitrary = fabs(n.x > 0.99f) ? vec3(0, 1, 0) : vec3(1, 0, 0);
	tangent1 = normalize(cross(n, arbitrary));
	tangent2 = normalize(cross(n, tangent1));
	return local_dir.x * tangent1 + local_dir.y * tangent2 + local_dir.z * n;
}

float SpecularBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	vec3 h = normalize(wi + wo);
	float nh = dot(n, h);
	if (nh <= 0.0f) return 0.0f;
	return (alpha + 1.0f) * pow(nh, alpha) / (2.0f * M_PI);
}

vec3 SpecularBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	vec3 h = normalize(wi + wo);
	float nh = std::max(dot(h, n), 0.0f);
	float spec = pow(nh, alpha);
	float cosTheta = dot(wi, n);
	if (cosTheta < 0.0f) return vec3(0.0f);
	return radiance * spec / cosTheta;
}

vec3 SpecularBRDF::sampleDir(const vec3& wo, const vec3& n) const
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

	vec3 wi = reflect(wo, h);
	return normalize(wi);
}

vec3 SpecularBRDF::reflect(const vec3& wo, const vec3& n) const
{
	return 2.0f * dot(wo, n) * n - wo;
}

void BSDF::generateWeight()
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

void BSDF::sampleBSDF(const vec3& wo_dir, const vec3& n)
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
	bxdf_index = std::min(bxdf_index, static_cast<int>(bxdfPtrs.size()) - 1);
	wi_dir = bxdfPtrs[bxdf_index]->sampleDir(wo_dir, n);
	eval = bxdfPtrs[bxdf_index]->eval(wo_dir, wi_dir, n);
	pdf = bxdfPtrs[bxdf_index]->pdf(wo_dir, wi_dir, n) * bxdfPtrs[bxdf_index]->getWeight();
	// add other contributation
	for (int i = 0; i < bxdfPtrs.size(); i++) {
		if (i == bxdf_index) continue;
		eval += bxdfPtrs[i]->eval(wo_dir, wi_dir, n);
		pdf += bxdfPtrs[i]->pdf(wo_dir, wi_dir, n) * bxdfPtrs[i]->getWeight();
	}
}