#!/usr/bin/env python3
"""
generateXML.py - 从OBJ文件生成完整的场景XML
包括相机设置和光源信息
"""

import numpy as np
import sys
import argparse
from pathlib import Path
import re


class Material:
    """材质类"""
    
    def __init__(self, name):
        self.name = name
        self.kd = np.array([0.8, 0.8, 0.8])  # 漫反射颜色
        self.ks = np.array([0.0, 0.0, 0.0])  # 镜面反射颜色
        self.ns = 1.0  # 高光指数
        self.ni = 1.0  # 折射率
        self.tr = np.array([1.0, 1.0, 1.0])  # 透明度
        self.is_light = False  # 是否为光源
        self.radiance = None  # 光源辐射度


class BBox:
    """包围盒类"""
    
    def __init__(self):
        self.min = np.array([float('inf'), float('inf'), float('inf')])
        self.max = np.array([-float('inf'), -float('inf'), -float('inf')])
    
    def expand(self, point):
        """扩展包围盒以包含给定点"""
        self.min = np.minimum(self.min, point)
        self.max = np.maximum(self.max, point)
    
    def center(self):
        """计算包围盒中心"""
        return (self.min + self.max) * 0.5
    
    def size(self):
        """计算包围盒尺寸"""
        return self.max - self.min
    
    def max_dim(self):
        """计算最大维度"""
        return np.max(self.size())


class CameraSettings:
    """相机设置类"""
    
    def __init__(self, eye, lookat, up, fovy, width=1024, height=1024):
        self.eye = eye
        self.lookat = lookat
        self.up = up
        self.fovy = fovy
        self.width = width
        self.height = height
    
    def to_xml(self):
        """转换为XML格式"""
        xml = f"""<camera type="perspective" width="{self.width}" height="{self.height}" fovy="{self.fovy:.4f}">
\t<eye x="{self.eye[0]:.3f}" y="{self.eye[1]:.3f}" z="{self.eye[2]:.3f}"/> 
\t<lookat x="{self.lookat[0]:.3f}" y="{self.lookat[1]:.3f}" z="{self.lookat[2]:.3f}"/> 
\t<up x="{self.up[0]:.3f}" y="{self.up[1]:.3f}" z="{self.up[2]:.3f}"/> 
</camera>"""
        return xml


class SceneParser:
    """场景解析器"""
    
    def __init__(self):
        self.materials = {}  # 材质字典
        self.used_materials = set()  # 使用的材质
        self.lights = []  # 光源列表
        self.bbox = BBox()
    
    def parse_mtl(self, mtl_file):
        """解析MTL文件"""
        print(f"正在解析材质文件: {mtl_file}")
        
        current_material = None
        
        with open(mtl_file, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                
                # 新材质
                if line.startswith('newmtl'):
                    if current_material:
                        self.materials[current_material.name] = current_material
                    
                    mat_name = line.split()[1]
                    current_material = Material(mat_name)
                
                # 漫反射颜色
                elif line.startswith('Kd') and current_material:
                    parts = line.split()
                    if len(parts) >= 4:
                        current_material.kd = np.array([float(parts[1]), float(parts[2]), float(parts[3])])
                
                # 镜面反射颜色
                elif line.startswith('Ks') and current_material:
                    parts = line.split()
                    if len(parts) >= 4:
                        current_material.ks = np.array([float(parts[1]), float(parts[2]), float(parts[3])])
                
                # 高光指数
                elif line.startswith('Ns') and current_material:
                    parts = line.split()
                    if len(parts) >= 2:
                        current_material.ns = float(parts[1])
                
                # 折射率
                elif line.startswith('Ni') and current_material:
                    parts = line.split()
                    if len(parts) >= 2:
                        current_material.ni = float(parts[1])
                
                # 透明度
                elif line.startswith('Tr') and current_material:
                    parts = line.split()
                    if len(parts) >= 4:
                        current_material.tr = np.array([float(parts[1]), float(parts[2]), float(parts[3])])
            
            # 添加最后一个材质
            if current_material:
                self.materials[current_material.name] = current_material
        
        print(f"解析完成，共 {len(self.materials)} 个材质")
    
    def parse_obj(self, obj_file):
        """解析OBJ文件"""
        print(f"正在解析OBJ文件: {obj_file}")
        
        with open(obj_file, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                
                # 顶点
                if line.startswith('v '):
                    parts = line.split()
                    if len(parts) >= 4:
                        x, y, z = float(parts[1]), float(parts[2]), float(parts[3])
                        self.bbox.expand(np.array([x, y, z]))
                
                # 使用材质
                elif line.startswith('usemtl'):
                    mat_name = line.split()[1]
                    self.used_materials.add(mat_name)
        
        print(f"解析完成，使用了 {len(self.used_materials)} 个材质")
    
    def identify_lights(self):
        """识别光源材质"""
        print("正在识别光源材质...")
        
        for mat_name in self.used_materials:
            if mat_name in self.materials:
                mat = self.materials[mat_name]
                
                # 检查是否为光源（根据材质名称或属性）
                if self.is_light_material(mat):
                    mat.is_light = True
                    # 默认辐射度：白色光源
                    if mat.radiance is None:
                        mat.radiance = np.array([34.0, 24.0, 8.0])  # 默认暖色光
                    self.lights.append(mat)
                    print(f"  发现光源: {mat_name}, 辐射度: {mat.radiance}")
        
        print(f"识别完成，共 {len(self.lights)} 个光源")
    
    def is_light_material(self, mat):
        """判断是否为光源材质"""
        # 方法1：根据材质名称判断
        light_keywords = ['light', 'emissive', 'emit', 'lamp', 'bulb']
        mat_name_lower = mat.name.lower()
        for keyword in light_keywords:
            if keyword in mat_name_lower:
                return True
        
        # 方法2：根据材质属性判断（高亮度）
        if np.max(mat.kd) > 0.9 and np.min(mat.kd) > 0.8:
            return True
        
        return False
    
    def calculate_camera(self):
        """计算推荐相机设置"""
        center = self.bbox.center()
        max_dim = self.bbox.max_dim()
        
        # 使用正面视角
        camera = CameraSettings(
            eye=center + np.array([0, 0, max_dim * 1.5]),
            lookat=center,
            up=np.array([0, 1, 0]),
            fovy=45.0
        )
        
        return camera
    
    def generate_xml(self, camera=None, width=1024, height=1024):
        """生成完整的XML文件"""
        xml_lines = []
        
        # XML头部
        xml_lines.append('<?xml version="1.0" encoding="utf-8"?>')
        xml_lines.append('')
        
        # 相机设置
        if camera:
            camera.width = width
            camera.height = height
            xml_lines.append(camera.to_xml())
            xml_lines.append('')
        
        # 光源设置
        for light in self.lights:
            radiance_str = f"{light.radiance[0]:.1f}, {light.radiance[1]:.1f}, {light.radiance[2]:.1f}"
            xml_lines.append(f'<light mtlname="{light.name}" radiance="{radiance_str}"/>')
        
        return '\n'.join(xml_lines)


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='从OBJ文件生成完整的场景XML')
    parser.add_argument('obj_file', help='OBJ文件路径')
    parser.add_argument('--output', '-o', help='输出XML文件路径')
    parser.add_argument('--width', '-W', type=int, default=1024, help='图像宽度')
    parser.add_argument('--height', '-H', type=int, default=1024, help='图像高度')
    parser.add_argument('--radiance', '-r', help='光源辐射度（格式：R,G,B）')
    parser.add_argument('--fovy', '-f', type=float, default=45.0, help='相机视场角')
    parser.add_argument('--camera-eye', help='相机位置（格式：x,y,z）')
    parser.add_argument('--camera-lookat', help='相机观察点（格式：x,y,z）')
    
    args = parser.parse_args()
    
    # 检查文件是否存在
    obj_file = Path(args.obj_file)
    if not obj_file.exists():
        print(f"错误: 文件不存在: {obj_file}")
        sys.exit(1)
    
    print(f"=== 场景XML生成器 ===")
    print(f"OBJ文件: {obj_file}")
    print()
    
    # 创建解析器
    parser_obj = SceneParser()
    
    # 解析MTL文件
    mtl_file = obj_file.with_suffix('.mtl')
    if mtl_file.exists():
        parser_obj.parse_mtl(str(mtl_file))
    else:
        print(f"警告: 未找到MTL文件: {mtl_file}")
    
    # 解析OBJ文件
    parser_obj.parse_obj(str(obj_file))
    
    # 识别光源
    parser_obj.identify_lights()
    
    # 打印场景分析
    print()
    print("=== 场景分析 ===")
    print(f"包围盒:")
    print(f"  最小值: ({parser_obj.bbox.min[0]:.3f}, {parser_obj.bbox.min[1]:.3f}, {parser_obj.bbox.min[2]:.3f})")
    print(f"  最大值: ({parser_obj.bbox.max[0]:.3f}, {parser_obj.bbox.max[1]:.3f}, {parser_obj.bbox.max[2]:.3f})")
    print(f"  中心: ({parser_obj.bbox.center()[0]:.3f}, {parser_obj.bbox.center()[1]:.3f}, {parser_obj.bbox.center()[2]:.3f})")
    print(f"  尺寸: ({parser_obj.bbox.size()[0]:.3f}, {parser_obj.bbox.size()[1]:.3f}, {parser_obj.bbox.size()[2]:.3f})")
    print(f"  最大维度: {parser_obj.bbox.max_dim():.3f}")
    print()
    
    # 计算相机设置
    if args.camera_eye and args.camera_lookat:
        # 使用用户指定的相机设置
        eye = np.array([float(x) for x in args.camera_eye.split(',')])
        lookat = np.array([float(x) for x in args.camera_lookat.split(',')])
        camera = CameraSettings(
            eye=eye,
            lookat=lookat,
            up=np.array([0, 1, 0]),
            fovy=args.fovy
        )
        print("使用用户指定的相机设置")
    else:
        # 使用自动计算的相机设置
        camera = parser_obj.calculate_camera()
        camera.fovy = args.fovy
        print("使用自动计算的相机设置")
    
    print(f"相机位置: ({camera.eye[0]:.3f}, {camera.eye[1]:.3f}, {camera.eye[2]:.3f})")
    print(f"观察点: ({camera.lookat[0]:.3f}, {camera.lookat[1]:.3f}, {camera.lookat[2]:.3f})")
    print(f"视场角: {camera.fovy:.1f}°")
    print()
    
    # 设置光源辐射度
    if args.radiance:
        radiance = np.array([float(x) for x in args.radiance.split(',')])
        for light in parser_obj.lights:
            light.radiance = radiance
        print(f"光源辐射度设置为: {radiance}")
    
    # 生成XML
    xml_content = parser_obj.generate_xml(camera, args.width, args.height)
    
    # 输出到文件
    if args.output:
        output_file = Path(args.output)
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(xml_content)
        print(f"XML已保存到: {output_file}")
    else:
        # 输出到控制台
        print()
        print("=== 生成的XML ===")
        print()
        print(xml_content)


if __name__ == '__main__':
    main()