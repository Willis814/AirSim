import numpy as np
from scipy.linalg import hessenberg


def householder_transformation(A):
    m, n = A.shape
    H = np.copy(A)
    for i in range(n - 2):
        x = H[i + 1:, i]
        e = np.zeros_like(x)
        e[0] = np.linalg.norm(x)
        u = x + np.sign(x[0]) * e
        v = u / np.linalg.norm(u)
        H[i + 1:, i:] -= 2.0 * np.outer(v, np.dot(v, H[i + 1:, i:]))
        H[:, i + 1:] -= 2.0 * np.outer(np.dot(H[:, i + 1:], v), v)
        print(np.round(H, 4), '\n')
    return np.round(H, 4)


# 给定的矩阵A
A = np.array([[1, 2, 3, 4],
              [4, 5, 6, 7],
              [2, 1, 5, 0],
              [4, 2, 1, 0]], dtype='float64')

# 转换为上Hessenberg形式
H = householder_transformation(A)
print(H)

H2 = hessenberg(A)

# 保留四位小数
H_rounded = np.round(H2, 4)

print(H_rounded)
