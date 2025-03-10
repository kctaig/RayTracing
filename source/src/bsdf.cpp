#include "bsdf.hpp"

float LambertianBRDF::pdf() const
{
	return 0.5f / M_PI;
}

vec3 LambertianBRDF::f(const vec3& wo, const vec3& wi, const vec3& n) const
{
	return radiance / M_PI;
}

vec3 LambertianBRDF::sampleDir(const vec3& wi, const vec3& n) const
{
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

float SpecularBRDF::pdf() const
{
	return 0.0f;
}

vec3 SpecularBRDF::f(const vec3& wo, const vec3& wi, const vec3& n) const
{
	return vec3();
}

vec3 SpecularBRDF::sampleDir(const vec3& wi, const vec3& n) const
{
	return vec3();
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
	int bxdf_id = static_cast<int>(genRandomFloat() * bxdfPtrs.size());
	wi_dir = bxdfPtrs[bxdf_id]->sampleDir(wo_dir, n);
	f = bxdfPtrs[bxdf_id]->f(wo_dir, wi_dir, n);
	f *= bxdfPtrs[bxdf_id]->getWeight();
	pdf = bxdfPtrs[bxdf_id]->pdf();
	// add other contributation
	for (int i = 0; i < bxdfPtrs.size(); i++) {
		if (i == bxdf_id) continue;
		vec3 f_i = bxdfPtrs[i]->f(wo_dir, wi_dir, n);
		float pdf_i = bxdfPtrs[i]->pdf();
		f += f_i * bxdfPtrs[i]->getWeight();
		pdf += pdf_i * bxdfPtrs[i]->getWeight();
	}
}