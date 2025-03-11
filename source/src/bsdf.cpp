#include "bsdf.hpp"

float LambertianBRDF::pdf(const vec3& wi, const vec3& n) const
{
	float cos_theta = std::max(EPSILON, dot(n, wi));
	return cos_theta / M_PI;
}

vec3 LambertianBRDF::f(const vec3& wo, const vec3& wi, const vec3& n) const
{
	//float cos_theta1 = dot(wo, n);
	//float cos_theta2 = dot(wi, n);
	//if (cos_theta1 > 0.f || cos_theta2 < 0.f) return vec3(0);
	return radiance / M_PI;
}

vec3 LambertianBRDF::sampleDir(const vec3& wi, const vec3& n) const
{
	 // 在单位圆盘上生成随机点
	float u = genRandomFloat(); // 随机数 [0, 1)
	float v = genRandomFloat(); // 随机数 [0, 1)
	// 将随机点映射到半球上（余弦加权采样）
	float phi = 2.0f * M_PI * u;
	float cosTheta = sqrt(v);
	float sinTheta = sqrt(1.0f - v);	
	// 生成局部坐标系中的方向
	vec3 localDir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
	// 将局部方向转换到世界坐标系
	vec3 arbitrary = (std::abs(n.x) > 0.9f ? vec3(0, 1, 0) : vec3(1, 0, 0));
	vec3 tangent = normalize(cross(n, arbitrary));
	vec3 bitangent = cross(n, tangent);
	return localDir.x * tangent + localDir.y * bitangent + localDir.z * n;
}

float SpecularBRDF::pdf(const vec3& wi, const vec3& n) const
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
	pdf = bxdfPtrs[bxdf_id]->pdf(wi_dir, n) * bxdfPtrs[bxdf_id]->getWeight();
	// add other contributation
	for (int i = 0; i < bxdfPtrs.size(); i++) {
		if (i == bxdf_id) continue;
		f += bxdfPtrs[i]->f(wo_dir, wi_dir, n);
		pdf += bxdfPtrs[i]->pdf(wi_dir, n) * bxdfPtrs[i]->getWeight();
	}
}