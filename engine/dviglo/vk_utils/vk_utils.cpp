// Copyright (c) the Dviglo project
// License: MIT

#include "vk_utils.hpp"

#include <dv_file.hpp>
#include <dv_log.hpp>

using namespace std;


namespace dviglo
{

vk::UniqueShaderModule load_shader(const vk::Device device, const fs::path& path)
{
    vector<u32> shader_code = read_all_data<u32>(path);

    if (shader_code.empty())
    {
        Log::writef_error("{} | shader_code.empty() | {}", DV_FUNC_SIG, path);
        return {};
    }

    vk::ShaderModuleCreateInfo sm_create_info
    {
        .codeSize = shader_code.size() * sizeof(u32),
        .pCode = shader_code.data(),
    };

    auto[vk_result, ret] = device.createShaderModuleUnique(sm_create_info).asTuple();

    if (vk_result != vk::Result::eSuccess)
    {
        Log::writef_error("{} | device.createShaderModuleUnique(...) | {}", DV_FUNC_SIG, vk::to_string(vk_result));
        return {};
    }

    return std::move(ret);
}

} // namespace dviglo
