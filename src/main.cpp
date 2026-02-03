#include "benchmark/benchmark.h"
#include <cstring>
#include <immintrin.h>
#include <processthreadsapi.h>
#include <windows.h>
#include <d3dx9math.h>
#include <xmmintrin.h>
#include "Inc/DirectXMath.h"
#include "read.hpp"
typedef D3DXMATRIX* (WINAPI *PFN_D3DXMatrixMultiply)(D3DXMATRIX*, const D3DXMATRIX*, const D3DXMATRIX*);
typedef int (WINAPI *B_strcmp)(const char*, const char*);
static HMODULE hD3DX = LoadLibraryA("d3dx9_43.dll");
static HMODULE libasm = LoadLibraryA("libad64.dll");
static PFN_D3DXMatrixMultiply pD3DXMatrixMultiply = (PFN_D3DXMatrixMultiply)GetProcAddress(hD3DX, "D3DXMatrixMultiply");
static B_strcmp Astrcmp = (B_strcmp)GetProcAddress(libasm, "A_stricmp");

D3DXMATRIX A;
D3DXMATRIX B;
D3DXMATRIX C;

D3DXVECTOR4 vec4a;
D3DXVECTOR4 vec4b; // out
D3DXMATRIX vec4c;


constexpr size_t bytes_per_iter = 16 * sizeof(float) * 3;

void setup(const benchmark::State& state) {
    float* Af = &A._11;
    float* Bf = &B._11;
    for (int i = 0; i < 16; ++i) {
        Af[i] = 1.0f;
        Bf[i] = 2.0f;
    }
}

void setup_transform(const benchmark::State& state) {
    vec4a.x = 20.984912221f;
    vec4a.y = 592.98278920f;
    vec4a.z = 42.823801342f;
    vec4a.w = 1002.8127331f;

    float* Cf = &vec4c._11;
    for (int i = 0; i < 16; ++i) {
        Cf[i] = 2.52141f;
    }

}

__attribute__((noinline))
__m256_u* MatrixMultiplyAVX(__m256_u* out, const __m256_u* B, const __m256_u* A) {
		__m256_u a01 = A[0];
		__m256_u a23 = A[1];

		__m128_u a0 = _mm256_castps256_ps128(a01);
		__m128_u a1 = _mm256_extractf128_ps(a01, 1);
		__m128_u a2 = _mm256_castps256_ps128(a23);
		__m128_u a3 = _mm256_extractf128_ps(a23, 1);

		for (int i = 0; i < 2; ++i) {
			__m256_u b_row = B[i];

			__m128_u b0 = _mm256_castps256_ps128(b_row);
			__m128_u b1 = _mm256_extractf128_ps(b_row, 1);

			__m128_u r0 = _mm_add_ps(
							_mm_add_ps(
								_mm_mul_ps(_mm_shuffle_ps(b0, b0, 0x00), a0),
								_mm_mul_ps(_mm_shuffle_ps(b0, b0, 0x55), a1)),
							_mm_add_ps(
								_mm_mul_ps(_mm_shuffle_ps(b0, b0, 0xAA), a2),
								_mm_mul_ps(_mm_shuffle_ps(b0, b0, 0xFF), a3)));

			__m128_u r1 = _mm_add_ps(
							_mm_add_ps(
								_mm_mul_ps(_mm_shuffle_ps(b1, b1, 0x00), a0),
								_mm_mul_ps(_mm_shuffle_ps(b1, b1, 0x55), a1)),
							_mm_add_ps(
								_mm_mul_ps(_mm_shuffle_ps(b1, b1, 0xAA), a2),
								_mm_mul_ps(_mm_shuffle_ps(b1, b1, 0xFF), a3)));
			out[i] = _mm256_insertf128_ps(_mm256_castps128_ps256(r0), r1, 1);
		}

		return out;
	}
	__attribute__((noinline))
	__m128_u* __fastcall intelsse3_D3DXMatrixMultiply(__m128_u* pOut, const float* pM1, const float* pM2) {

		__m128_u row0 = _mm_loadu_ps(pM2 + 0);
		__m128_u row1 = _mm_loadu_ps(pM2 + 4);
		__m128_u row2 = _mm_loadu_ps(pM2 + 8);
		__m128_u row3 = _mm_loadu_ps(pM2 + 12);

		__m128_u sum0 = _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 0), _mm_loadu_ps(pM1 + 0), 0), row0);
		sum0 = _mm_add_ps(sum0, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 1), _mm_loadu_ps(pM1 + 1), 0), row1));
		sum0 = _mm_add_ps(sum0, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 2), _mm_loadu_ps(pM1 + 2), 0), row2));
		sum0 = _mm_add_ps(sum0, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 3), _mm_loadu_ps(pM1 + 3), 0), row3));
		pOut[0] = sum0;

		__m128_u sum1 = _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 4), _mm_loadu_ps(pM1 + 4), 0), row0);
		sum1 = _mm_add_ps(sum1, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 5), _mm_loadu_ps(pM1 + 5), 0), row1));
		sum1 = _mm_add_ps(sum1, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 6), _mm_loadu_ps(pM1 + 6), 0), row2));
		sum1 = _mm_add_ps(sum1, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 7), _mm_loadu_ps(pM1 + 7), 0), row3));
		pOut[1] = sum1;

		__m128_u sum2 = _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 8), _mm_loadu_ps(pM1 + 8), 0), row0);
		sum2 = _mm_add_ps(sum2, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 9), _mm_loadu_ps(pM1 + 9), 0), row1));
		sum2 = _mm_add_ps(sum2, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 10), _mm_loadu_ps(pM1 + 10), 0), row2));
		sum2 = _mm_add_ps(sum2, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 11), _mm_loadu_ps(pM1 + 11), 0), row3));
		pOut[2] = sum2;

		__m128_u sum3 = _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 12), _mm_loadu_ps(pM1 + 12), 0), row0);
		sum3 = _mm_add_ps(sum3, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 13), _mm_loadu_ps(pM1 + 13), 0), row1));
		sum3 = _mm_add_ps(sum3, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 14), _mm_loadu_ps(pM1 + 14), 0), row2));
		sum3 = _mm_add_ps(sum3, _mm_mul_ps(_mm_shuffle_ps(_mm_loadu_ps(pM1 + 15), _mm_loadu_ps(pM1 + 15), 0), row3));
		pOut[3] = sum3;

		return pOut;
	}

__attribute__((noinline))
D3DXMATRIX* MatrixMultiplyAVX2(const D3DXMATRIX* A, const D3DXMATRIX* B, D3DXMATRIX* C)
{
    asm volatile(
        ".intel_syntax noprefix\n\t"
        "mov rax, rcx\n\t"
        "vmovups ymm0, [rdx]           \n\t"
        "vshufps ymm1, ymm0, ymm0, 0x55\n\t"
        "vbroadcastf128 ymm2, [r8+0x10]\n\t"
        "vmulps ymm1, ymm1, ymm2       \n\t"
        "vshufps ymm3, ymm0, ymm0, 0x0 \n\t"
        "vbroadcastf128 ymm4, [r8]     \n\t"
        "vfmadd213ps ymm3, ymm4, ymm1  \n\t"
        "vshufps ymm1, ymm0, ymm0, 0xff\n\t"
        "vbroadcastf128 ymm5, [r8+0x30]\n\t"
        "vfmadd213ps ymm1, ymm5, ymm3  \n\t"
        "vshufps ymm0, ymm0, ymm0, 0xaa\n\t"
        "vbroadcastf128 ymm3, [r8+0x20]\n\t"
        "vfmadd213ps ymm0, ymm3, ymm1  \n\t"
        "vmovups [rcx], ymm0           \n\t"

        "vmovups ymm0, [rdx+0x20]      \n\t"
        "vshufps ymm1, ymm0, ymm0, 0x55\n\t"
        "vmulps ymm1, ymm1, ymm2       \n\t"
        "vshufps ymm2, ymm0, ymm0, 0x0 \n\t"
        "vfmadd213ps ymm2, ymm4, ymm1  \n\t"
        "vshufps ymm1, ymm0, ymm0, 0xff\n\t"
        "vfmadd213ps ymm1, ymm5, ymm2  \n\t"
        "vshufps ymm0, ymm0, ymm0, 0xaa\n\t"
        "vfmadd213ps ymm0, ymm3, ymm1  \n\t"
        "vmovups [rcx+0x20], ymm0      \n\t"

    // for testing checking if you need vzeroupper after call exit
        // "vzeroupper\n\t"
    // checking if penalty perf works inside the function
        // "mulps xmm1, xmm2       \n\t"

        ".att_syntax prefix\n\t"
        :
        : "c"(C), "d"(A), "r"(B)
        : "ymm0","ymm1","ymm2","ymm3","ymm4","ymm5","memory"
    );

    return C;
}
// _Inout_       D3DXVECTOR4 *pOut,
// _In_    const D3DXVECTOR4 *pV,
// _In_    const D3DXMATRIX  *pM
float* BM_D3DXVec4Transform(float* out, const float* vec, const float* mat) {
    __m128 v = _mm_loadu_ps(vec);

    __m128 m0 = _mm_loadu_ps(mat);
    __m128 m1 = _mm_loadu_ps(mat + 4);
    __m128 m2 = _mm_loadu_ps(mat + 8);
    __m128 m3 = _mm_loadu_ps(mat + 12);

    __m128 x = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
    __m128 y = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
    __m128 z = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
    __m128 w = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));

    __m128 r = _mm_mul_ps(x, m0);
    r = _mm_add_ps(r, _mm_mul_ps(w, m3));
    r = _mm_add_ps(r, _mm_mul_ps(y, m1));
    r = _mm_add_ps(r, _mm_mul_ps(z, m2));

    _mm_store_ps(out, r);
    return out;
}
void bm_avx2(benchmark::State &s) {
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            MatrixMultiplyAVX(
                reinterpret_cast<__m256_u*>(&C._11),
                reinterpret_cast<const __m256_u*>(&B._11),
                reinterpret_cast<const __m256_u*>(&A._11)
            )
        );
        benchmark::ClobberMemory();
    }
    s.SetBytesProcessed(s.iterations() * bytes_per_iter);
}
__attribute__((noinline))
D3DXMATRIX* D3DXMatrixMultiplyTest(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2) {
		const auto& result = XMMatrixMultiply(*std::bit_cast<const DirectX::XMMATRIX*>(pM1), *std::bit_cast<const DirectX::XMMATRIX*>(pM2));
		DirectX::XMStoreFloat4x4(std::bit_cast<DirectX::XMFLOAT4X4*>(pOut), result);
		return pOut;
}
__attribute__((noinline,naked))
void penalty_test() {
    asm volatile(
        ".intel_syntax noprefix\n\t"
        "mulps xmm15, xmm1\n\t"
        "addps xmm14, xmm0\n\t"
        "ret\n\t"
        ".att_syntax prefix\n\t"
        :
        :
        :
    );
}
void bm_d3dx9_dynamic(benchmark::State &s) {
    for (auto _ : s) {
        // benchmark::DoNotOptimize(
            MatrixMultiplyAVX2(
                &C,
                &B,
                &A
            );
        // );
        benchmark::ClobberMemory();
    }
    s.SetBytesProcessed(s.iterations() * bytes_per_iter);
}
__attribute__((noinline))
std::uint64_t strlen_sse4(const char* str) {
    const __m128i zero = _mm_setzero_si128();
    std::uint64_t offset = 0ull;
    while(true) {
        __m128i load = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str + offset));
        std::int32_t val = _mm_cmpistri(zero, load, _SIDD_CMP_EQUAL_EACH);

        if (val != 16) {
            return offset + val;
        }

        offset += 16;
    }
}

__attribute__((noinline))
__attribute__((target("avx2,bmi")))
std::uint64_t strlen_avx2(const char* s) {
    const __m256i zero = _mm256_setzero_si256();
    const char* ptr = s;

    while (true) {
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr));
        __m256i cmp = _mm256_cmpeq_epi8(data, zero);

        uint32_t mask = static_cast<uint32_t>(_mm256_movemask_epi8(cmp));
        if (mask) {
            return static_cast<std::uint64_t>(ptr - s + __builtin_ctz(mask));
        }

        ptr += 32;
    }
}
__attribute__((noinline))
int strcasecmp_sse2(const char* s0, const char* s1) {
    if (__builtin_expect((s0 == nullptr || s1 == nullptr), 0)) {
        return (s0 == s1 ? 0 : (s0 == nullptr ? -1 : 1));
    }
    const __m128i upper_mask = _mm_set1_epi8(static_cast<char>(0xDF));
    // #pragma unroll (4)
    while (true) {
        const __m128i lcl = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s0));
        const __m128i lcr = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s1));

        const __m128i case_lcl = _mm_and_si128(lcl, upper_mask);
        const __m128i case_lcr = _mm_and_si128(lcr, upper_mask);
        const __m128i zero_check = _mm_cmpeq_epi8(lcl, _mm_setzero_si128());
        const __m128i cmp_mask = _mm_cmpeq_epi8(case_lcl, case_lcr);

        const uint32_t mask = static_cast<uint32_t>(_mm_movemask_epi8(_mm_or_si128(zero_check, ~cmp_mask)));

        if (mask) {
            const int idx = __builtin_ctz(mask);
            return static_cast<signed char>(s0[idx]) - static_cast<signed char>(s1[idx]);
        }
        s0 += 16;
        s1 += 16;
    }
}
__attribute__((noinline))
int strcmp_sse2v3(const char *s0, const char *s1) {
    if (__builtin_expect((s0 == nullptr || s1 == nullptr), 0)) {
        return (s0 == s1 ? 0 : (s0 == nullptr ? -1 : 1));
    }
    const __m128i *lp = (const __m128i *)s0;
    const __m128i *rp = (const __m128i *)s1;
    const __m128i all0 = _mm_setzero_si128();
    const __m128i upper_mask = _mm_set1_epi8(static_cast<char>(0xDF));

    __m128i l, r;
    unsigned int m;
    size_t i = 0;

    do {
        l = _mm_loadu_si128(lp + i);
        r = _mm_loadu_si128(rp + i);
        const __m128i case_lcl = _mm_and_si128(l, upper_mask);
        const __m128i case_lcr = _mm_and_si128(r, upper_mask);
        __m128i eq = _mm_cmpeq_epi8(l, r);
        __m128i null_mask = _mm_cmpeq_epi8(l, all0);

        m = (unsigned int)_mm_movemask_epi8(_mm_or_si128(null_mask, _mm_andnot_si128(eq, _mm_set1_epi8(-1))));

        ++i;
    } while (!m);

    int index = __builtin_ctz(m);
    const unsigned char* p0 = reinterpret_cast<const unsigned char*>(s0) + ((i - 1) * 16);
    const unsigned char* p1 = reinterpret_cast<const unsigned char*>(s1) + ((i - 1) * 16);
    return (int)p0[index] - (int)p1[index];
}
inline __m128i upcase_si128(__m128i src) { // Peter Cordes upcase
    // The above 2 paragraphs were comments here
    __m128i rangeshift = _mm_sub_epi8(src, _mm_set1_epi8('a' + 128));
    __m128i nomodify = _mm_cmpgt_epi8(rangeshift, _mm_set1_epi8(-128 + 25));  // 0:lower case   -1:anything else (upper case or non-alphabetic).  25 = 'z' - 'a'

    __m128i flip = _mm_andnot_si128(nomodify, _mm_set1_epi8(0x20));            // 0x20:lcase    0:non-lcase

    // just mask the XOR-mask so elements are XORed with 0 instead of 0x20
    return          _mm_xor_si128(src, flip);
    // it's easier to xor with 0x20 or 0 than to AND with ~0x20 or 0xFF
}
__attribute__((noinline))
int stricmp_sse42(const char* s1, const char* s2) {
    while (true) {
        const int mode = _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY;
        __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s1));
        __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s2));
        // v1 = upcase_si128(v1);
        // v2 = upcase_si128(v2);
        int v3 = _mm_cmpistri(v1, v2, mode);
        if (v3 != 16) {
            unsigned char ca = static_cast<unsigned char>(s1[v3]);
            unsigned char cb = static_cast<unsigned char>(s2[v3]);
            return static_cast<int>(ca) - static_cast<int>(cb);
        }

        if (_mm_cmpistrz(v1, v2, mode)) {
            return 0;
        }

        s1 += 16;
        s2 += 16;
    }
    return 0;
}
// test for penalty
void bm_penalty(benchmark::State& s) {
    for (auto _ : s) {
        MatrixMultiplyAVX2(&C, &B, &A);
    // non call instruction
        // asm volatile(
        //     ".intel_syntax noprefix\n\t"
        //     "mulps xmm15, xmm1\n\t"
        //     "addps xmm14, xmm0\n\t"
        //     ".att_syntax prefix\n\t"
        //     :
        //     :
        //     :
        // );
    // call instruction
        // penalty_test();
        benchmark::ClobberMemory();
    }
}
void bm_vec4(benchmark::State& s) {
    volatile float* out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = BM_D3DXVec4Transform(
                reinterpret_cast<float*>(&vec4b),
                reinterpret_cast<const float*>(&vec4a),
                reinterpret_cast<const float*>(&vec4c)
            )
        );
        benchmark::ClobberMemory();
    }
}

void bm_strlensse4(benchmark::State& s){
    std::uint64_t out = 0ull;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strlen_sse4(var2)
        );
        benchmark::ClobberMemory();
    }
}
void bm_strlenavx2(benchmark::State& s){
    std::uint64_t out = 0ull;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strlen_avx2(var2)
        );
        benchmark::ClobberMemory();
    }
}

size_t strlen_sse2v2(const char *s){
  const __m128i *vp =((__m128i*)s)-4, all0 = (__m128i){0};
  __m128i v0,v1,v2,v3,v;
  do{
    vp+=4;
    v = v0 = _mm_cmpeq_epi8(_mm_loadu_si128(vp+0),all0);
    v|= v1 = _mm_cmpeq_epi8(_mm_loadu_si128(vp+1),all0);
    v|= v2 = _mm_cmpeq_epi8(_mm_loadu_si128(vp+2),all0);
    v|= v3 = _mm_cmpeq_epi8(_mm_loadu_si128(vp+3),all0);
  }while(!(_mm_movemask_epi8(v)));
  uint64_t m = (uint64_t)_mm_movemask_epi8(v0) | ((uint64_t)_mm_movemask_epi8(v1)<<16) |
    ((uint64_t)_mm_movemask_epi8(v2)<<32) | ((uint64_t)_mm_movemask_epi8(v3)<<48);
  return (char*)vp - s + __builtin_ctzll(m);
}

void bm_strlendefault(benchmark::State& s){
    std::uint64_t out = 0ull;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strlen_sse2v2(var2)
        );
        benchmark::ClobberMemory();
    }
}

void bm_strcasecmptest1(benchmark::State& s){
    int out = 0;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strcasecmp_sse2(var2, var9)
        );
        benchmark::ClobberMemory();
    }
}

void bm_strcasecmptest2(benchmark::State& s){
    int out = 0;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = stricmp_sse42(var2, var9)
        );
        benchmark::ClobberMemory();
    }
}
void bm_strcasecmptest3(benchmark::State& s){
    int out = 0;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strcmp_sse2v3(var2, var9)
        );
        benchmark::ClobberMemory();
    }
}

__attribute__((noinline))
char* sse2_strchr_fixed16(const char* str, int c) {
    __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));

    __m128i v_char = _mm_set1_epi8(static_cast<char>(c));

    __m128i match_mask_vec = _mm_cmpeq_epi8(data, v_char);

    __m128i v_zero = _mm_setzero_si128();
    __m128i zero_mask_vec = _mm_cmpeq_epi8(data, v_zero);
    unsigned int match_mask = static_cast<unsigned>(
        _mm_movemask_epi8(match_mask_vec));
    unsigned int zero_mask = static_cast<unsigned>(
        _mm_movemask_epi8(zero_mask_vec));
    int zero_pos = 16;
    if (zero_mask != 0) {
            zero_pos = __builtin_ctz(zero_mask);

    }
    unsigned int final_match_mask = match_mask & ((1U << zero_pos) - 1);

    if (final_match_mask != 0) {
            return const_cast<char*>(str + __builtin_ctz(final_match_mask));
    }

    return nullptr;
}
__attribute__((noinline))
char* strchr_sse42(const char* str, int c) {
    const __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(str));
    const __m128i v_char = _mm_set1_epi8(static_cast<char>(c));

    const int result = _mm_cmpistri(v_char, data, _SIDD_CMP_EQUAL_EACH);
    if (result == 16) {
        return 0;
    }
    return (char*)(str + result);
}

void bm_strchr(benchmark::State& s){
    char* out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = sse2_strchr_fixed16(var5, 92)
        );
        benchmark::ClobberMemory();
    }
}
void bm_strchrsse4(benchmark::State& s){
    char* out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strchr_sse42(var5, 92)
        );
        benchmark::ClobberMemory();
    }
}
__attribute__((noinline))
int strcmp_sse2(const char *s0, const char *s1) {
    const __m128i *lp = (const __m128i *)s0;
    const __m128i *rp = (const __m128i *)s1;
    const __m128i all0 = _mm_setzero_si128();

    __m128i l, r;
    unsigned int m;
    size_t i = 0;

    do {
        l = _mm_loadu_si128(lp + i);
        r = _mm_loadu_si128(rp + i);

        __m128i eq = _mm_cmpeq_epi8(l, r);
        __m128i null_mask = _mm_cmpeq_epi8(l, all0);

        m = (unsigned int)_mm_movemask_epi8(_mm_or_si128(null_mask, _mm_andnot_si128(eq, _mm_set1_epi8(-1))));

        ++i;
    } while (!m);

    int index = __builtin_ctz(m);
    const unsigned char* p0 = reinterpret_cast<const unsigned char*>(s0) + ((i - 1) * 16);
    const unsigned char* p1 = reinterpret_cast<const unsigned char*>(s1) + ((i - 1) * 16);
    return (int)p0[index] - (int)p1[index];
}
__attribute__((noinline))
int strcmp_sse42(const char* s1, const char* s2) {
    while (true) {
        const int mode = _SIDD_CMP_EQUAL_EACH | _SIDD_NEGATIVE_POLARITY;
        __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s1));
        __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(s2));
        int v3 = _mm_cmpistri(v1, v2, mode);
        if (v3 != 16) {
            unsigned char ca = static_cast<unsigned char>(s1[v3]);
            unsigned char cb = static_cast<unsigned char>(s2[v3]);
            return static_cast<int>(ca) - static_cast<int>(cb);
        }

        if (_mm_cmpistrz(v1, v2, mode)) {
            return 0;
        }

        s1 += 16;
        s2 += 16;
    }
    // return 0;
}
void bm_strcmpsse2(benchmark::State& s){
    int out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strcmp_sse2(var6, var7)
        );
        benchmark::ClobberMemory();
    }
}
void bm_strcmpsse4(benchmark::State& s){
    int out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = strcmp_sse42(var6, var7)
        );
        benchmark::ClobberMemory();
    }
}

__attribute__((noinline))
int cst_time_memcmp_fastest1(const void *m1, const void *m2, size_t n) {
    const unsigned char *pm1 = (const unsigned char*)m1;
    const unsigned char *pm2 = (const unsigned char*)m2;
    int res = 0, diff;
    if (n > 0) {
        do {
            --n;
            diff = pm1[n] - pm2[n];
            res = (res & -!diff) | diff;
        } while (n != 0);
    }
    return (res > 0) - (res < 0);
}

__attribute__((noinline))
int memcmp_sse2(const void* a_void, const void* b_void, std::uint64_t size) {
    const char* a = static_cast<const char*>(a_void);
    const char* b = static_cast<const char*>(b_void);
    std::uint64_t offset = 0;
    for (; offset + 16 <= size; offset += 16) {
        __m128i_u va = _mm_loadu_si128(reinterpret_cast<const __m128i_u*>(a + offset));
        __m128i_u vb = _mm_loadu_si128(reinterpret_cast<const __m128i_u*>(b + offset));
        __m128i cmp = _mm_xor_si128(va, vb);
        int eqmask = _mm_movemask_epi8(_mm_cmpeq_epi8(cmp, _mm_setzero_si128()));
        unsigned int diffmask = (~static_cast<unsigned int>(eqmask)) & 0xFFFFu;

        if (diffmask) {
            std::uint64_t diff_byte_in_chunk = static_cast<std::uint64_t>(__builtin_ctz(diffmask));
            return static_cast<int>(static_cast<unsigned char>(a[offset + diff_byte_in_chunk]) -
                                    static_cast<unsigned char>(b[offset + diff_byte_in_chunk]));
        }
    }
    return memcmp(a + offset, b + offset, size - offset);
}
__attribute__((noinline))
int memcmp_scalar(const char* a, const char* b, std::uint64_t size) {
    return std::memcmp(a, b, size);
}

int avx2_memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (n >= 32) {
        __m256i v1 = _mm256_loadu_si256((const __m256i *)p1);
        __m256i v2 = _mm256_loadu_si256((const __m256i *)p2);

        __m256i cmp = _mm256_cmpeq_epi8(v1, v2);

        uint32_t mask = _mm256_movemask_epi8(cmp);

        if (mask != 0xFFFFFFFF) {
            uint32_t first_mismatch = __builtin_ctz(~mask);
            return (int)p1[first_mismatch] - (int)p2[first_mismatch];
        }

        p1 += 32;
        p2 += 32;
        n -= 32;
    }

    if (n >= 16) {
        __m128i v1 = _mm_loadu_si128((const __m128i *)p1);
        __m128i v2 = _mm_loadu_si128((const __m128i *)p2);
        __m128i cmp = _mm_cmpeq_epi8(v1, v2);
        uint32_t mask = _mm_movemask_epi8(cmp);

        if (mask != 0xFFFF) {
            uint32_t first_mismatch = __builtin_ctz(~mask);
            return (int)p1[first_mismatch] - (int)p2[first_mismatch];
        }
        p1 += 16;
        p2 += 16;
        n -= 16;
    }
    // slow vs memcmp on
    // 1,7,11 14 and 15 I think
    if (n >= 8) {
        uint64_t a = *(uint64_t*)p1;
        uint64_t b = *(uint64_t*)p2;

        int diff = (int)a - (int)b;
        if (diff) {
            return (diff > 0) - (diff < 0);
        }

        p1 += 8;
        p2 += 8;
        n -= 8;
    }

    if (n >= 4) {
        uint32_t a = *(uint32_t*)p1;
        uint32_t b = *(uint32_t*)p2;

        int diff = (int)a - (int)b;
        if (diff) {
            return (diff > 0) - (diff < 0);
        }

        p1 += 4;
        p2 += 4;
        n -= 4;
    }
    if (n >= 2) {
        uint16_t a = *(uint16_t*)p1;
        uint16_t b = *(uint16_t*)p2;

        int diff = (int)a - (int)b;
        if (diff) {
            return (diff > 0) - (diff < 0);
        }

        p1 += 2;
        p2 += 2;
        n -= 2;
    }
    if (n) {
        uint8_t a = *(uint8_t*)p1;
        uint8_t b = *(uint8_t*)p2;

        int diff = (int)a - (int)b;
        return (diff > 0) - (diff < 0);
    }
    // switch(n) {
    //     case 1: {
    //         int var2 = *(uint8_t*)s1 - *(uint8_t*)s2;
    //         if (var2 < 0) {
    //             return -1;
    //         } else if (var2 > 0) {
    //             return 1;
    //         } else {
    //             return 0;
    //         }
    //         break;
    //     }
    //     case 2: {
    //         int var2 = *(uint16_t*)s1 - *(uint16_t*)s2;
    //         if (var2 < 0) {
    //             return -1;
    //         } else if (var2 > 0) {
    //             return 1;
    //         } else {
    //             return 0;
    //         }
    //         break;
    //     }
    //     case 3: {
    //         int var2 = *(uint16_t*)s1 - *(uint16_t*)s2;
    //         int var3 = *(uint8_t*)(p1+2) - *(uint8_t*)(p2+2);
    //         int var4 = var2 + var3;
    //         if (var4 < 0) {
    //             return -1;
    //         } else if (var4 > 0) {
    //             return 1;
    //         } else {
    //             return 0;
    //         }
    //         break;
    //     }
    //     case 4: {
    //         int var2 = *(uint32_t*)s1 - *(uint32_t*)s2;
    //         if (var2 < 0) {
    //             return -1;
    //         } else if (var2 > 0) {
    //             return 1;
    //         } else {
    //             return 0;
    //         }
    //         break;
    //     }

    //     case 8: {
    //         int64_t var2 = *(uint64_t*)s1 - *(uint64_t*)s2;
    //         if (var2 < 0) {
    //             return -1;
    //         } else if (var2 > 0) {
    //             return 1;
    //         } else {
    //             return 0;
    //         }
    //         break;
    //     }

    // }
    // while (n--) {
    //     if (*p1 != *p2) {
    //         return (int)*p1 - (int)*p2;
    //     }
    //     p1++;
    //     p2++;
    // }

    return 0;
}
    // try movbe/bswap on byte 3 to 7
void bm_memcmp(benchmark::State& s){
    int out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = memcmp(var6, var7, 0)
        );
        benchmark::ClobberMemory();
    }
}
void bm_memcmpsse2(benchmark::State& s){
    int out;
    for (auto _ : s) {
        benchmark::DoNotOptimize(
            out = avx2_memcmp(var6, var7, 0)
        );
        benchmark::ClobberMemory();
    }
}


/* memcmp */
BENCHMARK(bm_memcmp);
BENCHMARK(bm_memcmpsse2);

/* strcmp */
// BENCHMARK(bm_strcmpsse2);
// BENCHMARK(bm_strcmpsse4);

/* strchr */
// BENCHMARK(bm_strchr);
// BENCHMARK(bm_strchrsse4);

/* strcasecmp */
// BENCHMARK(bm_strcasecmptest1);
// BENCHMARK(bm_strcasecmptest2);
// BENCHMARK(bm_strcasecmptest3);

/* strlen benchmark */
// BENCHMARK(bm_strlensse4);
// BENCHMARK(bm_strlenavx2);
// BENCHMARK(bm_strlendefault);

// BENCHMARK(bm_avx2)->Setup(setup);
// BENCHMARK(bm_vec4)->Setup(setup_transform)->Iterations(4736842105);
// BENCHMARK(bm_d3dx9_dynamic)->Setup(setup);
// BENCHMARK(bm_penalty)->Setup(setup);

int main(int argc, char** argv) {
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    SetThreadAffinityMask(GetCurrentThread(), 1);
    benchmark::MaybeReenterWithoutASLR(argc, argv);
    char arg0_default[] = "benchmark";
    char* args_default = reinterpret_cast<char*>(arg0_default);
    if (!argv) {
    argc = 1;
    argv = &args_default;
    }
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
