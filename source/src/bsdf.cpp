#include "bsdf.hpp"

float LambertianBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const
{
	float cosalpha = dot(wi, n);
	if (cosalpha > 0.0f)
		return 0.5f / M_PI;
	else return EPSILON;
}

vec3 LambertianBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const
{
	float cosalpha = dot(wi, n);
	if (cosalpha > 0.0f)
		return radiance / M_PI;
	else return vec3(0.0f);
}

vec3 LambertianBRDF::sampleDir(const vec3& wo, const vec3& n) const
{
	float u = genRandomFloat();
	float v = genRandomFloat();
	float z = std::fabs(1.0f - 2.0f * u);
	float r = std::sqrt(1.0f - z * z);
	float phi = 2.0f * M_PI * v;
	vec3 localDir = vec3(r * cos(phi), r * sin(phi), z);
	return toWorld(localDir, n);
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
	float spec = pow(std::max(dot(h, n), 0.0f), alpha);
	float cosTheta = dot(wi, n);
	if (cosTheta < 0.0f) return vec3(0.0f);
	return radiance * spec / cosTheta;
}

vec3 SpecularBRDF::sampleDir(const vec3& wo, const vec3& n) const
{
	// 生成随机数
	float u = genRandomFloat();
	float v = genRandomFloat();

	// 计算半程向量的球面坐标
	float phi = 2.0f * M_PI * u;
	float cosTheta = pow(v, 1.0f / (alpha + 1.0f));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

	// 将球面坐标转换为笛卡尔坐标
	vec3 h = vec3(
		sinTheta * cos(phi),
		sinTheta * sin(phi),
		cosTheta
	);

	// 将半程向量转换到局部坐标系
	h = toWorld(h, n);

	// 计算反射方向
	vec3 wi = reflect(wo, h);

	return wi;
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
	int bxdf_id = static_cast<int>(genRandomFloat() * bxdfPtrs.size());
	wi_dir = bxdfPtrs[bxdf_id]->sampleDir(wo_dir, n);
	eval = bxdfPtrs[bxdf_id]->eval(wo_dir, wi_dir, n);
	pdf = bxdfPtrs[bxdf_id]->pdf(wo_dir, wi_dir, n) * bxdfPtrs[bxdf_id]->getWeight();
	// add other contributation
	for (int i = 0; i < bxdfPtrs.size(); i++) {
		if (i == bxdf_id) continue;
		eval += bxdfPtrs[i]->eval(wo_dir, wi_dir, n);
		pdf += bxdfPtrs[i]->pdf(wo_dir, wi_dir, n) * bxdfPtrs[i]->getWeight();
	}
}