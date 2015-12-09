/*-------------------------------------------------------------------------
 * Vulkan CTS Framework
 * --------------------
 *
 * Copyright (c) 2015 Google Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be
 * included in all copies or substantial portions of the Materials.
 *
 * The Materials are Confidential Information as defined by the
 * Khronos Membership Agreement until designated non-confidential by
 * Khronos, at which point this condition clause shall be removed.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *//*!
 * \file
 * \brief Null (dummy) Vulkan implementation.
 *//*--------------------------------------------------------------------*/

#include "vkNullDriver.hpp"
#include "vkPlatform.hpp"
#include "vkImageUtil.hpp"
#include "tcuFunctionLibrary.hpp"
#include "deMemory.h"

#include <stdexcept>
#include <algorithm>

namespace vk
{

namespace
{

using std::vector;

#define VK_NULL_RETURN(STMT)					\
	do {										\
		try {									\
			STMT;								\
			return VK_SUCCESS;					\
		} catch (const std::bad_alloc&) {		\
			return VK_ERROR_OUT_OF_HOST_MEMORY;	\
		} catch (VkResult res) {				\
			return res;							\
		}										\
	} while (deGetFalse())

// \todo [2015-07-14 pyry] Check FUNC type by checkedCastToPtr<T>() or similar
#define VK_NULL_FUNC_ENTRY(NAME, FUNC)	{ #NAME, (deFunctionPtr)FUNC }

#define VK_NULL_DEFINE_DEVICE_OBJ(NAME)				\
struct NAME											\
{													\
	NAME (VkDevice, const Vk##NAME##CreateInfo*) {}	\
}

VK_NULL_DEFINE_DEVICE_OBJ(Fence);
VK_NULL_DEFINE_DEVICE_OBJ(Semaphore);
VK_NULL_DEFINE_DEVICE_OBJ(Event);
VK_NULL_DEFINE_DEVICE_OBJ(QueryPool);
VK_NULL_DEFINE_DEVICE_OBJ(BufferView);
VK_NULL_DEFINE_DEVICE_OBJ(ImageView);
VK_NULL_DEFINE_DEVICE_OBJ(ShaderModule);
VK_NULL_DEFINE_DEVICE_OBJ(PipelineCache);
VK_NULL_DEFINE_DEVICE_OBJ(PipelineLayout);
VK_NULL_DEFINE_DEVICE_OBJ(RenderPass);
VK_NULL_DEFINE_DEVICE_OBJ(DescriptorSetLayout);
VK_NULL_DEFINE_DEVICE_OBJ(Sampler);
VK_NULL_DEFINE_DEVICE_OBJ(Framebuffer);
VK_NULL_DEFINE_DEVICE_OBJ(CommandPool);

class Instance
{
public:
										Instance		(const VkInstanceCreateInfo* instanceInfo);
										~Instance		(void) {}

	PFN_vkVoidFunction					getProcAddr		(const char* name) const { return (PFN_vkVoidFunction)m_functions.getFunction(name); }

private:
	const tcu::StaticFunctionLibrary	m_functions;
};

class Device
{
public:
										Device			(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* deviceInfo);
										~Device			(void) {}

	PFN_vkVoidFunction					getProcAddr		(const char* name) const { return (PFN_vkVoidFunction)m_functions.getFunction(name); }

private:
	const tcu::StaticFunctionLibrary	m_functions;
};

class Pipeline
{
public:
	Pipeline (VkDevice, const VkGraphicsPipelineCreateInfo*) {}
	Pipeline (VkDevice, const VkComputePipelineCreateInfo*) {}
};

void* allocateHeap (const VkMemoryAllocateInfo* pAllocInfo)
{
	// \todo [2015-12-03 pyry] Alignment requirements?
	// \todo [2015-12-03 pyry] Empty allocations okay?
	if (pAllocInfo->allocationSize > 0)
	{
		void* const heapPtr = deMalloc((size_t)pAllocInfo->allocationSize);
		if (!heapPtr)
			throw std::bad_alloc();
		return heapPtr;
	}
	else
		return DE_NULL;
}

void freeHeap (void* ptr)
{
	deFree(ptr);
}

class DeviceMemory
{
public:
						DeviceMemory	(VkDevice, const VkMemoryAllocateInfo* pAllocInfo)
							: m_memory(allocateHeap(pAllocInfo))
						{
						}
						~DeviceMemory	(void)
						{
							freeHeap(m_memory);
						}

	void*				getPtr			(void) const { return m_memory; }

private:
	void* const			m_memory;
};

class Buffer
{
public:
						Buffer		(VkDevice, const VkBufferCreateInfo* pCreateInfo)
							: m_size(pCreateInfo->size)
						{}

	VkDeviceSize		getSize		(void) const { return m_size;	}

private:
	const VkDeviceSize	m_size;
};

class Image
{
public:
								Image			(VkDevice, const VkImageCreateInfo* pCreateInfo)
									: m_imageType	(pCreateInfo->imageType)
									, m_format		(pCreateInfo->format)
									, m_extent		(pCreateInfo->extent)
									, m_samples		(pCreateInfo->samples)
								{}

	VkImageType					getImageType	(void) const { return m_imageType;	}
	VkFormat					getFormat		(void) const { return m_format;		}
	VkExtent3D					getExtent		(void) const { return m_extent;		}
	VkSampleCountFlagBits		getSamples		(void) const { return m_samples;	}

private:
	const VkImageType			m_imageType;
	const VkFormat				m_format;
	const VkExtent3D			m_extent;
	const VkSampleCountFlagBits	m_samples;
};

class CommandBuffer
{
public:
						CommandBuffer(VkDevice, VkCommandPool, VkCommandBufferLevel)
						{}
};

class DescriptorSet
{
public:
	DescriptorSet (VkDevice, VkDescriptorPool, VkDescriptorSetLayout) {}
};

class DescriptorPool
{
public:
										DescriptorPool	(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo)
											: m_device	(device)
											, m_flags	(pCreateInfo->flags)
										{}
										~DescriptorPool	(void)
										{
											reset();
										}

	VkDescriptorSet						allocate		(VkDescriptorSetLayout setLayout);
	void								free			(VkDescriptorSet set);

	void								reset			(void);

private:
	const VkDevice						m_device;
	const VkDescriptorPoolCreateFlags	m_flags;

	vector<DescriptorSet*>				m_managedSets;
};

VkDescriptorSet DescriptorPool::allocate (VkDescriptorSetLayout setLayout)
{
	DescriptorSet* const	impl	= new DescriptorSet(m_device, VkDescriptorPool(reinterpret_cast<deUintptr>(this)), setLayout);

	try
	{
		m_managedSets.push_back(impl);
	}
	catch (...)
	{
		delete impl;
		throw;
	}

	return VkDescriptorSet(reinterpret_cast<deUintptr>(impl));
}

void DescriptorPool::free (VkDescriptorSet set)
{
	DescriptorSet* const	impl	= reinterpret_cast<DescriptorSet*>((deUintptr)set.getInternal());

	DE_ASSERT(m_flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

	delete impl;

	for (size_t ndx = 0; ndx < m_managedSets.size(); ++ndx)
	{
		if (m_managedSets[ndx] == impl)
		{
			std::swap(m_managedSets[ndx], m_managedSets.back());
			m_managedSets.pop_back();
			return;
		}
	}

	DE_FATAL("VkDescriptorSet not owned by VkDescriptorPool");
}

void DescriptorPool::reset (void)
{
	for (size_t ndx = 0; ndx < m_managedSets.size(); ++ndx)
		delete m_managedSets[ndx];
	m_managedSets.clear();
}

extern "C"
{

PFN_vkVoidFunction getInstanceProcAddr (VkInstance instance, const char* pName)
{
	return reinterpret_cast<Instance*>(instance)->getProcAddr(pName);
}

PFN_vkVoidFunction getDeviceProcAddr (VkDevice device, const char* pName)
{
	return reinterpret_cast<Device*>(device)->getProcAddr(pName);
}

VkResult createGraphicsPipelines (VkDevice device, VkPipelineCache, deUint32 count, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks*, VkPipeline* pPipelines)
{
	for (deUint32 ndx = 0; ndx < count; ndx++)
		pPipelines[ndx] = VkPipeline((deUint64)(deUintptr)new Pipeline(device, pCreateInfos+ndx));
	return VK_SUCCESS;
}

VkResult createComputePipelines (VkDevice device, VkPipelineCache, deUint32 count, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks*, VkPipeline* pPipelines)
{
	for (deUint32 ndx = 0; ndx < count; ndx++)
		pPipelines[ndx] = VkPipeline((deUint64)(deUintptr)new Pipeline(device, pCreateInfos+ndx));
	return VK_SUCCESS;
}

VkResult enumeratePhysicalDevices (VkInstance, deUint32* pPhysicalDeviceCount, VkPhysicalDevice* pDevices)
{
	if (pDevices && *pPhysicalDeviceCount >= 1u)
		*pDevices = reinterpret_cast<VkPhysicalDevice>((void*)(deUintptr)1u);

	*pPhysicalDeviceCount = 1;

	return VK_SUCCESS;
}

void getPhysicalDeviceProperties (VkPhysicalDevice, VkPhysicalDeviceProperties* props)
{
	deMemset(props, 0, sizeof(VkPhysicalDeviceProperties));

	props->apiVersion		= VK_API_VERSION;
	props->driverVersion	= 1u;
	props->deviceType		= VK_PHYSICAL_DEVICE_TYPE_OTHER;

	deMemcpy(props->deviceName, "null", 5);

	// \todo [2015-09-25 pyry] Fill in reasonable limits
	props->limits.maxTexelBufferElements	= 8096;
}

void getPhysicalDeviceQueueFamilyProperties (VkPhysicalDevice, deUint32* count, VkQueueFamilyProperties* props)
{
	if (props && *count >= 1u)
	{
		deMemset(props, 0, sizeof(VkQueueFamilyProperties));

		props->queueCount			= 1u;
		props->queueFlags			= VK_QUEUE_GRAPHICS_BIT|VK_QUEUE_COMPUTE_BIT;
		props->timestampValidBits	= 64;
	}

	*count = 1u;
}

void getPhysicalDeviceMemoryProperties (VkPhysicalDevice, VkPhysicalDeviceMemoryProperties* props)
{
	deMemset(props, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	props->memoryTypeCount				= 1u;
	props->memoryTypes[0].heapIndex		= 0u;
	props->memoryTypes[0].propertyFlags	= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

	props->memoryHeapCount				= 1u;
	props->memoryHeaps[0].size			= 1ull << 31;
	props->memoryHeaps[0].flags			= 0u;
}

void getPhysicalDeviceFormatProperties (VkPhysicalDevice, VkFormat, VkFormatProperties* pFormatProperties)
{
	const VkFormatFeatureFlags	allFeatures	= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
											| VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
											| VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
											| VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
											| VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
											| VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
											| VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT
											| VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
											| VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT
											| VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
											| VK_FORMAT_FEATURE_BLIT_SRC_BIT
											| VK_FORMAT_FEATURE_BLIT_DST_BIT;

	pFormatProperties->linearTilingFeatures		= allFeatures;
	pFormatProperties->optimalTilingFeatures	= allFeatures;
	pFormatProperties->bufferFeatures			= allFeatures;
}

void getBufferMemoryRequirements (VkDevice, VkBuffer bufferHandle, VkMemoryRequirements* requirements)
{
	const Buffer*	buffer	= reinterpret_cast<const Buffer*>(bufferHandle.getInternal());

	requirements->memoryTypeBits	= 1u;
	requirements->size				= buffer->getSize();
	requirements->alignment			= (VkDeviceSize)1u;
}

VkDeviceSize getPackedImageDataSize (VkFormat format, VkExtent3D extent, VkSampleCountFlagBits samples)
{
	return (VkDeviceSize)getPixelSize(mapVkFormat(format))
			* (VkDeviceSize)extent.width
			* (VkDeviceSize)extent.height
			* (VkDeviceSize)extent.depth
			* (VkDeviceSize)samples;
}

void getImageMemoryRequirements (VkDevice, VkImage imageHandle, VkMemoryRequirements* requirements)
{
	const Image*	image	= reinterpret_cast<const Image*>(imageHandle.getInternal());

	requirements->memoryTypeBits	= 1u;
	requirements->alignment			= 4u;
	requirements->size				= getPackedImageDataSize(image->getFormat(), image->getExtent(), image->getSamples());
}

VkResult mapMemory (VkDevice, VkDeviceMemory memHandle, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
	const DeviceMemory*	memory	= reinterpret_cast<DeviceMemory*>(memHandle.getInternal());

	DE_UNREF(size);
	DE_UNREF(flags);

	*ppData = (deUint8*)memory->getPtr() + offset;

	return VK_SUCCESS;
}

VkResult allocateDescriptorSets (VkDevice, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets)
{
	DescriptorPool* const	poolImpl	= reinterpret_cast<DescriptorPool*>((deUintptr)pAllocateInfo->descriptorPool.getInternal());

	for (deUint32 ndx = 0; ndx < pAllocateInfo->setLayoutCount; ++ndx)
	{
		try
		{
			pDescriptorSets[ndx] = poolImpl->allocate(pAllocateInfo->pSetLayouts[ndx]);
		}
		catch (const std::bad_alloc&)
		{
			for (deUint32 freeNdx = 0; freeNdx < ndx; freeNdx++)
				delete reinterpret_cast<DescriptorSet*>((deUintptr)pDescriptorSets[freeNdx].getInternal());

			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		catch (VkResult res)
		{
			for (deUint32 freeNdx = 0; freeNdx < ndx; freeNdx++)
				delete reinterpret_cast<DescriptorSet*>((deUintptr)pDescriptorSets[freeNdx].getInternal());

			return res;
		}
	}

	return VK_SUCCESS;
}

void freeDescriptorSets (VkDevice, VkDescriptorPool descriptorPool, deUint32 count, const VkDescriptorSet* pDescriptorSets)
{
	DescriptorPool* const	poolImpl	= reinterpret_cast<DescriptorPool*>((deUintptr)descriptorPool.getInternal());

	for (deUint32 ndx = 0; ndx < count; ++ndx)
		poolImpl->free(pDescriptorSets[ndx]);
}

VkResult resetDescriptorPool (VkDevice, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags)
{
	DescriptorPool* const	poolImpl	= reinterpret_cast<DescriptorPool*>((deUintptr)descriptorPool.getInternal());

	poolImpl->reset();

	return VK_SUCCESS;
}

VkResult allocateCommandBuffers (VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers)
{
	if (pAllocateInfo && pCommandBuffers)
	{
		for (deUint32 ndx = 0; ndx < pAllocateInfo->bufferCount; ++ndx)
		{
			pCommandBuffers[ndx] = reinterpret_cast<VkCommandBuffer>(new CommandBuffer(device, pAllocateInfo->commandPool, pAllocateInfo->level));
		}
	}

	return VK_SUCCESS;
}

void freeCommandBuffers (VkDevice device, VkCommandPool commandPool, deUint32 commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
	DE_UNREF(device);
	DE_UNREF(commandPool);

	for (deUint32 ndx = 0; ndx < commandBufferCount; ++ndx)
		delete reinterpret_cast<CommandBuffer*>(pCommandBuffers[ndx]);
}

#include "vkNullDriverImpl.inl"

} // extern "C"

Instance::Instance (const VkInstanceCreateInfo*)
	: m_functions(s_instanceFunctions, DE_LENGTH_OF_ARRAY(s_instanceFunctions))
{
}

Device::Device (VkPhysicalDevice, const VkDeviceCreateInfo*)
	: m_functions(s_deviceFunctions, DE_LENGTH_OF_ARRAY(s_deviceFunctions))
{
}

class NullDriverLibrary : public Library
{
public:
										NullDriverLibrary (void)
											: m_library	(s_platformFunctions, DE_LENGTH_OF_ARRAY(s_platformFunctions))
											, m_driver	(m_library)
										{}

	const PlatformInterface&			getPlatformInterface	(void) const { return m_driver;	}

private:
	const tcu::StaticFunctionLibrary	m_library;
	const PlatformDriver				m_driver;
};

} // anonymous

Library* createNullDriver (void)
{
	return new NullDriverLibrary();
}

} // vk
