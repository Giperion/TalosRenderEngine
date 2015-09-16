#include "cuda_runtime.h"
#include "math_functions.h"
#include "EngineCuda_C.cuh"

extern "C"
{
	//Как тут в вашем С устроить объекты, а?...
	int status;
	int ResourceCounter;
	wchar_t* DebugMessage;
	int BestDeviceID;
	cudaError_t errcode;
	//device specific
	int cores;

	void* argsMem;

	/*
	* По текущей реализации ширина кадра формируется значением максимального числа потоков на блок
	* Все потому что пока что задача кидается по типу: Одна линия кадра = Один блок
	* Что позволяет по максимуму создать блоки и потоки за один вызов.
	*/
	int height;

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

	inline int GetBestDeviceID(int Count)
	{
		int curDeviceID = 0;
		int BestComputePower = 0;
		int BestComputeDevice;
		int deviceProhibitenCounter = 0;
		cudaDeviceProp curDeviceProp;
		while (curDeviceID < Count)
		{
			cudaGetDeviceProperties(&curDeviceProp, curDeviceID);
			if (curDeviceProp.computeMode != cudaComputeMode::cudaComputeModeProhibited)
			{
				if (curDeviceProp.major > 0 && curDeviceProp.major < 9999)
				{
					if (BestComputePower < curDeviceProp.major) BestComputeDevice = curDeviceID;
				}
			}
			else
			{
				deviceProhibitenCounter++;
			}
			curDeviceID++;
		}

		if (deviceProhibitenCounter == Count)
		{
			//Òÿæåëûé ñëó÷àé
			DebugMessage = L"All devices in the system prohibiten computeMode. Please google it, and try again.";
			status = CENGINE_STATUS_FATALERROR;
			return -1;
		}
		return BestComputeDevice;
	}
	
	const char* GetErrorString(cudaError_t errcode)
	{
		return cudaGetErrorString(errcode);
	}


	bool cuda_init()
	{
		status = CENGINE_STATUS_INIT;
		DebugMessage = L" ";
		ResourceCounter = 0;
		argsMem = nullptr;
		//Determine if we have CUDA device?
		int Count;
		cudaDeviceProp deviceProp;
		errcode = cudaGetDeviceCount(&Count);
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
		int DeviceID = 0;

		//Several devices?
		if (Count > 1)
		{
			DebugMessage = L"Several devices detected! At this point there is no sync code to provide multidevice rendering, sorry.\nHowewer we choose the most powerfull device in the system...";
			//Use a best device. Also check compute_mode prohibiten
			BestDeviceID = GetBestDeviceID(Count);
			errcode = cudaSetDevice(BestDeviceID);
			if (BestDeviceID == -1) return false;
		}
		else
		{
			//if only one device, we at least must check is compute mode is prohibiten?
			errcode = cudaGetDeviceProperties(&deviceProp, DeviceID);
			if (deviceProp.computeMode == cudaComputeMode::cudaComputeModeProhibited)
			{
				status = CENGINE_STATUS_FATALERROR;
				DebugMessage = L"Device computeMode set to prohibiten! Can't compute!";
				return false;
			}
			BestDeviceID = DeviceID;
			errcode = cudaSetDevice(BestDeviceID);
		}

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
		
		ResourceCounter++;
		if (ResourceCounter == CENGINE_MAX_CUDA_RESOURCES)
		{
			status = CENGINE_STATUS_FATALERROR;
			return result;
		}
		
		status = CENGINE_STATUS_IDLE;
		return result;
	}

	void cuda_FreeTexture(cudaImage image)
	{

		ResourceCounter--;
		return;
	}

	void cuda_deinit()
	{
		if (status != CENGINE_STATUS_IDLE)
		{
			//TODO Sync code
		}
		if (ResourceCounter != 0)
		{
			status = CENGINE_STATUS_FATALERROR;
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
		dim3 grid (256, 256, 1);
		dim3 blocks(width / grid.x, height / grid.y, 1);
		if (argsMem != nullptr) cudaFree(argsMem);
		
		errcode = cudaConfigureCall(grid, blocks);

		errcode = cudaSetupArgument(&width, sizeof(int), 0);
		errcode = cudaSetupArgument(&height,sizeof(int), sizeof (int));
		errcode = cudaSetupArgument(&frame,sizeof(pFrame), sizeof (int)* 2);
		if (args != nullptr)
		{
			errcode = cudaMalloc(&argsMem, argsSize);
			errcode = cudaMemcpy(argsMem, args, argsSize, cudaMemcpyKind::cudaMemcpyHostToDevice);
			errcode = cudaSetupArgument(&argsMem, sizeof(void*), sizeof(int) * 2 + sizeof(pFrame));
		}


		errcode = cudaLaunch(testKernelFunc);

		return errcode;
		//testKernelFunc <<<grid, blocks >>> (width, height, frame);
	}

	__device__ inline int Lerp(int start, int end, double value)
	{
		return start + (end - start) * value;
	}

	//dim3 blockIdx <- gridDim
	//dim3 threadIdx <- blocks

	__global__ void testKernelFunc(int width, int height, pFrame frame, void* args)
	{
		//init pointers and pixel coord
		//compute target pixel index for thread and block
		int posX = threadIdx.x + (blockIdx.x * blockDim.x);
		int posY = threadIdx.y + (blockIdx.y * blockDim.y);

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

		double Value = (1.0 / (double)80) *  Iteration;
		//Ðàíüøå âîçâðàùàëè öâåò ïî òàáëèöå, íî óâû, ýòî íå ýôôåêòèâíûé ñïîñîá
		//return ResoulveColor(Iteration);
		int grayscaleComp = Lerp(0, 255, Value);

		target.R = grayscaleComp; target.G = grayscaleComp; target.B = grayscaleComp;
		target.A = 255;
	}



}