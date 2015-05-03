#include "cuda_runtime.h"

#define CENGINE_STATUS_INIT -1
#define CENGINE_STATUS_NOCUDA -2
#define CENGINE_STATUS_IDLE 1
#define CENGINE_STATUS_RENDERING 2
#define CENGINE_STATUS_RECONFIGURATE 3
#define CENGINE_STATUS_RENDERFINISHED 4
#define CENGINE_STATUS_FATALERROR 999

//Код под вопросом (Код под котом, доо)
#define CENGINE_MAX_CUDA_RESOURCES 256


extern "C"
{
	//Как тут в вашем С устроить объекты, а?...
	int status;
	int ResourceCounter;
	wchar_t* DebugMessage;

	//device specific
	int cores;

	/*
	* По текущей реализации ширина кадра формируется значением максимального числа потоков на блок
	* Все потому что пока что задача кидается по типу: Одна линия кадра = Один блок
	* Что позволяет по максимуму создать блоки и потоки за один вызов.
	*/
	int height;

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
			//Тяжелый случай
			DebugMessage = L"All devices in the system prohibiten computeMode. Please google it, and try again.";
			status = CENGINE_STATUS_FATALERROR;
			return -1;
		}
		return BestComputeDevice;
	}
	
	static struct cudaImage
	{
		int width;
		int height;
		size_t bytes;
		void* cudaData;
	};

	bool cuda_init()
	{
		status = CENGINE_STATUS_INIT;
		DebugMessage = L" ";
		cudaError_t errcode;
		ResourceCounter = 0;
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
		int DeviceID;
		errcode = cudaGetDevice(&DeviceID);

		//Several devices?
		if (Count > 1)
		{
			DebugMessage = L"Several devices detected! At this point there is no sync code to provide multidevice rendering, sorry.\nHowewer we choose the most powerfull device in the system...";
			//Use a best device. Also check compute_mode prohibiten
			int BestDeviceID = GetBestDeviceID(Count);
			if (BestDeviceID == -1) return false;
		}
		else
		{
			//if only one device, we at least must check is compute mode is prohibiten?
			cudaGetDeviceProperties(&deviceProp, DeviceID);
			if (deviceProp.computeMode == cudaComputeMode::cudaComputeModeProhibited)
			{
				status = CENGINE_STATUS_FATALERROR;
				DebugMessage = L"Device computeMode set to prohibiten! Can't compute!";
				return false;
			}
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



}