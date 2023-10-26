#include "vk_api.h"

#include "absl/strings/str_format.h"
#include "glog/logging.h"

#if defined(__linux__)
#include <dlfcn.h>
#elif defined(_WIN64)
#include <Windows.h>
#endif

namespace lance {
namespace rendering {
const VkApi* VkApi::get() {
  static const VkApi api;
  return &api;
}

VkApi::VkApi() {
#if defined(__linux__)
  shared_library_handle_ = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
  CHECK(shared_library_handle_ != nullptr) << "err_msg: " << dlerror();

  const auto get_symbol_by_name = [this](const char* name) {
    return dlsym(shared_library_handle_, name);
  };

#elif defined(_WIN64)
  shared_library_handle_ = LoadLibrary(TEXT("vulkan-1.dll"));
  CHECK(shared_library_handle_ != nullptr);

  const auto get_symbol_by_name = [this](const char* name) -> void* {
    return reinterpret_cast<void*>(
        GetProcAddress(reinterpret_cast<HMODULE>(shared_library_handle_), name));
  };
#else
  const auto get_symbol_by_name = [this](const char*) -> void* { return nullptr; };
#endif

#define VK_API_LOAD(API)                                                \
  do {                                                                  \
    API = reinterpret_cast<decltype(::API)*>(get_symbol_by_name(#API)); \
    CHECK(API != nullptr);                                              \
  } while (false)

  VK_API_LOAD(vkEnumerateInstanceLayerProperties);
  VK_API_LOAD(vkEnumerateInstanceExtensionProperties);

  VK_API_LOAD(vkCreateInstance);
  VK_API_LOAD(vkCreateDevice);
  VK_API_LOAD(vkAllocateMemory);
  VK_API_LOAD(vkCreateBuffer);
  VK_API_LOAD(vkCreateBufferView);
  VK_API_LOAD(vkCreateImage);
  VK_API_LOAD(vkCreateImageView);
  VK_API_LOAD(vkEnumeratePhysicalDevices);
  VK_API_LOAD(vkGetPhysicalDeviceProperties);
  VK_API_LOAD(vkGetPhysicalDeviceQueueFamilyProperties);

#undef VK_API_LOAD
}

VkApi::~VkApi() {
#if defined(__linux__)
  if (shared_library_handle_) {
    dlclose(shared_library_handle_);
  }
#elif defined(_WIN64)
  // windows
  if (shared_library_handle_) {
    CloseHandle(shared_library_handle_);
  }
#else
#endif
}

std::string VkResult_name(VkResult ret_code) {
  switch (ret_code) {
#define CASE_TO_STRING(NAME) \
  case NAME: {               \
    return #NAME;            \
  } break

    CASE_TO_STRING(VK_ERROR_EXTENSION_NOT_PRESENT);
    CASE_TO_STRING(VK_ERROR_OUT_OF_POOL_MEMORY);

#undef CASE_TO_STRING

    default: {
      return absl::StrFormat("UnknownError, ret_code: %d", static_cast<int32_t>(ret_code));
    }
  }
}
}  // namespace rendering
}  // namespace lance
