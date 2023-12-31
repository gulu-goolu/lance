#pragma once

#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "lance/core/object.h"
#include "lance/core/util.h"
#include "vulkan/vulkan_core.h"

namespace lance {
namespace rendering {
class Device;
class ShaderModule;
class CommandBuffer;
class Buffer;
class DescriptorSet;

class Instance : public core::Inherit<Instance, core::Object> {
 public:
  // create instance
  static absl::StatusOr<core::RefCountPtr<Instance>> create(absl::Span<const char*> layers,
                                                            absl::Span<const char*> extensions);

  // create a instance for 3d rendering
  static absl::StatusOr<core::RefCountPtr<Instance>> create_for_3d();

  Instance(VkInstance vk_instance) : vk_instance_(vk_instance) {}

  ~Instance() override;

  VkInstance vk_instance() const { return vk_instance_; }

  absl::StatusOr<std::vector<VkPhysicalDevice>> enumerate_physical_devices() const;

  absl::StatusOr<core::RefCountPtr<Device>> create_device(absl::Span<const char*> extensions);

  absl::StatusOr<core::RefCountPtr<Device>> create_device_for_graphics();

 private:
  VkInstance vk_instance_{VK_NULL_HANDLE};
};

class Surface : public core::Inherit<Surface, core::Object> {
 public:
  Surface(core::RefCountPtr<Instance> instance, VkSurfaceKHR vk_surface)
      : instance_(instance), vk_surface_(vk_surface) {}

  ~Surface() override;

  VkSurfaceKHR vk_surface() const { return vk_surface_; }

 private:
  core::RefCountPtr<Instance> instance_;
  VkSurfaceKHR vk_surface_{VK_NULL_HANDLE};
};

class Device : public core::Inherit<Device, core::Object> {
 public:
  explicit Device(core::RefCountPtr<Instance> instance, VkPhysicalDevice vk_physical_device,
                  VkDevice vk_device, absl::Span<const uint32_t> queue_family_indices);

  ~Device();

  VkDevice vk_device() const { return vk_device_; }

  absl::StatusOr<core::RefCountPtr<ShaderModule>> create_shader_module(const core::Blob* blob);

  absl::StatusOr<core::RefCountPtr<ShaderModule>> create_shader_from_source(
      VkShaderStageFlagBits stage, const char* source);

  absl::StatusOr<uint32_t> find_queue_family_index(VkQueueFlags flags) const;

  absl::Status submit(uint32_t queue_family_index,
                      absl::Span<const VkCommandBuffer> vk_command_buffers);

  absl::StatusOr<uint32_t> find_memory_type_index(uint32_t type_bits,
                                                  VkMemoryPropertyFlags flags) const;

  absl::StatusOr<core::RefCountPtr<Buffer>> create_buffer(
      VkBufferUsageFlags usage, size_t size, VkMemoryPropertyFlags memory_property_flags);

 private:
  core::RefCountPtr<Instance> instance_;
  VkPhysicalDevice vk_physical_device_{VK_NULL_HANDLE};
  VkDevice vk_device_{VK_NULL_HANDLE};
  std::vector<uint32_t> queue_family_indices_;
};

class DeviceMemory : public core::Inherit<DeviceMemory, core::Object> {
 public:
  static absl::StatusOr<core::RefCountPtr<DeviceMemory>> create(
      const core::RefCountPtr<Device>& device, uint32_t memory_type_index, size_t allocation_size);

  DeviceMemory(core::RefCountPtr<Device> device, VkDeviceMemory vk_device_memory)
      : device_(device), vk_device_memory_(vk_device_memory) {}

  ~DeviceMemory();

  VkDeviceMemory vk_device_memory() const { return vk_device_memory_; }

  // map for read
  absl::StatusOr<void*> map(size_t offset, size_t size);

  //
  absl::Status unmap();

 private:
  core::Ref<Device> device_;
  VkDeviceMemory vk_device_memory_{VK_NULL_HANDLE};
};

class Buffer : public core::Inherit<Buffer, core::Object> {
 public:
  virtual Device* device() const = 0;

  virtual VkBuffer vk_buffer() const = 0;

  VkMemoryRequirements memory_requirements() const;
};

class Image : public core::Inherit<Image, core::Object> {
 public:
  ~Image();

  virtual Device* device() const = 0;

  virtual VkImage vk_image() const = 0;

  VkMemoryRequirements memory_requirements() const;
};

class ImageView : public core::Inherit<ImageView, core::Object> {
 public:
  ImageView(core::RefCountPtr<Device> device, VkImageView image_view)
      : device_(device), vk_image_view_(image_view) {}

  ~ImageView();

  VkImageView vk_image_view() const { return vk_image_view_; }

 private:
  core::RefCountPtr<Device> device_;
  VkImageView vk_image_view_{VK_NULL_HANDLE};
};

class ShaderModule : public core::Inherit<ShaderModule, core::Object> {
 public:
  ShaderModule(core::RefCountPtr<Device> device, VkShaderModule vk_shader_module)
      : device_(device), vk_shader_module_(vk_shader_module) {}

  ~ShaderModule();

  VkShaderModule vk_shader_module() const { return vk_shader_module_; }

 private:
  core::RefCountPtr<Device> device_;
  VkShaderModule vk_shader_module_{VK_NULL_HANDLE};
};

class DescriptorSetLayout : public core::Inherit<DescriptorSetLayout, core::Object> {
 public:
  static absl::StatusOr<core::Ref<DescriptorSetLayout>> create(
      const core::Ref<Device>& device, absl::Span<const VkDescriptorSetLayoutBinding> bindings);

  static absl::StatusOr<core::Ref<DescriptorSetLayout>> create_for_single_descriptor(
      const core::Ref<Device>& device, VkDescriptorType type, VkShaderStageFlags stage);

  DescriptorSetLayout(core::RefCountPtr<Device> device,
                      VkDescriptorSetLayout vk_descriptor_set_layout)
      : device_(device), vk_descriptor_set_layout_(vk_descriptor_set_layout) {}

  ~DescriptorSetLayout();

  VkDescriptorSetLayout vk_descriptor_set_layout() const { return vk_descriptor_set_layout_; }

 private:
  core::RefCountPtr<Device> device_;
  VkDescriptorSetLayout vk_descriptor_set_layout_{VK_NULL_HANDLE};
};

class PipelineLayout : public core::Inherit<PipelineLayout, core::Object> {
 public:
  PipelineLayout(core::RefCountPtr<Device> device, VkPipelineLayout vk_pipeline_layout)
      : device_(device), vk_pipeline_layout_(vk_pipeline_layout) {}

  ~PipelineLayout();

  VkPipelineLayout vk_pipeline_layout() const { return vk_pipeline_layout_; }

 private:
  core::RefCountPtr<Device> device_;
  VkPipelineLayout vk_pipeline_layout_{VK_NULL_HANDLE};
};

class Pipeline : public core::Inherit<Pipeline, core::Object> {
 public:
  Pipeline(core::RefCountPtr<Device> device, VkPipeline vk_pipeline,
           const core::RefCountPtr<PipelineLayout>& pipeline_layout)
      : device_(device), vk_pipeline_(vk_pipeline), pipeline_layout_(pipeline_layout) {}

  ~Pipeline();

  VkPipeline vk_pipeline() const { return vk_pipeline_; }

 private:
  core::RefCountPtr<Device> device_;
  VkPipeline vk_pipeline_{VK_NULL_HANDLE};
  core::RefCountPtr<PipelineLayout> pipeline_layout_;
};

class CommandPool : public core::Inherit<CommandPool, core::Object> {
 public:
  static absl::StatusOr<core::RefCountPtr<CommandPool>> create(
      const core::RefCountPtr<Device>& device, uint32_t queue_family_index);

  CommandPool(core::RefCountPtr<Device> device, VkCommandPool vk_command_pool)
      : device_(device), vk_command_pool_(vk_command_pool) {}

  ~CommandPool();

  const core::RefCountPtr<Device>& device() const { return device_; }

  VkCommandPool vk_command_pool() const { return vk_command_pool_; }

  absl::StatusOr<core::RefCountPtr<CommandBuffer>> allocate_command_buffer(
      VkCommandBufferLevel level);

 private:
  core::RefCountPtr<Device> device_;
  VkCommandPool vk_command_pool_{VK_NULL_HANDLE};
};

class CommandBuffer : public core::Inherit<CommandBuffer, core::Object> {
 public:
  CommandBuffer(core::RefCountPtr<CommandPool> command_pool, VkCommandBuffer vk_command_buffer)
      : command_pool_(command_pool), vk_command_buffer_(vk_command_buffer) {}

  ~CommandBuffer();

  VkCommandBuffer vk_command_buffer() const { return vk_command_buffer_; }

  absl::Status begin();
  absl::Status end();

  absl::Status add_temporary_resource(core::RefCountPtr<core::Object> resource);

 private:
  core::RefCountPtr<CommandPool> command_pool_;
  VkCommandBuffer vk_command_buffer_{VK_NULL_HANDLE};

  std::vector<core::RefCountPtr<core::Object>> temporary_resources_;
};

class Framebuffer : public core::Inherit<Framebuffer, core::Object> {
 public:
  Framebuffer(const core::RefCountPtr<Device>& device, VkFramebuffer vk_framebuffer)
      : device_(device), vk_framebuffer_(vk_framebuffer) {}

  ~Framebuffer();

  VkFramebuffer vk_framebuffer() const { return vk_framebuffer_; }

 private:
  core::RefCountPtr<Device> device_;
  VkFramebuffer vk_framebuffer_{VK_NULL_HANDLE};
};

class RenderPass : public core::Inherit<RenderPass, core::Object> {
 public:
  RenderPass(core::RefCountPtr<Device> device, VkRenderPass vk_render_pass)
      : device_(device), vk_render_pass_(vk_render_pass) {}

  ~RenderPass();

  VkRenderPass vk_render_pass() const { return vk_render_pass_; }

 private:
  core::RefCountPtr<Device> device_;
  VkRenderPass vk_render_pass_{VK_NULL_HANDLE};
};

class DescriptorPool : public core::Inherit<DescriptorPool, core::Object> {
 public:
  ~DescriptorPool();

  Device* device() const { return device_.get(); }

  VkDescriptorPool vk_descriptor_pool() const { return vk_descriptor_pool_; }

  absl::StatusOr<core::RefCountPtr<DescriptorSet>> allocate_descriptor_set(
      const DescriptorSetLayout* layout);

 private:
  core::RefCountPtr<Device> device_;
  VkDescriptorPool vk_descriptor_pool_{VK_NULL_HANDLE};
};

class DescriptorSet : public core::Inherit<DescriptorSet, core::Object> {
 public:
  virtual VkDescriptorSet vk_descriptor_set() const = 0;
};

class Swapchain : public core::Inherit<Swapchain, core::Object> {
 public:
  absl::Status acquire_next_image();

  absl::StatusOr<uint32_t> back_buffer_index() const;

  // present back buffer
  absl::Status present();
};

}  // namespace rendering
}  // namespace lance
