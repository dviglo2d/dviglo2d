#pragma once

#include <dv_string.hpp>
#include <dv_subsystem_index.hpp>
#include <vector>


namespace dviglo
{

// Хранит аргументы командной строки
class MainArgs final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static MainArgs* instance_ = nullptr;

    std::vector<StrUtf8> args_;

public:
    static MainArgs* instance() { return instance_; }

    MainArgs(i32 argc, char* argv[]);
    ~MainArgs() final;

    const std::vector<StrUtf8>& get() const { return args_; }
};

} // namespace dviglo

#define DV_MAIN_ARGS (dviglo::MainArgs::instance())
