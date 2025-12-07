import unittest
import numpy as np
import numc as nc
import time
import sys
import random
import resource
# =============================================================================
#  CONFIG & HELPERS
# =============================================================================

# 浮点数比较精度
DECIMAL_PLACES = 5


def create_pair(rows, cols, rand=True):
    """生成一对数据相同的 NumPy 数组和 NumC 矩阵"""
    if rand:
        # 生成 -100 到 100 之间的随机浮点数
        np_mat = np.random.uniform(-10.0, 10.0, (rows, cols))
    else:
        np_mat = np.zeros((rows, cols))

    # 尝试使用 2D list 初始化
    try:
        nc_mat = nc.Matrix(np_mat.tolist())
    except Exception as e:
        # 如果 2D list 初始化失败，尝试逐个赋值 (fallback)
        nc_mat = nc.Matrix(rows, cols)
        for i in range(rows):
            for j in range(cols):
                nc_mat.set(i, j, np_mat[i, j])

    return np_mat, nc_mat


def check_eq(test_case, np_obj, nc_obj, name="Result"):
    """比较 NumPy 对象和 NumC 对象是否相等"""

    # 1. 如果是数字 (Float/Int)
    if isinstance(nc_obj, (float, int)):
        if isinstance(np_obj, np.ndarray):
            # Numpy 可能会把标量包在数组里
            np_val = np_obj.item()
        else:
            np_val = np_obj
        test_case.assertAlmostEqual(np_val, nc_obj, places=DECIMAL_PLACES,
                                    msg=f"{name} value mismatch (Scalar)")
        return

    # 2. 如果是矩阵 (Matrix)
    # 检查 Shape
    # NumC 1D 矩阵 shape 应该是 (N,)，但如果是 (1, N) 或 (N, 1) 我们也暂且兼容比较
    np_shape = np_obj.shape
    nc_shape = nc_obj.shape

    # 标准化 Shape 进行比较
    np_flat_len = np.prod(np_shape)
    nc_flat_len = np.prod(nc_shape)

    test_case.assertEqual(np_flat_len, nc_flat_len,
                          f"{name} element count mismatch: {np_shape} vs {nc_shape}")

    # 检查所有元素的值
    # 无论维度如何，我们都 flatten 之后比较，这样最稳健
    np_flat = np_obj.flatten()

    # 手动 Flatten NumC 矩阵
    nc_flat = []
    rows = nc_shape[0] if len(nc_shape) > 0 else 0
    cols = nc_shape[1] if len(nc_shape) > 1 else 0

    # 处理 1D 矩阵特例 (shape长度为1)
    if len(nc_shape) == 1:
        cols = nc_shape[0]
        rows = 1
        # 尝试读取
        for i in range(cols):
            try:
                # 尝试作为行向量读取
                nc_flat.append(nc_obj.get(0, i))
            except:
                # 尝试作为列向量读取
                nc_flat.append(nc_obj.get(i, 0))
    else:
        # 2D 矩阵
        for i in range(rows):
            for j in range(cols):
                nc_flat.append(nc_obj.get(i, j))

    for i in range(len(np_flat)):
        test_case.assertAlmostEqual(np_flat[i], nc_flat[i], places=DECIMAL_PLACES,
                                    msg=f"{name} mismatch at flattened index {i}")


# =============================================================================
#  TEST SUITE
# =============================================================================

class TestNumCComprehensive(unittest.TestCase):

    def setUp(self):
        print(f"Running: {self._testMethodName} ... ", end="", flush=True)

    def tearDown(self):
        print("OK")

    # --- 1. 基础功能 & 1D 矩阵 ---
    def test_01_1d_vector_operations(self):
        """测试 1D 矩阵的创建、索引和运算"""
        # 创建 1x5 向量
        np_vec, nc_vec = create_pair(1, 5)

        # 索引测试 (Indexing) -> Float
        check_eq(self, np_vec[0, 2], nc_vec[2], "1D Indexing")

        # 切片测试 (Slicing) -> Matrix
        np_slice = np_vec[:, 1:4]
        nc_slice = nc_vec[1:4]
        check_eq(self, np_slice, nc_slice, "1D Slicing")

        # 1x1 切片 -> 必须是 Matrix
        nc_slice_1x1 = nc_vec[1:2]
        self.assertTrue(hasattr(nc_slice_1x1, 'shape'), "1D slice 1x1 should return Matrix not float")

        # 负数索引 (如果你的实现支持的话，不支持可以注释掉)
        # check_eq(self, np_vec[0, -1], nc_vec[-1], "1D Negative Indexing")

    # --- 2. 复杂的 2D 切片 (Slicing Hell) ---
    def test_02_complex_slicing(self):
        """测试各种花式切片"""
        rows, cols = 6, 6
        np_mat, nc_mat = create_pair(rows, cols)

        # [slice, slice]
        check_eq(self, np_mat[1:4, 2:5], nc_mat[1:4, 2:5], "2D Slice")

        # [int, slice] -> 1D Matrix
        check_eq(self, np_mat[2, :], nc_mat[2, :], "Row Slice")

        # [slice, int] -> 1D Matrix
        check_eq(self, np_mat[:, 3], nc_mat[:, 3], "Col Slice")

        # [slice, slice] -> 1x1 Result -> Float (特殊规则)
        check_eq(self, np_mat[0, 0], nc_mat[0:1, 0:1], "1x1 Slice to Float")

        # [start:stop] 省略 start/stop
        check_eq(self, np_mat[:3, 3:], nc_mat[:3, 3:], "Partial Slice")

    # --- 3. Set Subscript (赋值) ---
    def test_03_set_subscript_complex(self):
        """测试复杂的赋值操作"""
        np_mat, nc_mat = create_pair(4, 4)

        # 赋值标量
        val = 99.9
        np_mat[1, 1] = val
        nc_mat[1, 1] = val
        check_eq(self, np_mat, nc_mat, "Set Scalar")

        # 赋值 1x1 切片
        nc_mat[2:3, 2:3] = val
        np_mat[2, 2] = val
        check_eq(self, np_mat, nc_mat, "Set 1x1 Slice")

        # 赋值 2D 区域
        sub_np, _ = create_pair(2, 2)
        np_mat[0:2, 0:2] = sub_np
        nc_mat[0:2, 0:2] = sub_np.tolist()
        check_eq(self, np_mat, nc_mat, "Set 2D Region")

        # 赋值 1D 区域 (一行)
        row_np, _ = create_pair(1, 4)
        np_mat[3, :] = row_np
        nc_mat[3, :] = row_np.tolist()[0]  # 传入 1D list
        check_eq(self, np_mat, nc_mat, "Set Row")

    # --- 4. 矩阵运算 (Math) ---
    def test_04_math_properties(self):
        """测试数学运算性质"""
        size = 5
        np_a, nc_a = create_pair(size, size)
        np_b, nc_b = create_pair(size, size)

        # (A + B) * A
        np_res = (np_a + np_b) @ np_a
        nc_res = (nc_a + nc_b) * nc_a
        check_eq(self, np_res, nc_res, "(A+B)*A")

        # Neg & Abs
        check_eq(self, -np_a, -nc_a, "Neg")
        check_eq(self, np.abs(np_a), abs(nc_a), "Abs")

        # Power 0 (Identity)
        np_id = np.eye(size)
        nc_id = nc_a ** 0
        check_eq(self, np_id, nc_id, "Power 0")

        # Power 3
        np_p3 = np.linalg.matrix_power(np_a, 3)
        nc_p3 = nc_a ** 3
        check_eq(self, np_p3, nc_p3, "Power 3")

    # --- 5. 错误处理 (Negative Testing) ---
    def test_05_error_handling(self):
        """测试是否正确抛出异常，而不是 Crash"""
        m = nc.Matrix(3, 3)

        # 1. 索引越界
        with self.assertRaises(IndexError):
            _ = m[10, 10]

        # 2. 维度不匹配的加法
        m2 = nc.Matrix(2, 2)
        with self.assertRaises(ValueError):  # 或 Runtime/TypeError
            _ = m + m2

        # 3. 错误的切片赋值 (长度不对)
        with self.assertRaises(ValueError):
            m[0:2, 0:2] = [1, 2]  # 应该是 4 个元素

        # 4. 错误的切片步长 (Step != 1)
        with self.assertRaises(ValueError):
            _ = m[0:3:2, :]

    # --- 6. 边缘情况 (Edge Cases) ---
    def test_06_edge_cases(self):
        """测试空矩阵、非方阵乘法等"""
        # 非方阵乘法 (2x3) * (3x2) -> (2x2)
        np_a, nc_a = create_pair(2, 3)
        np_b, nc_b = create_pair(3, 2)

        np_res = np_a @ np_b
        nc_res = nc_a * nc_b
        check_eq(self, np_res, nc_res, "Non-square Mul")

        # 1x1 矩阵运算
        np_1, nc_1 = create_pair(1, 1)
        np_2, nc_2 = create_pair(1, 1)
        check_eq(self, np_1 + np_2, nc_1 + nc_2, "1x1 Math")

    # --- 7. 终极压力测试 (Stress Test: High Load) ---
    def test_99_high_intensity_stress(self):
        """
        高强度、大矩阵压力测试。
        目的：通过大量分配和计算大矩阵，强制暴露内存泄漏和 Segfault。
        """
        ITERATIONS = 500  # 循环次数
        MIN_DIM = 500  # 最小维度
        MAX_DIM = 700  # 最大维度 (300x300 double ≈ 720KB)

        print(f"\n    [Stress Test] Running {ITERATIONS} iterations with matrices up to {MAX_DIM}x{MAX_DIM}...")
        start_time = time.time()

        for i in range(ITERATIONS):
            # 1. 随机生成维度
            rows = random.randint(MIN_DIM, MAX_DIM)
            cols = random.randint(MIN_DIM, MAX_DIM)

            # 2. 分配大矩阵 (使用 fill 构造函数，纯 C 分配，速度快且压力在 malloc)
            # a: Ref = 1
            a = nc.Matrix(rows, cols, 1.0)
            # b: Ref = 1
            b = nc.Matrix(rows, cols, 2.0)

            # 3. 基础运算 (产生临时大对象)
            # c = a + b. c 是新分配的 (Ref=1)
            c = a + b

            # 4. 链式运算 (测试引用计数传递)
            # d = -c. d (Ref=1). c 在这里只是被读取，计数不变
            d = -c

            # 5. 矩阵乘法 (最消耗 CPU 和 内存 的操作)
            # 为了防止测试跑太久，我们只对“较小”的大矩阵做乘法
            if rows <= 700 and cols <= 700:
                # 构造转置矩阵以便相乘: (r, c) * (c, r) -> (r, r)
                # t: Ref=1
                t = nc.Matrix(cols, rows, 0.5)
                # mul_res: Ref=1
                mul_res = a * t

            # 6. 复杂的切片视图 (测试 allocate_matrix_ref)
            # 切掉最后一行一列
            if rows > 2 and cols > 2:
                # view: Ref=1. 它共享 d 的数据。d 的 ref_cnt 应该增加。
                view = d[1:rows - 1, 1:cols - 1]

                # 7. Set Slice (写操作)
                # 修改视图，应该同时也修改 d
                view[0, 0] = 999.0

                # 验证数据共享 (C 层面是否正确解引用)
                # 注意：view 的 (0,0) 对应 d 的 (1,1)
                val_d = d.get(1, 1)
                if abs(val_d - 999.0) > 0.001:
                    self.fail("Stress test: Data sharing between View and Parent failed")

            # 8. 打印心跳 (每 100 次打印一次，证明还活着)
            if (i + 1) % 100 == 0:

            # --- 循环结束时的内存回收检查 ---
            # 在这里，局部变量 a, b, c, d, t, mul_res, view 将离开作用域。
            # Python 的 GC 会调用它们的 tp_dealloc (即 Matrix61c_dealloc)。
            # Matrix61c_dealloc 会调用 deallocate_matrix。
            #
            # 如果 deallocate_matrix 有逻辑错误 (如没释放 data)，内存占用会飙升。
            # 如果有 Double Free (如释放了 parent 两次)，这里会直接 Segfault。
                # 获取当前进程占用的内存 (Linux下单位是 KB)
                usage_kb = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
                usage_mb = usage_kb / 1024.0
                print(f"        Iter {i + 1}/{ITERATIONS} | Memory: {usage_mb:.2f} MB", flush=True)

        duration = time.time() - start_time
        print(f"Done in {duration:.2f}s. No Segfaults detected.")


if __name__ == '__main__':
    # 启用更详细的输出
    unittest.main(verbosity=0)