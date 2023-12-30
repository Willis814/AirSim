"""import numpy as np
import scipy.integrate as integrate

# 目标函数
def f(x):
    return np.sqrt(x)

# Legendre 多项式
def phi_0(x):
    return 1

def phi_1(x):
    return x

def phi_2(x):
    return (3 * x**2 - 1) / 2

# 计算 Gram 矩阵的元素
def gram_matrix_element(i, j):
    phi_i = [phi_0, phi_1, phi_2][i]
    phi_j = [phi_0, phi_1, phi_2][j]
    return integrate.quad(lambda x: phi_i(x) * phi_j(x), 0, 1)[0]

# 构建 Gram 矩阵
G = np.array([[gram_matrix_element(i, j) for j in range(3)] for i in range(3)])

# 计算右端项的元素
def right_side_element(i):
    phi_i = [phi_0, phi_1, phi_2][i]
    return integrate.quad(lambda x: f(x) * phi_i(x), 0, 1)[0]

# 构造右端项向量
b = np.array([right_side_element(i) for i in range(3)])

# 求解线性方程组
c = np.linalg.solve(G, b)
print(b)
# 输出结果
print("Coefficients:", c)
"""

import numpy as np

# 定义矩阵 A 和向量 b
A = np.array([[1, 0, 0**2], [1, 1, 1**2], [1, 2, 2**2], [1, 3, 3**2], [1, 4, 4**2]])
b = np.array([1, 3, 7, 13, 21])

# 求解线性最小二乘问题
c, residuals, rank, s = np.linalg.lstsq(A, b, rcond=None)

# 输出逼近多项式的系数
print("Coefficients:", c)
