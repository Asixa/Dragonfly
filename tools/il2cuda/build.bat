llc -mcpu=sm_20 kernel.ll -o kernel.ptx
clang++ sample.cpp -o cuda.exe --cuda-gpu-arch=compute_70 -I "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA\v10.1/include" -L "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1/lib/x64"   -lcuda
pause