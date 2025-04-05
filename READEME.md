# 基于 Monte Carlo 的路径追踪算法

## 程序说明
- 构建方式：cmake
- 场景文件位置：建议放在主目录下（可以再main函数中修改位置）
- 输出图片位置：output/
- 使用的库：stb_image、tiny_obj_loader、tinyxml2、glm
- 主要技术：
  - 光线与三角面片求交使用bvh加速
  - 路径追踪算法（Monte Carlo 近似积分）
  - 多重重要性采样（直接光源采样和BRDF采样）

## 结果展示
最大弹射次数：5
### Cornell-Box
![alt text](output/cornell-box_100.png)
### Veach-vis
![alt text](output/veach-mis_1000.png)
### Bathroom2
由于模型的差异（视口后面的面片未被删除）有些比起原图会看起来更亮，由于模型法向量并不是向外的，因此代码中求交时会将交点的法线向外。
![alt text](output/bathroom2_1000.png)

## 光传输渲染方程
### 渲染方程原理
渲染方程是计算机图形学的核心，基于辐射度量学和光度学模拟光在场景中的传播与反射。其遵循能量守恒，即进入表面的光能等于反射、透射和吸收光能之和。根据材质，光线可进行兰伯特漫反射或镜面反射。透射和折射则遵循斯涅尔定律。辐射度量学提供的数学工具，如辐射强度和辐射率，为渲染方程的建立奠定了理论基础。

### 方程表达
在光传输渲染方程中，我们使用 $\omega_o$ 和 $\omega_i$ 来表示方向，其中 $\omega_o$ 表示出射方向（观察方向），而 $\omega_i$ 表示入射方向。其描述了在点 $p$ 处，沿出射方向 $\omega_o$ 的总辐射亮度是由自发光和反射光组成的。反射光部分是通过积分计算所有入射方向 $\omega_i$ 的贡献，每个方向的贡献由入射辐射亮度、反射率函数和入射角的余弦值决定。以下是使用这些符号的渲染方程：

$$
L_o(p, \omega_o) = L_e(p, \omega_o) + \int_{\Omega} f_r(p, \omega_i, \omega_o) L_i(p, \omega_i) (\omega_i \cdot n) \, d\omega_i\tag{1}
$$

其中：
- $L_o(p, \omega_o)$: 在点 $p$ 处沿出射方向 $\omega_o$ 的辐射亮度。
- $L_e(p, \omega_o)$: 在点 $p$ 处沿出射方向 $\omega_o$ 的自发光亮度。
- $f_r(p, \omega_i, \omega_o)$: 在点 $p$ 处的反射率函数，描述了入射光 $\omega_i$ 转换为出射光 $\omega_o$ 的比例。
- $L_i(p, \omega_i)$: 在点 $p$ 处沿入射方向 $\omega_i$ 的入射辐射亮度。
- $\omega_i \cdot n$: 入射方向 $\omega_i$ 与表面法向量 $n$ 的点积，描述入射光照对点 $p$ 的贡献。
- $\int_{\Omega}$: 对整个半球 $\Omega$ 上所有可能的入射方向进行积分。

## 基于 Monte Carlo 的路径追踪的路径追踪算法
路径追踪（Path Tracing）是一种基于蒙特卡洛积分（Monte Carlo Integration） 计算全局光照的渲染方法。它可以逼真地模拟光线的 多次弹射（bounces），从而生成逼真的 间接光照、软阴影、漫反射、折射、焦散等 复杂光照效果。路径追踪的核心目标是求解上述的渲染方程

### 路径追踪的 Monte Carelo 近似
蒙特卡洛路径追踪的核心思想是：

1. **从摄像机发射光线**，找到与场景的交点（**光线投射**）。
2. **随机采样一个反射方向**（根据 BRDF 重要性采样）。
3. **沿采样方向继续追踪光线**（递归计算反射光）。
4. **在多个路径上取平均值**，最终收敛于真实的光照计算。

我们使用 **蒙特卡洛估计积分**：

$$
L_o(p, \omega_o) \approx L_e(p, \omega_o) + \frac{1}{N} \sum_{i=1}^{N} \frac{f_r(p, \omega_i, \omega_o) L_i(p, \omega_i) (\omega_i \cdot n)}{p(\omega_i)} \tag{2}
$$

其中：
- $N$ 是样本数（采样次数越多，结果越精确）。
- $p(\omega_i)$ 是采样方向的概率密度函数（PDF）。

### 伪代码表示
```cpp
vec3 path_tracing(Ray ray, int depth) {
    if (depth == MAX_DEPTH) return vec3(0, 0, 0); // 终止递归
    
    Intersection hit = scene_intersect(ray);
    if (!hit) return vec3(0, 0, 0); // 没有交点，返回黑色
    
    vec3 Lo = hit.Le;  // 自发光贡献
    
    // 重要性采样方向
    vec3 wi = sample_dir(hit.normal);
    
    Ray new_ray(hit.position, wi);
    vec3 Li = path_tracing(new_ray, depth + 1); // 递归计算间接光
    
    double BRDF = eval_BRDF(hit, wi, ray.direction);
    double cosine_term = dot(wi, hit.normal);
    
    return Lo + (Li * BRDF * cosine_term) / pdf(wi);
}
```

### 多重重要性采样（Multiple Importance Sampling）
多重重要性采样（MIS）通过组合不同采样策略的优势来降低方差。在实践中，通常需要平衡三种主要的采样策略：BRDF采样、光源采样和环境贴图采样。基于功率启发式（power heuristic）的权重分配方法被证明在多数情况下都能取得良好的效果。最新研究显示，基于机器学习的自适应采样策略可以进一步优化采样分布。


