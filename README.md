# DSP-Quantization-2022
최적의 Scalar uniform/non-uniform 양자화기 및 역양자화기 설계

+ 8-bit raw 이미지 파일의 크기를 낮추면서 화질은 유지할 수 있는 양자화/역양자화 코드 구현
  - Loss : (Bits per pixel) + 4 * MSE (Distortion)
 
+ Decision Level($d_i$) & Reconstruction Level($r_i$) 결정 방식
  - Uniform quantizer - $d_i$ : 이전 상태의 인접한 $r_i$ 값들의 평균, $r_i$ : 이전 상태의 인접한 $d_i$ 값들의 평균
  - Non-uniform quantizer - $d_i$ : 이전 상태의 인접한 $r_i$ 값들의 평균, $r_i$ : Discrete Lloyd-Max algorithm
