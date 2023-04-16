#pragma once

#include <dv_string.hpp>
#include <dv_subsystem_base.hpp>
#include <vector>


namespace dviglo
{

// Хранит аргументы командной строки
class MainArgs final : public SubsystemBase<MainArgs>
{
private:
    std::vector<StrUtf8> args_;

public:
    MainArgs(i32 argc, char* argv[]);
    ~MainArgs();

    const std::vector<StrUtf8>& get() const { return args_; }
};

} // namespace dviglo

#define DV_MAIN_ARGS (dviglo::MainArgs::instance())
