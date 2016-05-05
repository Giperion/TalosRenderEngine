#include "stdafx.h"
#include "CLEngineRenderer.h"
#include "GPUImage.h"

#define CHECKCLRESULT(result, function) if (!CLEngineRenderer::CheckClResult (result, UnicodeString (#function))) \
	return RSTATUS_INTERNALERROR

CLEngineRenderer::CLEngineRenderer()
{
	//init OpenCL
}

CLEngineRenderer::~CLEngineRenderer()
{
	//deinit OpenCL
}

pFrame CLEngineRenderer::GetRenderFrame()
{
	return 0;
}

void CLEngineRenderer::Render(RenderArgs* args)
{
	
}

RendererStatus CLEngineRenderer::RenderInit(PresentMethod method, class DrawEngine* presenter)
{
	if (method != PM_OpenGL)
	{
		Log::GetInstance()->PrintMsg(UnicodeString(L"CLEngineRenderer::RenderInit: Not OpenGL presenters not supported"));
		return RSTATUS_INVALIDPARAM;
	}

	cl_int statusCode;

	//ןכאעפמנלו ÀיÄו
	cl_platform_id platforms[5];
	cl_uint AvaliablePlatforms;
	statusCode = clGetPlatformIDs(5, &platforms[0], &AvaliablePlatforms);
	CHECKCLRESULT(statusCode, "clGetPlatformIDs");
	if (AvaliablePlatforms > 1)
	{
		LOG(L"CLEngineRenderer::RenderInit: Multiple OpenCL platforms? Application can use only one");
	}
	if (AvaliablePlatforms < 1)
	{
		LOG(L"CLEngineRenderer::RenderInit: No avaliable OpenCL platforms: abort");
		return RSTATUS_INTERNALERROR;
	}

	cl_device_id devices[100];
	ZeroMemory(&devices[0], sizeof(cl_device_id) * 100);
	cl_uint AvaliableDevices;

	statusCode = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 100, &devices[0], &AvaliableDevices);
	CHECKCLRESULT(statusCode, "clGetDeviceIDs");

	cl_ulong DeviceMaxMemAllocSize;

	statusCode = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &DeviceMaxMemAllocSize, NULL);
	CHECKCLRESULT(statusCode, "clGetDeviceInfo");
	////create context
	//cl_context_properties properties[] =
	//{

	//};

	return RSTATUS_OK;
}

void CLEngineRenderer::RenderDestroy(PresentMethod method, class DrawEngine* presenter)
{

}

bool CLEngineRenderer::CheckClResult(cl_int CL_code, UnicodeString& FunctionName)
{
	if (CL_code != CL_SUCCESS)
	{
		UnicodeString ErrMsg = UnicodeString(L"ERROR IN OPENCL RENDER ENGINE: \r\n\tFUNCTION: ");
		ErrMsg += FunctionName;
		ErrMsg += L"\r\n\tCODE: ";
		
		switch (CL_code)
		{
		case CL_DEVICE_NOT_FOUND:
			ErrMsg += L"CL_DEVICE_NOT_FOUND";
			break;
		case CL_DEVICE_NOT_AVAILABLE:
			ErrMsg += L"CL_DEVICE_NOT_AVAILABLE";
			break;
		case CL_COMPILER_NOT_AVAILABLE:
			ErrMsg += L"CL_COMPILER_NOT_AVAILABLE";
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:      
			ErrMsg += L"CL_MEM_OBJECT_ALLOCATION_FAILURE";
			break;
		case CL_OUT_OF_RESOURCES: 
			ErrMsg += L"CL_OUT_OF_RESOURCES";
			break;
		case CL_OUT_OF_HOST_MEMORY:               
			ErrMsg += L"CL_OUT_OF_HOST_MEMORY";
			break;
		case CL_PROFILING_INFO_NOT_AVAILABLE:     
			ErrMsg += L"CL_PROFILING_INFO_NOT_AVAILABLE";
			break;
		case CL_MEM_COPY_OVERLAP:
			ErrMsg += L"CL_MEM_COPY_OVERLAP";
			break;
		case CL_IMAGE_FORMAT_MISMATCH:
			ErrMsg += L"CL_IMAGE_FORMAT_MISMATCH";
			break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:
			ErrMsg += L"CL_IMAGE_FORMAT_NOT_SUPPORTED";
			break;
		case CL_BUILD_PROGRAM_FAILURE:
			ErrMsg += L"CL_BUILD_PROGRAM_FAILURE";
			break;
		case CL_MAP_FAILURE:
			ErrMsg += L"CL_MAP_FAILURE";
			break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			ErrMsg += L"CL_MISALIGNED_SUB_BUFFER_OFFSET";
			break;
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
			ErrMsg += L"CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
			break;
		case CL_INVALID_VALUE:
			ErrMsg += L"CL_INVALID_VALUE";
			break;
		case CL_INVALID_DEVICE_TYPE:
			ErrMsg += L"CL_INVALID_DEVICE_TYPE";
			break;
		case CL_INVALID_PLATFORM:
			ErrMsg += L"CL_INVALID_PLATFORM";
			break;
		case CL_INVALID_DEVICE:
			ErrMsg += L"CL_INVALID_DEVICE";
			break;
		case CL_INVALID_CONTEXT:
			ErrMsg += L"CL_INVALID_CONTEXT";
			break;
		case CL_INVALID_QUEUE_PROPERTIES:
			ErrMsg += L"CL_INVALID_QUEUE_PROPERTIES";
			break;
		case CL_INVALID_COMMAND_QUEUE:
			ErrMsg += L"CL_INVALID_COMMAND_QUEUE";
			break;
		case CL_INVALID_HOST_PTR:
			ErrMsg += L"CL_INVALID_HOST_PTR";
			break;
		case CL_INVALID_MEM_OBJECT:
			ErrMsg += L"CL_INVALID_MEM_OBJECT";
			break;
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
			ErrMsg += L"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
			break;
		case CL_INVALID_IMAGE_SIZE:
			ErrMsg += L"CL_INVALID_IMAGE_SIZE";
			break;
		case CL_INVALID_SAMPLER:
			ErrMsg += L"CL_INVALID_SAMPLER";
			break;
		case CL_INVALID_BINARY:
			ErrMsg += L"CL_INVALID_BINARY";
			break;
		case CL_INVALID_BUILD_OPTIONS:
			ErrMsg += L"CL_INVALID_BUILD_OPTIONS";
			break;
		case CL_INVALID_PROGRAM:
			ErrMsg += L"CL_INVALID_PROGRAM";
			break;
		case CL_INVALID_PROGRAM_EXECUTABLE:
			ErrMsg += L"CL_INVALID_PROGRAM_EXECUTABLE";
			break;
		case CL_INVALID_KERNEL_NAME:
			ErrMsg += L"CL_INVALID_KERNEL_NAME";
			break;
		case CL_INVALID_KERNEL_DEFINITION:
			ErrMsg += L"CL_INVALID_KERNEL_DEFINITION";
			break;
		case CL_INVALID_KERNEL:
			ErrMsg += L"CL_INVALID_KERNEL";
			break;
		case CL_INVALID_ARG_INDEX:
			ErrMsg += L"CL_INVALID_ARG_INDEX";
			break;
		case CL_INVALID_ARG_VALUE:
			ErrMsg += L"CL_INVALID_ARG_VALUE";
			break;
		case CL_INVALID_ARG_SIZE:
			ErrMsg += L"CL_INVALID_ARG_SIZE";
			break;
		case CL_INVALID_KERNEL_ARGS:
			ErrMsg += L"CL_INVALID_KERNEL_ARGS";
			break;
		case CL_INVALID_WORK_DIMENSION:
			ErrMsg += L"CL_INVALID_WORK_DIMENSION";
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			ErrMsg += L"CL_INVALID_WORK_GROUP_SIZE";
			break;
		case CL_INVALID_WORK_ITEM_SIZE:
			ErrMsg += L"CL_INVALID_WORK_ITEM_SIZE";
			break;
		case CL_INVALID_GLOBAL_OFFSET:
			ErrMsg += L"CL_INVALID_GLOBAL_OFFSET";
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			ErrMsg += L"CL_INVALID_EVENT_WAIT_LIST";
			break;
		case CL_INVALID_EVENT:
			ErrMsg += L"CL_INVALID_EVENT";
			break;
		case CL_INVALID_OPERATION:
			ErrMsg += L"CL_INVALID_OPERATION";
			break;
		case CL_INVALID_GL_OBJECT:
			ErrMsg += L"CL_INVALID_GL_OBJECT";
			break;
		case CL_INVALID_BUFFER_SIZE:
			ErrMsg += L"CL_INVALID_BUFFER_SIZE";
			break;
		case CL_INVALID_MIP_LEVEL:
			ErrMsg += L"CL_INVALID_MIP_LEVEL";
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			ErrMsg += L"CL_INVALID_GLOBAL_WORK_SIZE";
			break;
		case CL_INVALID_PROPERTY:
			ErrMsg += L"CL_INVALID_PROPERTY";
			break;
		default:
			ErrMsg += "UNKNOWN_CODE";
			break;
		}
		Log::GetInstance()->PrintMsg(ErrMsg);
		return false;
	}
	return true;
}

#undef CHECKCLRESULT