# BxDF 详解

## 目录

1. [BxDF 基本概念](#bxdf-基本概念)
2. [LambertianDiffuseBRDF（漫反射）](#lambertiandiffusebrdf漫反射)
3. [PhongSpecularBRDF（高光反射）](#phongspecularbrdf高光反射)
4. [MirrorSpecularBRDF（镜面反射）](#mirrorspecularbrdf镜面反射)
5. [BSDF 混合机制](#bsdf-混合机制)
6. [在渲染中的应用](#在渲染中的应用)

---

## BxDF 基本概念

### 什么是 BxDF？

BxDF（Bidirectional Reflectance Distribution Function，双向反射分布函数）描述了**光线如何在表面上反射**的物理规律。

**物理意义**：给定入射方向 `wo` 和出射方向 `wi`，BxDF告诉我们：
- 有多少光线从 `wo` 方向反射到 `wi` 方向
- 反射光线的颜色和强度

### BxDF 的三个核心函数

```cpp
class BxDF {
    // 1. PDF：概率密度函数
    virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
    
    // 2. eval：BRDF值（反射率）
    virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
    
    // 3. sampleDir：采样反射方向
    virtual vec3 sampleDir(const vec3& wo, const vec3& n) const = 0;
};
```

### 渲染方程

$$
L_o(p, \omega_o) = L_e(p, \omega_o) + \int_{\Omega} f_r(p, \omega_i, \omega_o) L_i(p, \omega_i) (\omega_i \cdot n) \, d\omega_i
$$

**蒙特卡洛近似**：

$$
L_o(p, \omega_o) \approx L_e(p, \omega_o) + \frac{1}{N} \sum_{i=1}^{N} \frac{f_r(p, \omega_i, \omega_o) L_i(p, \omega_i) (\omega_i \cdot n)}{p(\omega_i)}
$$

其中：
- $L_o$：出射辐射亮度
- $L_e$：自发光辐射亮度
- $f_r$：BRDF（双向反射分布函数）
- $L_i$：入射辐射亮度
- $p(\omega_i)$：采样方向的概率密度函数（PDF）

---

## LambertianDiffuseBRDF（漫反射）

### 物理原理

漫反射遵循**兰伯特余弦定律**：反射光强度与入射角的余弦成正比。

$$
\text{反射光强} \propto \cos(\theta) = \mathbf{n} \cdot \omega_i
$$

### 1. PDF 计算

```cpp
float pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    if (dot(n, wi) < 0.f) return 0.f;  // 背面，无效
    return 1.0f / M_PI;  // 投影面积空间的均匀分布
}
```

**为什么是 `1/π`？**

- 半球在平面上的投影面积是 `π`
- 在投影面积空间中均匀采样
- 与BRDF的归一化一致

**能量守恒验证**：

$$
\int_{\Omega} \frac{1}{\pi} (\mathbf{n} \cdot \omega_i) \, d\omega_i = 1
$$

### 2. BRDF 评估

```cpp
vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    return radiance / M_PI;
}
```

**兰伯特BRDF公式**：

$$
f_r(\omega_i, \omega_o) = \frac{\rho}{\pi}
$$

其中：
- $\rho$ 是反照率（albedo），即材质颜色
- $\pi$ 是归一化因子

**能量守恒验证**：

$$
\int_{\Omega} \frac{\rho}{\pi} (\mathbf{n} \cdot \omega_i) \, d\omega_i = \rho
$$

计算积分：

$$
\int_{\Omega} (\mathbf{n} \cdot \omega_i) \, d\omega_i = \int_0^{2\pi} \int_0^{\pi/2} \cos(\theta) \sin(\theta) \, d\theta d\phi = \pi
$$

所以：

$$
\frac{\rho}{\pi} \times \pi = \rho \quad \checkmark \text{ 能量守恒}
$$

### 3. 方向采样

```cpp
vec3 sampleDir(const vec3& wo, const vec3& n) const {
    // 在单位圆盘上均匀采样
    vec2 offset = 2.f * vec2(genRandomFloat(), genRandomFloat()) - vec2(1, 1);
    
    // 将正方形映射到圆盘（平方根映射）
    vec2 disk;
    if (abs(offset.x) > abs(offset.y)) {
        float r = offset.x;
        float theta = M_PI / 4.f * (offset.y / offset.x);
        disk = r * vec2(cos(theta), sin(theta));
    } else {
        float r = offset.y;
        float theta = M_PI / 2.f - M_PI / 4.f * (offset.x / offset.y);
        disk = r * vec2(cos(theta), sin(theta));
    }
    
    // 投影到半球
    float z = sqrt(1 - disk.x * disk.x - disk.y * disk.y);
    vec3 localDir = vec3(disk.x, disk.y, z);
    
    // 转换到世界坐标系
    return toWorld(localDir, n);
}
```

**采样步骤**：

1. **在单位圆盘上均匀采样**：使用平方根映射避免中心聚集
2. **投影到半球**：$z = \sqrt{1 - x^2 - y^2}$
3. **转换到世界坐标**：使用切线空间变换

**为什么这样采样？**

- 确保在半球上均匀分布
- PDF = `1/π`
- 避免拒绝采样，效率高

### 漫反射特点

| 特性 | 值 |
|------|-----|
| **PDF** | `1/π` |
| **BRDF** | `radiance / π` |
| **采样** | 圆盘投影到半球 |
| **能量守恒** | 完全抵消 |
| **应用场景** | 纸张、墙壁、布料等粗糙表面 |

---

## PhongSpecularBRDF（高光反射）

### 物理原理

Phong模型是一种**经验模型**，用于模拟光滑表面的镜面反射。它基于**半程向量**（halfway vector）来计算高光强度。

**核心思想**：
- 当观察方向与完美反射方向接近时，高光强度最大
- 使用指数参数控制高光的集中程度

### 半程向量

半程向量是入射方向和出射方向的角平分线：

$$
\mathbf{h} = \frac{\omega_i - \omega_o}{\|\omega_i - \omega_o\|}
$$

**几何意义**：
- 当 $\omega_i$ 是完美反射方向时，$\mathbf{h}$ 与法向量 $\mathbf{n}$ 重合
- $\mathbf{n} \cdot \mathbf{h}$ 越接近1，高光越强

### 1. PDF 计算

```cpp
float pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    if (dot(n, wi) < 0.f) return 0.f;  // 背面，无效
    const vec3 h = normalize(wi - wo);  // 半程向量
    const float nh = dot(n, h);
    return (alpha + 1.0f) * pow(nh, alpha) / (2.0f * M_PI);
}
```

**Phong分布公式**：

$$
p(\mathbf{h}) = \frac{\alpha + 1}{2\pi} (\mathbf{n} \cdot \mathbf{h})^\alpha
$$

**参数说明**：
- $\alpha$：高光指数（shininess）
  - $\alpha = 1$：宽大、柔和的高光
  - $\alpha = 100$：尖锐、明亮的高光
- $(\mathbf{n} \cdot \mathbf{h})^\alpha$：高光强度随角度衰减
- $\frac{\alpha + 1}{2\pi}$：归一化因子

**能量守恒验证**：

$$
\int_{\Omega} \frac{\alpha + 1}{2\pi} (\mathbf{n} \cdot \mathbf{h})^\alpha \, d\omega_h = 1
$$

### 2. BRDF 评估

```cpp
vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    const vec3 h = normalize(wi - wo);
    const float nh = dot(h, n);
    const float spec = pow(nh, alpha);
    const float normFactor = (alpha + 2.0f) / (2.0f * M_PI);
    return radiance * spec * normFactor;
}
```

**Phong BRDF公式**：

$$
f_r(\omega_i, \omega_o) = k_s \frac{\alpha + 2}{2\pi} (\mathbf{n} \cdot \mathbf{h})^\alpha
$$

**参数说明**：
- $k_s$：镜面反射系数（材质颜色）
- $(\mathbf{n} \cdot \mathbf{h})^\alpha$：高光强度
- $\frac{\alpha + 2}{2\pi}$：归一化因子

**为什么归一化因子是 $\frac{\alpha + 2}{2\pi}$？**

PDF归一化：$\frac{\alpha + 1}{2\pi}$（半程向量空间）
BRDF归一化：$\frac{\alpha + 2}{2\pi}$（入射方向空间）

两个空间的雅可比行列式不同，所以归一化因子也不同。

### 3. 方向采样

```cpp
vec3 sampleDir(const vec3& wo, const vec3& n) const {
    const float u = genRandomFloat();
    const float v = genRandomFloat();
    
    // 计算球坐标
    const float phi = 2.0f * M_PI * u;
    const float cosTheta = pow(v, 1.0f / (alpha + 1.0f));
    const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    
    // 转换为笛卡尔坐标
    auto h = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    h = toWorld(h, n);
    
    // 根据反射定律计算出射方向
    const vec3 wi = wo - 2.0f * dot(wo, h) * h;
    return normalize(wi);
}
```

**采样步骤**：

1. **在半程向量空间中采样**：
   - $\phi = 2\pi u$（方位角，均匀分布）
   - $\cos\theta = v^{1/(\alpha+1)}$（极角，Phong分布）

2. **转换到世界坐标系**：
   ```cpp
   h = toWorld(h, n);
   ```

3. **根据反射定律计算出射方向**：
   ```cpp
   wi = wo - 2(wo·h)h
   ```

**为什么 $\cos\theta = v^{1/(\alpha+1)}$？**

这是**逆变换采样**的结果。

Phong分布的累积分布函数（CDF）：

$$
\text{CDF}(\theta) = 1 - \cos^{\alpha+1}(\theta)
$$

逆变换：

$$
v = \text{CDF}(\theta) = 1 - \cos^{\alpha+1}(\theta)
$$

$$
\cos^{\alpha+1}(\theta) = 1 - v
$$

$$
\cos(\theta) = (1 - v)^{1/(\alpha+1)}
$$

由于 $v$ 和 $1-v$ 都是均匀分布，可以简化为：

$$
\cos(\theta) = v^{1/(\alpha+1)}
$$

### Phong 高光参数的影响

| alpha 值 | 高光特性 | 应用场景 |
|----------|---------|---------|
| 1-10 | 宽大、柔和的高光 | 塑料、橡胶 |
| 10-50 | 中等高光 | 油漆、皮革 |
| 50-200 | 尖锐、明亮的高光 | 金属、玻璃 |

---

## MirrorSpecularBRDF（镜面反射）

### 物理原理

镜面反射是**理想化的镜面反射**，光线只在一个方向上反射，即完美反射方向。

**核心特点**：
- 只有一个反射方向（完美反射定律）
- 反射角等于入射角
- 包含Fresnel效应（角度相关的反射率）

### 1. PDF 计算

```cpp
float pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    if (dot(wi, n) < 0.f) return 0.f;  // 背面，无效
    return 1.0f;  // Delta分布
}
```

**Delta分布**：

镜面反射的PDF是**Delta分布**（狄拉克δ函数），因为只有一个反射方向。

$$
p(\omega) = \delta(\omega - \omega_r)
$$

其中 $\omega_r$ 是完美反射方向。

**为什么是1.0？**

- Delta分布在完美反射方向上可以是任何值
- 选择1.0是为了简化计算
- 避免复杂的Delta函数处理

**蒙特卡洛积分中的处理**：

$$
\int_{\Omega} f(\omega) \delta(\omega - \omega_r) \, d\omega = f(\omega_r)
$$

在代码中：

```cpp
// 采样方向就是完美反射方向
vec3 wi = sampleDir(wo, n);

// PDF = 1.0
float pdf = 1.0;

// 蒙特卡洛估计
vec3 contribution = eval * Li * cosTheta / pdf;
                  = eval * Li * cosTheta / 1.0
                  = eval * Li * cosTheta
```

### 2. BRDF 评估

```cpp
vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    const float cosTheta = dot(wi, n);
    const vec3 fresnelSchlick = radiance + (vec3(1.0f) - radiance) * pow(1.0f - cosTheta, 5.0f);
    return fresnelSchlick / cosTheta;
}
```

**镜面反射BRDF公式**：

$$
f_r(\omega_i, \omega_o) = \frac{F(\omega_o)}{\cos(\theta_o)}
$$

其中：
- $F(\omega_o)$ 是Fresnel反射率
- $\cos(\theta_o) = \mathbf{n} \cdot \omega_o$ 是出射角的余弦

### Fresnel-Schlick 近似

```cpp
vec3 fresnelSchlick = radiance + (vec3(1.0f) - radiance) * pow(1.0f - cosTheta, 5.0f);
```

**数学表达式**：

$$
F(\theta) = F_0 + (1 - F_0) (1 - \cos\theta)^5
$$

**参数说明**：
- $F_0 = \text{radiance}$：垂直入射时的反射率（材质颜色）
- $\cos\theta = \mathbf{n} \cdot \omega_o$：出射角的余弦
- $5$：Schlick近似的指数

**为什么是5次方？**
- 这是Schlick提出的经验公式
- 近似Fresnel方程的精确解
- 计算效率高，效果好

### Fresnel 效应

Fresnel效应描述了**反射率随角度变化**的现象：

| 入射角 | 反射率 | 现象 |
|-------|-------|------|
| 0°（垂直） | $F_0$ | 材质固有反射率 |
| 30° | $F_0 + 0.5(1-F_0)$ | 反射率增加 |
| 60° | $F_0 + 0.94(1-F_0)$ | 反射率显著增加 |
| 90°（掠射） | 1.0 | 几乎完全反射 |

**实际例子**：
- **水面**：垂直看下去透明，掠射看像镜子
- **玻璃**：垂直看透明，掠射看反射
- **金属**：所有角度都有较强反射

### 3. 方向采样

```cpp
vec3 sampleDir(const vec3& wo, const vec3& n) const {
    return normalize(wo - 2.0f * dot(wo, n) * n);
}
```

**完美反射定律的向量形式**：

$$
\omega_i = \omega_o - 2(\omega_o \cdot \mathbf{n})\mathbf{n}
