//
// Created by kodor on 12/25/21.
//

#include <spectrum.h>
#include "pbrt.h"

// Ray tracer
#include "api.h"
#include "parser.h"
#include "paramset.h"
#include "parallel.h"
#include "memory.h"
#include "imageio.h"

// Vulkan

#include "vulkanexamplebase.h"


static float currentPosition[4] = { 0, 0, 0, 0, },
        cameraPosition[4] = { 400, 20, 30, 0, },
        cameraLook[4] = { 0, 63, -100, 0, },
        lightPosition[4] = { 0 };
using namespace pbrt;

#define VERTEX_BUFFER_BIND_ID 0

enum class model_type { SPHERE = 0, TRIANGLE = 1, KETTLE = 2 };

struct Vertex {
    float pos[3];
    float uv[2];
    float normal[3];
};

struct Position {
    float pos[4];
};

struct Model {
    float pos[4];
    model_type type;
};



struct ParamListItem {
    std::string name;
    double *doubleValues = nullptr;
    const char **stringValues = nullptr;
    size_t size = 0;
    bool isString = false;
};

class VulkanExample : public VulkanExampleBase {
public:

    bool isPathSet = false;

    Options options;

    float cameraFOV = 45.0f;

    int32_t objectIndex = 0;
    int32_t accelStrucIndex = 0;
    int32_t sceneIndex = 0;
    std::vector<std::string> objectsAvailable = { "Сфера", "Треугольник", "Высокополигональная модель"};
    std::vector<std::string> accelStructsAvailable = { "BVH", "K-мерное дерево"};
    std::vector<std::string> scenesAvailable = { "killeroo.pbrt", "room.pbrt", "teapot-area-light.pbrt"};

    std::vector<Model> models;


    uint8_t * g_imageBufferVK;
    Point2i g_resolution;

    std::vector<std::string> scenesPaths =
            { "../scenes/killeroo-simple.pbrt",
              "../scenes/room.pbrt",
              "../scenes/teapot-area-light.pbrt"};


    struct Texture {
        VkSampler sampler;
        VkImage image;
        VkImageLayout imageLayout;
        VkDeviceMemory deviceMemory;
        VkImageView view;
        uint32_t width, height;
    } texture;


    std::string currentFile;

    struct {
        VkPipelineVertexInputStateCreateInfo  inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    vks::Buffer uniformBufferVS;
    vks::Buffer vertexBuffer;
    vks::Buffer indexBuffer;
    uint32_t indexCount;

    struct {
        glm::mat4 projection;
        glm::mat4 modelView;
        glm::vec4 viewPos;
        float lodBias = 0.0f;
    } uboVS;

    struct {
        VkPipeline solid;
    } pipelines;

    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptorSet;
    VkDescriptorSetLayout descriptorSetLayout;

    VulkanExample() : VulkanExampleBase() {
        title = "Трассировка пути";
        google::InitGoogleLogging("");
        pbrtInit(options);

        g_imageBufferVK = new uint8_t[700 * 700 * 4];
        g_resolution.x = 700;
        g_resolution.y = 700;

        camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
        camera.setRotation(glm::vec3(0.0f, 0.f, 0.0f));
        camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);

    }

    ~VulkanExample() {
        destroyTextureImage(texture);

        vkDestroyPipeline(device, pipelines.solid, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        vertexBuffer.destroy();
        indexBuffer.destroy();
        uniformBufferVS.destroy();

        pbrtCleanup();

    }

    void manualSceneLoad() {
        setLook();
        addCamera();
        addFilm();
        pbrtWorldBegin();
        addLight();
        addModels();
        pbrtWorldEnd();
    }

    void setLook() {

        ParamSet params;
        pbrtLookAt(cameraPosition[0],
                   cameraPosition[1],
                   cameraPosition[2],
                   cameraLook[0],
                   cameraLook[1],
                   cameraLook[2],
                   0, 0, 1
        );

    }

    void addCamera() {
        ParamSet params;

        std::unique_ptr<float[]> fov(new float[1]);
        for (int i = 0; i < 1; i++) {
            fov[i] = cameraFOV;
        }

        params.AddFloat("fov", std::move(fov), 1);

        pbrtCamera("perspective", std::move(params));
    }

    void addFilm() {
        ParamSet params;

        std::unique_ptr<int[]> xres(new int[1]);
        for (int i = 0; i < 1; i++) {
            xres[i] = 700;
        }

        params.AddInt("xresolution", std::move(xres), 1);

        std::unique_ptr<int[]> yres(new int[1]);
        for (int i = 0; i < 1; i++) {
            yres[i] = 700;
        }

        params.AddInt("yresolution", std::move(yres), 1);

        std::unique_ptr<std::string[]> filename(new std::string[1]);
        filename[0] = "result.exr";

        params.AddString("filename", std::move(filename), 1);

        pbrtFilm("image", std::move(params));

        std::unique_ptr<int[]> samples(new int[1]);
        for (int i = 0; i < 1; i++) {
            samples[i] = 8;
        }

        params.AddInt("pixelsamples", std::move(samples), 1);

        pbrtSampler("halton", std::move(params));

        pbrtIntegrator("path", std::move(params));



    }

    void addLight() {
        pbrtAttributeBegin();

        ParamSet params;
        std::unique_ptr<float[]> color_kd(new float[3]);
        for (int i = 0; i < 3; i++) {
            color_kd[i] = 0;
        }
        params.AddFloat("Kd", std::move(color_kd), 3);
        pbrtMaterial("plastic", std::move(params));

        pbrtTranslate(lightPosition[0], lightPosition[1], lightPosition[2]);


        std::unique_ptr<float[]> color(new float[3]);
        for (int i = 0; i < 3; i++) {
            color[i] = 2000;
        }


        params.AddFloat("L", std::move(color), 3);

        std::unique_ptr<int[]> samples(new int[1]);
        for (int i = 0; i < 1; i++) {
            samples[i] = 8;
        }

        params.AddInt("nsamples", std::move(samples), 1);

        pbrtAreaLightSource("area", std::move(params));

        std::unique_ptr<float[]> radius(new float[1]);
        for (int i = 0; i < 1; i++) {
            radius[i] = 3;
        }

        params.AddFloat("radius", std::move(radius), 1);
        pbrtShape("sphere", std::move(params));

        pbrtAttributeEnd();
    }


    void addModel() {
        Model model;
        model.pos[0] = currentPosition[0];
        model.pos[1] = currentPosition[1];
        model.pos[2] = currentPosition[2];
        if (objectIndex == 0) {
            model.type = model_type::SPHERE;
        } else if (objectIndex == 1) {
            model.type = model_type::TRIANGLE;
        } else if (objectIndex == 2) {
            model.type = model_type::KETTLE;
        }

        models.push_back(model);
    }

    void addModels() {
        for (auto &model : models) {
            pbrtAttributeBegin();
            {
                ParamSet params;
                std::unique_ptr<float[]> color_kd(new float[3]);
                color_kd[0] = 0.5; color_kd[1] = 0.5; color_kd[2] = 0.8;
                params.AddFloat("Kd", std::move(color_kd), 3);
                pbrtMaterial("plastic", std::move(params));
            }

            {
                ParamSet params;
                pbrtTranslate(model.pos[0],
                              model.pos[1],
                              model.pos[2]);
            }

            {
                switch (objectIndex) {
                    case(0): {
                        ParamSet params;
                        std::unique_ptr<float[]> radius(new float[1]);
                        radius[0] = 100;
                        params.AddFloat("radius", std::move(radius), 1);
                        pbrtShape("sphere", std::move(params));
                        break;
                    }
                    case(1): {
                        pbrtParseFile("../scenes/geometry/triangle.pbrt");
                        break;
                    }
                    case(2): {
                        pbrtParseFile("../scenes/geometry/killeroo.pbrt");
                        break;
                    }
                    default:
                        break;
                }

            }

            pbrtAttributeEnd();

        }

    }

    void loadTexture() {
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;


        texture.width = g_resolution.x;
        texture.height = g_resolution.y;
        size_t texSize = g_resolution.x * g_resolution.y * 4;

        // We prefer using staging to copy the texture data to a device local optimal image
        VkBool32 useStaging = true;

        // Only use linear tiling if forced
        bool forceLinearTiling = false;
        if (forceLinearTiling) {
            // Don't use linear if format is not supported for (linear) shader sampling
            // Get device properties for the requested texture format
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
            useStaging = !(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
        }

        VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
        VkMemoryRequirements memReqs = {};

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;

        VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo();
        bufferCreateInfo.size = texSize;
        // This buffer is used as a transfer source for the buffer copy
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

        // Get memory requirements for the staging buffer (alignment, memory type bits)
        vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        // Get memory type index for a host visible buffer
        memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
        VK_CHECK_RESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

        // Copy texture data into host local staging buffer
        uint8_t *data;
        VK_CHECK_RESULT(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
        memcpy(data, g_imageBufferVK, texSize);
        vkUnmapMemory(device, stagingMemory);

        // Setup buffer copy regions for each mip level
        std::vector<VkBufferImageCopy> bufferCopyRegions;

        uint32_t offset = 0;

        for (uint32_t i = 0; i < 1; i++) {
            // Calculate offset into staging buffer for the current mip level
            size_t offset = 0;
            // Setup a buffer image copy structure for the current mip level
            VkBufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bufferCopyRegion.imageSubresource.mipLevel = i;
            bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = g_resolution.x;
            bufferCopyRegion.imageExtent.height = g_resolution.y;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = offset;
            bufferCopyRegions.push_back(bufferCopyRegion);
        }

        // Create optimal tiled target image on the device
        VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // Set initial layout of the image to undefined
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.extent = { texture.width, texture.height, 1 };
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VK_CHECK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &texture.image));

        vkGetImageMemoryRequirements(device, texture.image, &memReqs);
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &texture.deviceMemory));
        VK_CHECK_RESULT(vkBindImageMemory(device, texture.image, texture.deviceMemory, 0));

        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        // Image memory barriers for the texture image

        // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
        VkImageSubresourceRange subresourceRange = {};
        // Image only contains color data
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // Start at first mip level
        subresourceRange.baseMipLevel = 0;
        // We will transition on all mip levels
        subresourceRange.levelCount = 1;
        // The 2D texture only has one layer
        subresourceRange.layerCount = 1;

        // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
        VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();;
        imageMemoryBarrier.image = texture.image;
        imageMemoryBarrier.subresourceRange = subresourceRange;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
        // Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
        vkCmdPipelineBarrier(
                copyCmd,
                VK_PIPELINE_STAGE_HOST_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);

        // Copy mip levels from staging buffer
        vkCmdCopyBufferToImage(
                copyCmd,
                stagingBuffer,
                texture.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(bufferCopyRegions.size()),
                bufferCopyRegions.data());

        // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
        // Source pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
        // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        vkCmdPipelineBarrier(
                copyCmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &imageMemoryBarrier);

        // Store current layout for later reuse
        texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

        // Clean up staging resources
        vkFreeMemory(device, stagingMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);

        VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
        sampler.magFilter = VK_FILTER_LINEAR;
        sampler.minFilter = VK_FILTER_LINEAR;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        // Set max level-of-detail to mip level count of the texture
        sampler.maxLod = (useStaging) ? 1.0f : 0.0f;
        // Enable anisotropic filtering
        // This feature is optional, so we must check if it's supported on the device
        if (vulkanDevice->features.samplerAnisotropy) {
            // Use max. level of anisotropy for this example
            sampler.maxAnisotropy = vulkanDevice->properties.limits.maxSamplerAnisotropy;
            sampler.anisotropyEnable = VK_TRUE;
        } else {
            // The device does not support anisotropic filtering
            sampler.maxAnisotropy = 1.0;
            sampler.anisotropyEnable = VK_FALSE;
        }
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(device, &sampler, nullptr, &texture.sampler));

        // Create image view
        // Textures are not directly accessed by the shaders and
        // are abstracted by image views containing additional
        // information and sub resource ranges
        VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = format;
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
        // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        // Linear tiling usually won't support mip maps
        // Only set mip map count if optimal tiling is used
        view.subresourceRange.levelCount = 1;
        // The view will be based on the texture's image
        view.image = texture.image;
        VK_CHECK_RESULT(vkCreateImageView(device, &view, nullptr, &texture.view));
    }

    void destroyTextureImage(Texture texture) {
        vkDestroyImageView(device, texture.view, nullptr);
        vkDestroyImage(device, texture.image, nullptr);
        vkDestroySampler(device, texture.sampler, nullptr);
    }

    void prepareUniformBuffers()
    {
        // Vertex shader uniform buffer block
        VK_CHECK_RESULT(vulkanDevice->createBuffer(
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &uniformBufferVS,
                sizeof(uboVS),
                &uboVS));
        VK_CHECK_RESULT(uniformBufferVS.map());

        updateUniformBuffers();
    }

    void updateUniformBuffers()
    {
        uboVS.projection = camera.matrices.perspective;
        uboVS.modelView = camera.matrices.view;
        uboVS.viewPos = camera.viewPos;
        memcpy(uniformBufferVS.mapped, &uboVS, sizeof(uboVS));
    }

    void buildCommandBuffers() {
        VkCommandBufferBeginInfo cmdBufferInfo =
                vks::initializers::commandBufferBeginInfo();
        VkClearValue clearValues[2];
        clearValues[0].color = defaultClearColor;
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo  renderPassBeginInfo =
                vks::initializers::renderPassBeginInfo();
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
        {
            // Set target frame buffer
            renderPassBeginInfo.framebuffer = frameBuffers[i];

            VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufferInfo));

            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

            VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(drawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(drawCmdBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(drawCmdBuffers[i], indexCount, 1, 0, 0, 0);

            drawUI(drawCmdBuffers[i]);

            vkCmdEndRenderPass(drawCmdBuffers[i]);

            VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }


    }

    void draw() {
        VulkanExampleBase::prepareFrame();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

        VulkanExampleBase::submitFrame();

    }

    void generateQuad() {
        std::vector<Vertex> vertices =
                {
                        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
                        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
                        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
                        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
                };

        std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0};
        indexCount = static_cast<uint32_t>(indices.size());

        VK_CHECK_RESULT(vulkanDevice->createBuffer(
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &vertexBuffer,
                vertices.size() * sizeof(Vertex),
                vertices.data()));
        // Index buffer
        VK_CHECK_RESULT(vulkanDevice->createBuffer(
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &indexBuffer,
                indices.size() * sizeof(uint32_t),
                indices.data()));
    }

    void setupVertexDescriptions() {

        vertices.bindingDescriptions.resize(1);
        vertices.bindingDescriptions[0] =
                vks::initializers::vertexInputBindingDescription(
                        VERTEX_BUFFER_BIND_ID,
                        sizeof(Vertex),
                        VK_VERTEX_INPUT_RATE_VERTEX);

        // Attribute descriptions
        // Describes memory layout and shader positions
        vertices.attributeDescriptions.resize(3);
        // Location 0 : Position
        vertices.attributeDescriptions[0] =
                vks::initializers::vertexInputAttributeDescription(
                        VERTEX_BUFFER_BIND_ID,
                        0,
                        VK_FORMAT_R32G32B32_SFLOAT,
                        offsetof(Vertex, pos));
        // Location 1 : Texture coordinates
        vertices.attributeDescriptions[1] =
                vks::initializers::vertexInputAttributeDescription(
                        VERTEX_BUFFER_BIND_ID,
                        1,
                        VK_FORMAT_R32G32_SFLOAT,
                        offsetof(Vertex, uv));
        // Location 1 : Vertex normal
        vertices.attributeDescriptions[2] =
                vks::initializers::vertexInputAttributeDescription(
                        VERTEX_BUFFER_BIND_ID,
                        2,
                        VK_FORMAT_R32G32B32_SFLOAT,
                        offsetof(Vertex, normal));

        vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
        vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
        vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
        vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
        vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();

    }

    void setupDescriptorPool() {
        std::vector<VkDescriptorPoolSize> poolSizes =
                {
                        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
                        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
                };

        VkDescriptorPoolCreateInfo descriptorPoolInfo =
                vks::initializers::descriptorPoolCreateInfo(
                        static_cast<uint32_t>(poolSizes.size()),
                        poolSizes.data(),
                        2);

        VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

    }

    void setupDescriptorSetLayout() {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
                {
                        // Binding 0 : Vertex shader uniform buffer
                        vks::initializers::descriptorSetLayoutBinding(
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VK_SHADER_STAGE_VERTEX_BIT,
                                0),
                        // Binding 1 : Fragment shader image sampler
                        vks::initializers::descriptorSetLayoutBinding(
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                VK_SHADER_STAGE_FRAGMENT_BIT,
                                1)
                };

        VkDescriptorSetLayoutCreateInfo descriptorLayout =
                vks::initializers::descriptorSetLayoutCreateInfo(
                        setLayoutBindings.data(),
                        static_cast<uint32_t>(setLayoutBindings.size()));

        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
                vks::initializers::pipelineLayoutCreateInfo(
                        &descriptorSetLayout,
                        1);

        VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));

    }

    void setupDescriptorSet() {
        VkDescriptorSetAllocateInfo allocInfo =
                vks::initializers::descriptorSetAllocateInfo(
                        descriptorPool,
                        &descriptorSetLayout,
                        1);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

        // Setup a descriptor image info for the current texture to be used as a combined image sampler
        VkDescriptorImageInfo textureDescriptor;
        textureDescriptor.imageView = texture.view;				// The image's view (images are never directly accessed by the shader, but rather through views defining subresources)
        textureDescriptor.sampler = texture.sampler;			// The sampler (Telling the pipeline how to sample the texture, including repeat, border, etc.)
        textureDescriptor.imageLayout = texture.imageLayout;	// The current layout of the image (Note: Should always fit the actual use, e.g. shader read)

        std::vector<VkWriteDescriptorSet> writeDescriptorSets =
                {
                        // Binding 0 : Vertex shader uniform buffer
                        vks::initializers::writeDescriptorSet(
                                descriptorSet,
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                0,
                                &uniformBufferVS.descriptor),
                        // Binding 1 : Fragment shader texture sampler
                        //	Fragment shader: layout (binding = 1) uniform sampler2D samplerColor;
                        vks::initializers::writeDescriptorSet(
                                descriptorSet,
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// The descriptor set will use a combined image sampler (sampler and image could be split)
                                1,												// Shader binding point 1
                                &textureDescriptor)								// Pointer to the descriptor image for our texture
                };

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

    }

    void preparePipelines() {

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
                vks::initializers::pipelineInputAssemblyStateCreateInfo(
                        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                        0,
                        VK_FALSE);

        VkPipelineRasterizationStateCreateInfo rasterizationState =
                vks::initializers::pipelineRasterizationStateCreateInfo(
                        VK_POLYGON_MODE_FILL,
                        VK_CULL_MODE_NONE,
                        VK_FRONT_FACE_COUNTER_CLOCKWISE,
                        0);

        VkPipelineColorBlendAttachmentState blendAttachmentState =
                vks::initializers::pipelineColorBlendAttachmentState(
                        0xf,
                        VK_FALSE);

        VkPipelineColorBlendStateCreateInfo colorBlendState =
                vks::initializers::pipelineColorBlendStateCreateInfo(
                        1,
                        &blendAttachmentState);

        VkPipelineDepthStencilStateCreateInfo depthStencilState =
                vks::initializers::pipelineDepthStencilStateCreateInfo(
                        VK_TRUE,
                        VK_TRUE,
                        VK_COMPARE_OP_LESS_OR_EQUAL);

        VkPipelineViewportStateCreateInfo viewportState =
                vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

        VkPipelineMultisampleStateCreateInfo multisampleState =
                vks::initializers::pipelineMultisampleStateCreateInfo(
                        VK_SAMPLE_COUNT_1_BIT,
                        0);

        std::vector<VkDynamicState> dynamicStateEnables = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState =
                vks::initializers::pipelineDynamicStateCreateInfo(
                        dynamicStateEnables.data(),
                        static_cast<uint32_t>(dynamicStateEnables.size()),
                        0);

        // Load shaders
        std::array<VkPipelineShaderStageCreateInfo,2> shaderStages;

        shaderStages[0] = loadShader(getShadersPath() + "rt.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = loadShader(getShadersPath() + "rt.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        VkGraphicsPipelineCreateInfo pipelineCreateInfo =
                vks::initializers::pipelineCreateInfo(
                        pipelineLayout,
                        renderPass,
                        0);

        pipelineCreateInfo.pVertexInputState = &vertices.inputState;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid));
    }

    void prepare() {
        VulkanExampleBase::prepare();
        loadTexture();
        generateQuad();
        setupVertexDescriptions();
        prepareUniformBuffers();
        setupDescriptorSetLayout();
        preparePipelines();
        setupDescriptorPool();
        setupDescriptorSet();
        buildCommandBuffers();
        prepared = true;

    }

    virtual void render() {
        if (!prepared)
            return;
        draw();

    }

    virtual void OnUpdateUIOverlay(vks::UIOverlay *overlay) {
        if (overlay->header("Загрузить сцену из файла")) {
            if (overlay->comboBox("Выбрать объект", &sceneIndex, scenesAvailable)) {

            }
            if (overlay->button("Сгенерировать изображение")) {
                pbrtParseFile(scenesPaths[sceneIndex]);
                pbrtGetImageBuffer(&g_imageBufferVK, g_resolution);
                loadTexture();
                generateQuad();
                setupVertexDescriptions();
                prepareUniformBuffers();
                setupDescriptorSetLayout();
                preparePipelines();
                setupDescriptorPool();
                setupDescriptorSet();
                buildCommandBuffers();
            }
        }

        if (overlay->header("Задать сцену самостоятельно")) {
            if (overlay->comboBox("Выбрать объект", &objectIndex, objectsAvailable)) {

            }

            overlay->text("Положение объекта");

            if (overlay->sliderFloat("mx", &currentPosition[0], -1000, 1000)) {
            }

            if (overlay->sliderFloat("my", &currentPosition[1], -1000, 1000)) {
            }
            if (overlay->sliderFloat("mz", &currentPosition[2], -1000, 1000)) {
            }

            if (overlay->button("Добавить объект")) {
                addModel();
            }

            overlay->text("Положение камеры");

            if (overlay->sliderFloat("cx", &currentPosition[0], -1000, 1000)) {
            }

            if (overlay->sliderFloat("cy", &currentPosition[1], -1000, 1000)) {
            }
            if (overlay->sliderFloat("cz", &currentPosition[2], -1000, 1000)) {
            }

            overlay->text("Направление камеры");

            if (overlay->sliderFloat("dx", &cameraPosition[0], -1000, 1000)) {
            }

            if (overlay->sliderFloat("dy", &cameraPosition[1], -1000, 1000)) {
            }
            if (overlay->sliderFloat("dz", &cameraPosition[2], -1000, 1000)) {
            }


            if (overlay->sliderFloat("Угол обзора камеры", &cameraFOV, 0, 180)) {
                std::cout << cameraFOV << std::endl;
            }

            overlay->text("Положение освещения");

            if (overlay->sliderFloat("lx", &cameraPosition[0], -1000, 1000)) {
            }

            if (overlay->sliderFloat("ly", &cameraPosition[1], -1000, 1000)) {
            }
            if (overlay->sliderFloat("lz", &cameraPosition[2], -1000, 1000)) {
            }

            if (overlay->comboBox("Выбрать ускоряющую структуру",
                                  &accelStrucIndex, accelStructsAvailable)){

                buildCommandBuffers();
            }

            if (overlay->button("Построить заданную сцену")) {
                manualSceneLoad();
                pbrtGetImageBuffer(&g_imageBufferVK, g_resolution);
                loadTexture();
                models.clear();
                generateQuad();
                setupVertexDescriptions();
                prepareUniformBuffers();
                setupDescriptorSetLayout();
                preparePipelines();
                setupDescriptorPool();
                setupDescriptorSet();
                buildCommandBuffers();
            }
        }
    }
};

VULKAN_EXAMPLE_MAIN()
