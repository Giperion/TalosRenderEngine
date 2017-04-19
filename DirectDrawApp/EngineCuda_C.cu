#include "cuda_runtime.h"
#include "math_functions.h"
#include "EngineCuda_C.cuh"

extern "C"
{
	//Как тут в вашем С устроить объекты, а?...
	//global for all
	typedef unsigned int DeviceToken;
	typedef int EngineStatus;

	struct EngineData
	{
		DeviceToken ActiveDevice;

		int DeviceCount;
		int ComputeDeviceCount;
		DeviceToken* Devices;

		//minimal specs
		unsigned int ComputeMajor;
		unsigned int ComputeMinor;
	};

	EngineData DataDesc;

	EngineStatus status;
	cudaError_t errcode;
	wchar_t* DebugMessage;

	int BestDeviceID;

	void* argsMemTest;


	int tempDevices[1024];

	/*
	* По текущей реализации ширина кадра формируется значением максимального числа потоков на блок
	* Все потому что пока что задача кидается по типу: Одна линия кадра = Один блок
	* Что позволяет по максимуму создать блоки и потоки за один вызов.
	*/

	__device__ int getGlobalIdx_1D_2D()
	{
			return blockIdx.x * blockDim.x * blockDim.y
				+ threadIdx.y * blockDim.x + threadIdx.x;
	}

	__device__ int getGlobalIdx_2D_2D()
	{
			int blockId = blockIdx.x
				+ blockIdx.y * gridDim.x;
			int threadId = blockId * (blockDim.x * blockDim.y)
				+ (threadIdx.y * blockDim.x)
				+ threadIdx.x;
			return threadId;
		}

	static inline void FastZeroMemory(void* dst, size_t Size)
	{
		int Method = 0;

		//memory aligned?
		if (!(Size % 8))
		{
			Method = 1;
		}

		if (!(Size % 4) && Method == 0)
		{
			Method = 2;
		}

		int IterCount = Size;
		byte* byteMem = (byte*)dst;
		__int64* qwordMem = (__int64*)dst;
		__int32* dwordMem = (__int32*)dst;

		switch (Method)
		{
		case 0:
			for (int i = 0; i < IterCount; i++)
			{
				*byteMem = 0;
				byteMem++;
			}
			break;
		case 1:
			IterCount /= 8;
			for (int i = 0; i < IterCount; i++)
			{
				*qwordMem = 0;
				qwordMem++;
			}
			break;
		case 2:
			IterCount /= 4;
			for (int i = 0; i < IterCount; i++)
			{
				*dwordMem = 0;
				dwordMem++;
			}
			break;
		default:
			break;
		}
	}
	
	const char* GetErrorString(cudaError_t errcode)
	{
		return cudaGetErrorString(errcode);
	}


	bool cuda_init()
	{
		status = CENGINE_STATUS_INIT;
		DebugMessage = L" ";
		argsMemTest = nullptr;
		//Determine if we have CUDA device?
		cudaDeviceProp deviceProp;
		errcode = cudaGetDeviceCount(&DataDesc.DeviceCount);
		if (errcode == cudaErrorInsufficientDriver || errcode == cudaErrorNoDevice)
		{
			if (errcode == cudaErrorInsufficientDriver)
			{
				DebugMessage = L"Cuda driver is outdated! Please update you graphic card driver and try again!";
			}
			else
			{
				DebugMessage = L"There is no cuda device in this machine!";
			}

			status = CENGINE_STATUS_NOCUDA;
			return false;
		}
		//set compute model to max. Later we update that when looking devices
		DataDesc.ComputeMajor = 0x0000FFFF;
		DataDesc.ComputeMinor = 0x000000FF;

		//errcode = cudaMallocHost(&DataDesc.Devices, sizeof(DeviceToken)*DataDesc.DeviceCount);
		DataDesc.Devices = (DeviceToken*)&tempDevices[0];
		for (int DeviceID = 0; DeviceID < DataDesc.DeviceCount; DeviceID++)
		{
			errcode = cudaGetDeviceProperties(&deviceProp, DeviceID);
			if (deviceProp.computeMode != cudaComputeMode::cudaComputeModeProhibited)
			{
				DataDesc.ComputeDeviceCount++;
			}
			else continue;

			int ComputeVer = (deviceProp.major * 10) + deviceProp.minor;
			int KnownComputeVer = (DataDesc.ComputeMajor * 10) + DataDesc.ComputeMinor;
			if (ComputeVer < KnownComputeVer)
			{
				DataDesc.ComputeMajor = deviceProp.major;
				DataDesc.ComputeMinor = deviceProp.minor;
			}

			DataDesc.Devices[DataDesc.ComputeDeviceCount - 1] = DeviceID;
		}
		if (DataDesc.ComputeDeviceCount == 0)
		{
			if (deviceProp.computeMode == cudaComputeMode::cudaComputeModeProhibited)
			{
				status = CENGINE_STATUS_FATALERROR;
				DebugMessage = L"Device computeMode set to prohibiten! Can't compute!";
				return false;
			}
		}
		errcode = cudaSetDevice(DataDesc.Devices[0]);
		//Get device stuff
		status = CENGINE_STATUS_IDLE;
		return true;
	}



	cudaImage cuda_AllocTexture(int width, int height)
	{
		cudaImage result;
		result.width = width;
		result.height = height;
		result.bytes = (width * height) * 3;
		
		status = CENGINE_STATUS_IDLE;
		return result;
	}


	void cuda_deinit()
	{
		if (status != CENGINE_STATUS_IDLE)
		{
			//TODO Sync code
		}
	}

	void cuda_execCode(char* code)
	{

	}
	void WaitCudaThread()
	{
		cudaDeviceSynchronize();
	}

	cudaError_t temp_callKernels(int width, int height, pFrame frame, void* args, int argsSize)
	{
		///#TODO: Different devices can have different recommeded grid
		dim3 blocks(4, 4, 1);
		dim3 grid(width / blocks.x, height / blocks.y, 1);

		//dim3 grid(256, 256, 1);
		//dim3 blocks(width / grid.x, height / grid.y, 1);

		errcode = cudaConfigureCall(grid, blocks);

		errcode = cudaSetupArgument(&width, sizeof(int), 0);
		errcode = cudaSetupArgument(&height, sizeof(int), sizeof(int));
		errcode = cudaSetupArgument(&frame, sizeof(pFrame), sizeof(int) * 2);
		if (args != nullptr)
		{
			if (argsMemTest == nullptr)
			{
				errcode = cudaMalloc(&argsMemTest, argsSize);
			}
			errcode = cudaMemcpy(argsMemTest, args, argsSize, cudaMemcpyKind::cudaMemcpyHostToDevice);
			errcode = cudaSetupArgument(&argsMemTest, sizeof(void*), (sizeof(int) * 2) + sizeof(pFrame));
		}


		errcode = cudaLaunch(testKernelFunc);
		return errcode;
	}

	__device__ inline int Lerp(int start, int end, double value)
	{
		return start + (end - start) * value;
	}

	//dim3 blockIdx <- gridDim
	//dim3 threadIdx <- blocks

	__device__ FColor HSV2RGB(double Hue, double Saturation, double Value)
	{
		double      hh, p, q, t, ff;
		long        i;
		FColor        out;
		out.A = 1.0;

		if (Saturation <= 0.0) {       // < is bogus, just shuts up warnings
			out.R = Value;
			out.G = Value;
			out.B = Value;
			return out;
		}
		hh = Hue;
		if (hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = Value * (1.0 - Saturation);
		q = Value * (1.0 - (Saturation * ff));
		t = Value * (1.0 - (Saturation * (1.0 - ff)));

		switch (i) {
		case 0:
			out.R = Value;
			out.G = t;
			out.B = p;
			break;
		case 1:
			out.R = q;
			out.G = Value;
			out.B = p;
			break;
		case 2:
			out.R = p;
			out.G = Value;
			out.B = t;
			break;

		case 3:
			out.R = p;
			out.G = q;
			out.B = Value;
			break;
		case 4:
			out.R = t;
			out.G = p;
			out.B = Value;
			break;
		case 5:
		default:
			out.R = Value;
			out.G = p;
			out.B = q;
			break;
		}
		return out;
	}

	__device__ Color FColor2Color(FColor inColor)
	{
		Color Out;

		Out.R = Lerp(0, 255, inColor.R);
		Out.G = Lerp(0, 255, inColor.G);
		Out.B = Lerp(0, 255, inColor.B);
		Out.A = Lerp(0, 255, inColor.A);

		return Out;
	}

	__global__ void testKernelFunc(int width, int height, pFrame frame, void* args)
	{
		//init pointers and pixel coord
		//compute target pixel index for thread and block
		int posX = threadIdx.x + (blockIdx.x * blockDim.x);
		int posY = threadIdx.y + (blockIdx.y * blockDim.y);
		//posY += StartY;

		Color* mainFrame = (Color*)frame;
		MandelbrotView* mView = (MandelbrotView*)args;
		double Scale = mView->scale;

		int newIndex = (posY * width) + posX;

		Color& target = mainFrame[newIndex];


		int x = posX;
		int y = posY;

		double centerX = -0.5;
		double centerY = 0.5;

		double ResultX;
		double ResultY;

		double Zx = 0;
		double Zy = 0;
		double Zx_x2 = 0;
		double Zy_x2 = 0;

		const double MinimumResultX = centerX - Scale;
		const double MaximumResultX = centerX + Scale;
		const double MinimumResultY = centerY - Scale;
		const double MaximumResultY = centerY + Scale;

		
		double PixelWidth = (MaximumResultX - MinimumResultX) / width;
		double PixelHeight = (MaximumResultY - MinimumResultY) / height;


		int Iteration = 0;
		int MaxIteration = mView->iteration;
		
		const double EscapeRadius = 2.0L;
		double EscapeRadius_x2 = EscapeRadius * EscapeRadius;

		ResultX = (MinimumResultX + PixelWidth * x) + mView->x;
		ResultY = (MinimumResultY + PixelHeight * y) + mView->y;

		if (fabs(ResultY) < PixelHeight / 2) ResultY = 0.0;

        for (; Iteration < MaxIteration && ((Zx_x2 + Zy_x2) < EscapeRadius_x2); Iteration++)
        {
            Zy = 2 * Zx * Zy + ResultY;
            Zx = Zx_x2 - Zy_x2 + ResultX;
            Zx_x2 = Zx * Zx;
            Zy_x2 = Zy * Zy;
        }

		double NewValue = 1.0 * (Iteration % mView->iteration);
		FColor HDRColor = HSV2RGB(Iteration % 361, 0.01, NewValue);
		target = FColor2Color(HDRColor);
	}

	__global__ void JuliaKernelFunc(int width, int height, pFrame frame, void* args)
	{
		int posX = threadIdx.x + (blockIdx.x * blockDim.x);
		int posY = threadIdx.y + (blockIdx.y * blockDim.y);
		//posY += StartY;

		Color* mainFrame = (Color*)frame;
		MandelbrotView* mView = (MandelbrotView*)args;

		double cRe, cIm;
		double newRe, newIm, oldRe, oldIm;
		int newIndex = (posY * width) + posX;

		Color& target = mainFrame[newIndex];

		//pick some values for the constant c, this determines the shape of the Julia Set
		cRe = -0.7;
		cIm = 0.27015;

		double ScaledX = mView->x * 0.03;
		ScaledX /= mView->scale * 2;
		double ScaledY = mView->y * 0.03;
		ScaledY /= mView->scale * 2;

		//calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
		newRe = 1.5 * (posX - width / 2) / (0.5 * mView->scale * width) + ScaledX;
		newIm = (posY - height / 2) / (0.5 * mView->scale * height) + ScaledY;

		//i will represent the number of iterations
		int i;
		//start the iteration process
		for (i = 0; i < mView->iteration; i++)
		{
			//remember value of previous iteration
			oldRe = newRe;
			oldIm = newIm;
			//the actual iteration, the real and imaginary part are calculated
			newRe = oldRe * oldRe - oldIm * oldIm + cRe;
			newIm = 2 * oldRe * oldIm + cIm;
			//if the point is outside the circle with radius 2: stop
			if ((newRe * newRe + newIm * newIm) > 4) break;
		}

		FColor FResult = HSV2RGB((double)(i % 361), 0.8, 1.0 * (i < mView->iteration));

		target = FColor2Color(FResult);
	}

}