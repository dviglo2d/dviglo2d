// Copyright (c) the Dviglo project
// License: MIT

#pragma once

#include "main_args.hpp"
#include "os_window.hpp"
#include "timer.hpp"

#include "../audio/audio.hpp"
#include "../gl_utils/shader_cache_old.hpp"
#include "../gl_utils/texture_cache.hpp"
#include "../res/freetype.hpp"

#include <dv_log.hpp>
#include <glad/gl.h>
#include <SDL3/SDL.h>

#include <memory>

#include "../vulkan/vulkan_utils.hpp"


namespace dviglo
{

class ApplicationBase : public SubsystemIndex
{
private:
    // Вызван ли уже метод on_frame_begin()
    bool is_on_frame_begin_called = false;

#ifdef DV_CTEST
    // Через сколько секунд после запуска приложение автоматически закроется.
    // При значении 0 закрываться не будет.
    // Задаётся с помощью параметра -duration x
    i64 duration_ = 0;
#endif

protected:
    // Пользователь желает прервать главный цикл
    bool should_exit_ = false;

    ApplicationBase()
    {
        const std::vector<StrUtf8>& args = DV_MAIN_ARGS->get();

        // Нулевой аргумент - запускаемый файл, поэтому пропускаем
        for (size_t i = 1; i < args.size(); ++i)
        {
            const StrUtf8& arg = args[i];

            if (arg == "-duration")
            {
                // Получаем следующий аргумент, если есть
                if (i + 1 < args.size())
                {
                    ++i;
#ifdef DV_CTEST
                    duration_ = to_u64(args[i]);
#endif
                }
            }
        }
    }

    // Метод вызывается перед обработчиками событий
    virtual void on_frame_begin() {}

    // Обработчики событий вызываются перед update()
    virtual void handle_sdl_event(const SDL_Event& event)
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            should_exit_ = true;
            return;

        case SDL_EVENT_WINDOW_RESIZED:
            {
                i32 width = event.window.data1;
                i32 height = event.window.data2;
                glViewport(0, 0, width, height);
                return;
            }
        }
    }

    virtual void update(i64 ns) { (void)ns; }
    virtual void draw() {}
    virtual void on_frame_end() {}

public:
    void exit() { should_exit_ = true; }

    // Методы ниже должны быть публичными, чтобы SDL мог их вызвать.
    // Пользователь не должен их вызывать

    SDL_AppResult main_event(const SDL_Event* event)
    {
        if (!is_on_frame_begin_called)
        {
            on_frame_begin();
            is_on_frame_begin_called = true;
        }

        handle_sdl_event(*event);

        if (should_exit_)
            return SDL_APP_SUCCESS;
        else
            return SDL_APP_CONTINUE;
    }

    SDL_AppResult main_iterate()
    {
#ifdef DV_CTEST
        // При CTest выходим через duration_ секунд после запуска приложения
        if (duration_ && get_ticks_ns() >= s_to_ns(duration_))
            should_exit_ = true;
#endif

        static vk::UniqueSemaphore     image_available_sem_;

        static vk::UniqueFence     in_flight_fence_; // <--- Добавил Забор
        static vk::UniqueCommandPool command_pool;
        static vk::UniqueCommandBuffer command_buffer;

        static vk::UniqueFence     acquire_fence;

        // [НОВОЕ]: Семафор, который скажет функции present(), что треугольник нарисован
        static vk::UniqueSemaphore     render_finished_sem_;

        static std::vector<vk::Image> swapchain_images;

        vk::Result result;

        if (!image_available_sem_)
        {
            vk::SemaphoreCreateInfo sem_info{ .sType = vk::StructureType::eSemaphoreCreateInfo };
            std::tie(result, image_available_sem_) = DV_OS_WINDOW->vk_device_->createSemaphoreUnique(sem_info).asTuple();
        }

        // Создаем семафор окончания рендера один раз
        if (!render_finished_sem_)
        {
            std::tie(result, render_finished_sem_) = DV_OS_WINDOW->vk_device_->createSemaphoreUnique({}).asTuple();
        }


        if (!in_flight_fence_)
        {
            vk::FenceCreateInfo fence_info{};
            // Без флага eSignaled, так как мы сначала делаем Reset+Submit, потом Wait
            std::tie(result, in_flight_fence_) = DV_OS_WINDOW->vk_device_->createFenceUnique(fence_info).asTuple();
            if (result != vk::Result::eSuccess) Log::writef_error("Failed to create in_flight_fence: {}", vk::to_string(result));
        }

        if (!acquire_fence)
        {
            vk::FenceCreateInfo fence_info{};
            // Без флага eSignaled, так как мы сначала делаем Reset+Submit, потом Wait
            std::tie(result, acquire_fence) = DV_OS_WINDOW->vk_device_->createFenceUnique(fence_info).asTuple();
            if (result != vk::Result::eSuccess) Log::writef_error("Failed to create acquire_fence: {}", vk::to_string(result));
        }

        if (!command_pool)
        {
            vk::CommandPoolCreateInfo pool_info{ .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = DV_OS_WINDOW->vk_graphics_queue_family_index_ };
            std::tie(result, command_pool) = DV_OS_WINDOW->vk_device_->createCommandPoolUnique(pool_info).asTuple();

            vk::CommandBufferAllocateInfo alloc_info{ .commandPool = *command_pool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
            std::vector<vk::UniqueCommandBuffer> bufs;
            std::tie(result, bufs) = DV_OS_WINDOW->vk_device_->allocateCommandBuffersUnique(alloc_info).asTuple();
            command_buffer = std::move(bufs[0]);

            std::tie(result, swapchain_images) = DV_OS_WINDOW->vk_device_->getSwapchainImagesKHR(DV_OS_WINDOW->swapchain_->get()).asTuple();
        }

        DV_OS_WINDOW->vk_device_->resetFences(1, &acquire_fence.get());


        // Запрашиваем картинку. Заблокирует поток при vk::PresentModeKHR::eFifo
        auto [res, idx] = DV_OS_WINDOW->swapchain_->acquire_next_image(*DV_OS_WINDOW->vk_device_, *acquire_fence);


        vk::Result acquire_result = res;

        if (acquire_result == vk::Result::eErrorOutOfDateKHR)
        {
            // Свопчейн сломан. Чиним его прямо здесь.
            //recreate_swapchain(swapchain_->present_mode());

            return SDL_APP_CONTINUE; // Ждем новый кадр
        }

        // То, что acquire_next_image вернул картинку, ещё не значит, что presentation engine уже закончил из неё читать, поэтому ждём
        DV_OS_WINDOW->vk_device_->waitForFences(1, &acquire_fence.get(), vk::True, UINT64_MAX);


        // =====================================================================
                // ШАГ 1: Запрашиваем пустую картинку. 
                // ВАЖНО: Именно здесь драйвер заблокирует CPU, если мы уперлись в лимит 
                // монитора (например, в режиме FIFO), чтобы мы не генерировали кадры вхолостую.
                // =====================================================================
        /*

            if (acquire_result == vk::Result::eErrorOutOfDateKHR)
            {
                // Свопчейн сломан. Чиним его прямо здесь.
                recreate_swapchain(swapchain_->present_mode());

                continue // Ждем новый калр
            }

            или сразу запрашиваем нвоый кадр после пересоздания

        do 
        {
            auto [res, idx] = swapchain_->acquire_next_image(device_, *image_available_sem_);
            acquire_result = res;
            image_index = idx;

            if (acquire_result == vk::Result::eErrorOutOfDateKHR)
            {
                // Свопчейн сломан. Чиним его прямо здесь.
                recreate_swapchain(swapchain_->present_mode());
                
                // ВАЖНО: Мы не выходим из цикла do-while, а идем на следующий виток,
                // чтобы тут же попробовать взять картинку из нового свопчейна.
            }
            // eSuboptimalKHR нас устраивает, мы продолжаем рисовать
            
        } while (acquire_result == vk::Result::eErrorOutOfDateKHR);

        // пересоздаём свопчейн и снвоа запрашиваем кадр

        но тут будет вечный цикл при сверонутом окне, так как он создаёт свопчей нулевого размера

        поэтому

        void Renderer::recreate_swapchain(...)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    // Если окно свернуто (размер 0) — ставим процессор на паузу
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents(); // Блокирует поток, пока не придет событие от ОС
    }

    // Только когда ширина > 0, мы создаем новый свопчейн
    device_.waitIdle();
    swapchain_ = Swapchain::construct(...);
}

        */

        static i64 old_ticks_ns = get_ticks_ns(); // Время начала прошлого кадра
        i64 new_ticks_ns = get_ticks_ns(); // Время начала кадра
        i64 ns = new_ticks_ns - old_ticks_ns;
        assert(ns >= 0);
        old_ticks_ns = new_ticks_ns;

        // Если точности get_ticks_ns() не хватает
        if (ns == 0)
        {
            Log::write_info("ns == 0");

            // Ждём полмиллисекунды
            delay_ns(ns_per_ms / 2);
            //return SDL_APP_CONTINUE; // Нельзя выходить, так как захватили изображение, его надо отпустить в present
            new_ticks_ns = get_ticks_ns();
            ns = new_ticks_ns - old_ticks_ns;

        }

        if (!is_on_frame_begin_called)
        {
            on_frame_begin();
            is_on_frame_begin_called = true;
        }

        update(ns);
        draw();

#if 0 /// TODO: Очистка делается в рендерпасе
        // <--- ДОБАВЛЕНО: Запись команд очистки экрана
        command_buffer->reset();
        vk::CommandBufferBeginInfo begin_info{};
        command_buffer->begin(begin_info);

        vk::Image target_image = swapchain_images[idx];

        // Создаем range вручную
        vk::ImageSubresourceRange range;
        range.aspectMask = vk::ImageAspectFlagBits::eColor;
        range.baseMipLevel = 0;
        range.levelCount = 1;
        range.baseArrayLayer = 0;
        range.layerCount = 1;

        // 1. БАРЬЕР: Undefined -> TransferDst (заполняем поля вручную)
        vk::ImageMemoryBarrier barrier1;
        barrier1.srcAccessMask = {}; // 0
        barrier1.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier1.oldLayout = vk::ImageLayout::eUndefined;
        barrier1.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier1.image = target_image;
        barrier1.subresourceRange = range;

        // Передаем barrier1 как объект. nullptr для пустых массивов.
        command_buffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eTransfer,
            {},
            nullptr, // memoryBarriers
            nullptr, // bufferBarriers
            barrier1 // imageBarriers
        );

        // 2. ОЧИСТКА В ЧЕРНЫЙ
        vk::ClearColorValue clear_color = {};
        // Заполняем массив вручную, так как конструкторов нет
        clear_color.float32[0] = 1.0f;
        clear_color.float32[1] = 0.0f;
        clear_color.float32[2] = 0.0f;
        clear_color.float32[3] = 1.0f; // Альфа

        command_buffer->clearColorImage(target_image, vk::ImageLayout::eTransferDstOptimal, clear_color, range);

        // 3. БАРЬЕР: TransferDst -> PresentSrc
        vk::ImageMemoryBarrier barrier2;
        barrier2.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier2.dstAccessMask = {}; // 0
        barrier2.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier2.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier2.image = target_image;
        barrier2.subresourceRange = range;

        command_buffer->pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe,
            {},
            nullptr,
            nullptr,
            barrier2
        );

        command_buffer->end();

#endif

        // ----------------------------------------------------

        // Используем только 1 кадр в полете, не начимнаем новый кадр, если старый еще не отобразился
        // для минимизаии инпут лага (синхронный рендеринг).
        // Сперва ждем прцоессор, потом ждем видеокарту (прцоессор простаивает), но зато инпут лаг минимальный
        // но фпс ниже

        /*

    // =====================================================================
        // ШАГ 3: Записываем команды рендера для полученной картинки
        // =====================================================================
        record_command_buffer(image_index);

        // =====================================================================
        // ШАГ 4: Отправляем команды на видеокарту
        // =====================================================================
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::SubmitInfo submit_info = {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &(*image_available_sem_), // Ждем, пока свопчейн реально отдаст картинку
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &(*command_buffer_),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &(*render_finished_sem_) // Зажжется, когда GPU закончит
        };

        // Очищаем наш единственный забор перед отправкой
        device_.resetFences(1, &(*render_fence_));
        graphics_queue_.submit(submit_info, *render_fence_);

        // =====================================================================
        // ШАГ 5: БЛОКИРОВКА ПРОЦЕССОРА (Суть 1 кадра в полете)
        // Процессор засыпает и ждет, пока видеокарта (GPU) не выполнит ВСЕ команды.
        // Это гарантирует, что на следующем витке цикла мы безопасно перезапишем command_buffer_.
        // =====================================================================
        device_.waitForFences(1, &(*render_fence_), VK_TRUE, std::numeric_limits<uint64_t>::max());

        // =====================================================================
        // ШАГ 6: Отдаем готовую картинку контроллеру монитора
        // =====================================================================
        vk::Result present_result = swapchain_->present(present_queue_, image_index, *render_finished_sem_);

        if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR)
        {
            recreate_swapchain(swapchain_->present_mode());
        }

        */

        // Сбрасываем забор перед использованием
// (исправлено на твой стиль доступа к device)




       // DV_OS_WINDOW->vk_device_->resetFences(1, &(*in_flight_fence_));

#if 0 // Отправка оистки не нужна

        //vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eTransfer };

        vk::SubmitInfo submit_info = {
            //.waitSemaphoreCount = 1,
            //.pWaitSemaphores = &(*image_available_sem_), // Ждем, когда картинка придет
            //.pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,                     // 0 команд - это законно
            .pCommandBuffers = &(*command_buffer),
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr  // Сигналим, что "рендер" (пустой) окончен
        };

        //DV_OS_WINDOW->vk_graphics_queue_.submit(submit_info, *in_flight_fence_);
#endif


        // =================================================================== Была очистка, а теперь рисуем

#if 1

        {
            // TODO: Тест рендеринга треугольника

            vk::Result vk_result;

            // -------------------------------- Загружаем шейдеры из файлов

            fs::path base_path = get_base_path();
            vk::UniqueShaderModule vert_shader_module = load_shader(DV_OS_WINDOW->vk_device_.get(), base_path / "engine_data/shaders/simple.vert.spv");

            if (!vert_shader_module)
            {
                Log::writef_error("{} | !vert_shader_module", DV_FUNC_SIG);
                //return;
            }

            vk::UniqueShaderModule frag_shader_module = load_shader(DV_OS_WINDOW->vk_device_.get(), base_path / "engine_data/shaders/simple.frag.spv");

            if (!frag_shader_module)
            {
                Log::writef_error("{} | !frag_shader_module", DV_FUNC_SIG);
                //return;
            }

            // -------------------------------- Графический конвеер

            // -------------------------------- Шейдерные стадии

            vk::PipelineShaderStageCreateInfo pipeline_vert_shader_stage_create_info
            {
                .stage = vk::ShaderStageFlagBits::eVertex,
                .module = *vert_shader_module,
                .pName = "main",
            };

            vk::PipelineShaderStageCreateInfo pipeline_frag_shader_stage_create_info
            {
                .stage = vk::ShaderStageFlagBits::eFragment,
                .module = *frag_shader_module,
                .pName = "main",
            };

            vk::PipelineShaderStageCreateInfo shader_stages[] = { pipeline_vert_shader_stage_create_info, pipeline_frag_shader_stage_create_info };

            // -------------------------------- Стадия сборки примитивов из вершин

            vk::PipelineVertexInputStateCreateInfo vertex_input_info; // Все по дефолту и обнулено

            // -------------------------------- Стадия сборки примитивов из вершин

            vk::PipelineInputAssemblyStateCreateInfo assemply_info
            {
                .topology = vk::PrimitiveTopology::eTriangleList,
            };

            // -------------------------------- Вьюпорт

            vk::Viewport viewport
            {
                .x = 0.f,
                .y = 0.f,
                .width = 512.f,
                .height = 512.f,
                .minDepth = 0.f,
                .maxDepth = 1.f,
            };

            vk::Rect2D scissor
            {
                .offset = {0, 0},
                .extent = {512, 512},
            };

            vk::PipelineViewportStateCreateInfo viewport_state
            {
                .viewportCount = 1,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor,
            };

            // -------------------------------- Растеризатор

            vk::PipelineRasterizationStateCreateInfo rasterizer
            {
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eBack,
                .frontFace = vk::FrontFace::eClockwise,
                .lineWidth = 1.f,
            };

            // -------------------------------- Мультисэмплинг

            vk::PipelineMultisampleStateCreateInfo multisampling
            {
                .rasterizationSamples = vk::SampleCountFlagBits::e1,
                .minSampleShading = 1.f,
            };

            // -------------------------------- Смешивание цветов в цветовом включении

            vk::PipelineColorBlendAttachmentState color_blend_attachment
            {
                .blendEnable = false,
                .srcColorBlendFactor = vk::BlendFactor::eOne,
                .dstColorBlendFactor = vk::BlendFactor::eZero,
                .colorBlendOp = vk::BlendOp::eAdd,
                .srcAlphaBlendFactor = vk::BlendFactor::eOne,
                .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                .alphaBlendOp = vk::BlendOp::eAdd,
                .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
            };

            vk::PipelineColorBlendStateCreateInfo color_blending
            {
                .attachmentCount = 1,
                .pAttachments = &color_blend_attachment,
            };

            // -------------------------------- Макет пайплана

            vk::PipelineLayoutCreateInfo pipeline_layout_info; // Всё пусто

            vk::UniquePipelineLayout vk_graphics_pipeline_layout;
            std::tie(vk_result, vk_graphics_pipeline_layout) = DV_OS_WINDOW->vk_device_->createPipelineLayoutUnique(pipeline_layout_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vk_device_->createPipelineLayoutUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
               // return;
            }

            // -------------------------------- Пайплайн

            vk::Format offscreen_format = vk::Format::eB8G8R8A8Unorm;
            vk::PipelineRenderingCreateInfo pipeline_rendering_info
            {
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &offscreen_format,
            };

            vk::GraphicsPipelineCreateInfo graphics_pipeline_info
            {
                .pNext = &pipeline_rendering_info,
                .stageCount = 2,
                .pStages = shader_stages,
                .pVertexInputState = &vertex_input_info,
                .pInputAssemblyState = &assemply_info,
                .pViewportState = &viewport_state,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pColorBlendState = &color_blending,
                .layout = *vk_graphics_pipeline_layout,
            };

            vk::UniquePipeline vk_graphics_pipeline;
            std::tie(vk_result, vk_graphics_pipeline) = DV_OS_WINDOW->vk_device_->createGraphicsPipelineUnique(nullptr, graphics_pipeline_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vk_device_->createGraphicsPipelineUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
               // return;
            }

            // -------------------------------- Рендерим

            // -------------------------------- Выделяем командный буфер из пула

            vk::CommandBufferAllocateInfo cb_alloc_info
            {
                .commandPool = DV_OS_WINDOW->vk_command_pool_.get(),
                .commandBufferCount = 1,
            };

            std::vector<vk::UniqueCommandBuffer> command_buffers;
            std::tie(vk_result, command_buffers) = DV_OS_WINDOW->vk_device_->allocateCommandBuffersUnique(cb_alloc_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vk_device_->allocateCommandBuffersUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
               // return;
            }

            vk::CommandBuffer command_buffer = *(command_buffers[0]);

            // -------------------------------- Пишем в коммандный буфер

            vk::CommandBufferBeginInfo command_buffer_begin_info;

            vk_result = command_buffer.begin(command_buffer_begin_info);

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | command_buffer->begin(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                //return;
            }

            // В Vulkan 1.4 без RenderPass мы сами должны подготовить картинку к рендеру.
            // Переводим её из eUndefined в eColorAttachmentOptimal.
            // =========================================================================
            vk::ImageMemoryBarrier2 barrier_to_attach
            {
                .srcStageMask = vk::PipelineStageFlagBits2::eNone,
                .srcAccessMask = vk::AccessFlagBits2::eNone,
                .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .image = DV_OS_WINDOW->swapchain_->offscreen_image_.image(),
                .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
            };
            vk::DependencyInfo dep_info_attach{ .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier_to_attach };
            command_buffer.pipelineBarrier2(dep_info_attach);


            /*
            vk::ClearValue clear_color
            {
                .color
                {
                    .float32 = {{0.25f, 0.15f, 0.7f, 1.0f}}
                }
            };

            vk::RenderPassBeginInfo rp_info
            {
                .renderPass = DV_OS_WINDOW->swapchain_->offscreen_image_.render_pass.get(),
                .framebuffer = DV_OS_WINDOW->swapchain_->offscreen_framebuffer(),
                .renderArea = {.offset = {0, 0}, .extent = DV_OS_WINDOW->swapchain_->offscreen_image_extent()},
                .clearValueCount = 1,
                .pClearValues = &clear_color,
            };

            command_buffer.beginRenderPass(rp_info, vk::SubpassContents::eInline);
            */

            vk::RenderingAttachmentInfo color_attachment
            {
                .imageView = DV_OS_WINDOW->swapchain_->offscreen_image_.view.get(),
                .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .clearValue = { vk::ClearColorValue{ std::array<float, 4>{0.25f, 0.15f, 0.7f, 1.0f} } }
            };

            vk::RenderingInfo rendering_info
            {
                .renderArea = { {0, 0}, DV_OS_WINDOW->swapchain_->offscreen_image_extent() },
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &color_attachment
            };

            command_buffer.beginRendering(rendering_info);


            command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *vk_graphics_pipeline);
            command_buffer.draw(3, 1, 0, 0);

            //command_buffer.endRenderPass();
            command_buffer.endRendering();


            // Подготавливаем отрендеренную картинку для функции Swapchain::present.
            // Переводим в ShaderReadOnlyOptimal, так как на неё будет накладываться гамма
            vk::ImageMemoryBarrier2 barrier_to_read
            {
                .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
                .dstAccessMask = vk::AccessFlagBits2::eShaderRead,
                .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .image = DV_OS_WINDOW->swapchain_->offscreen_image_.image(),
                .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
            };
            vk::DependencyInfo dep_info_read{ .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier_to_read };
            command_buffer.pipelineBarrier2(dep_info_read);

            vk_result = command_buffer.end();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | command_buffer.end() | {}", DV_FUNC_SIG, vk::to_string(vk_result));
                //return;
            }

            // -------------------------------- Забор

            vk::UniqueFence fence;

            vk::FenceCreateInfo fence_info;

            std::tie(vk_result, fence) = DV_OS_WINDOW->vk_device_->createFenceUnique(fence_info).asTuple();

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vk_device_->createFenceUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
               // return;
            }

            // -------------------------------- Отправляем в очередь

            /*
            vk::SubmitInfo submit_info
            {
                .commandBufferCount = 1,
                .pCommandBuffers = &command_buffer,
            };

            vk_result = DV_OS_WINDOW->vk_graphics_queue_.submit(1, &submit_info, *fence);


            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | compute_queue.submit(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
               // return;
            }

            */

            // Теперь используем submit2 чтобы ждать треугольник на GPU

            vk::CommandBufferSubmitInfo cmd_info{ .commandBuffer = command_buffer };

            vk::SemaphoreSubmitInfo signal_info
            {
                .semaphore = render_finished_sem_.get(),
                .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput
            };

            vk::SubmitInfo2 submit_info_2
            {
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &cmd_info,
                .signalSemaphoreInfoCount = 1,
                .pSignalSemaphoreInfos = &signal_info
            };

            DV_OS_WINDOW->vk_graphics_queue_.submit2(1, &submit_info_2, *fence);

            // -------------------------------- Ждем результат

            vk_result = DV_OS_WINDOW->vk_device_->waitForFences(1, &fence.get(), vk::True, 100000000000);

            if (vk_result != vk::Result::eSuccess)
            {
                Log::writef_error("{} | vk_device_->waitForFences(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
               // return;
            }

            // TODO Передавать фреймбуфер в draw();
        }

#endif









        // 5. ДОБАВИЛ: Ожидание (Синхронный режим)
// Блокируем CPU, пока видеокарта не обработает submit
        //DV_OS_WINDOW->vk_device_->waitForFences(1, &(*in_flight_fence_), VK_TRUE, UINT64_MAX);

       // vk::Result present_result = DV_OS_WINDOW->swapchain_->present(DV_OS_WINDOW->vk_graphics_queue_, DV_OS_WINDOW->vk_present_queue_, idx,
       //     DV_OS_WINDOW->vk_device_.get(), DV_OS_WINDOW->vk_command_pool_.get());

        // TODO: Надо передалть на семафор, чтобы ждало треугольник на GPU
        vk::Result present_result = DV_OS_WINDOW->swapchain_->present(DV_OS_WINDOW->vk_graphics_queue_, DV_OS_WINDOW->vk_present_queue_, idx,
            DV_OS_WINDOW->vk_device_.get(), DV_OS_WINDOW->vk_command_pool_.get(),
            render_finished_sem_.get());

        if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR)
        {
            //recreate_swapchain(swapchain_->present_mode());
        }

        // смена режима через пересозданеи свпочейна
        // recreate_swapchain(vk::PresentModeKHR::eMailbox);

        //DV_OS_WINDOW->swapchain_->present();



        //SDL_GL_SwapWindow(DV_OS_WINDOW->window());

        // Убираем буферизацию и инпут лаг при включённой вертикалке на NVIDIA в Windows.
        // По ощущениям работает лучше, когда стоит после SDL_GL_SwapWindow(...), а не до
        //glFinish();

        //frame_idx = (frame_idx + 1) % 2;

        on_frame_end();
        is_on_frame_begin_called = false;

        if (should_exit_)
            return SDL_APP_SUCCESS;
        else
            return SDL_APP_CONTINUE;
    }
};

} // namespace dviglo
