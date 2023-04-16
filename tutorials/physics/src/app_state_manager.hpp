#pragma once

#include "app_state_base.hpp"

#include <dv_subsystem_index.hpp>
#include <vector>

using namespace dviglo;
using namespace std;


enum class AppStateId
{
    null = 0, // Ещё ни одно состояние не выбрано
    main_screen,
    spacewar,
    number // Число состояний
};


// Подсистема для переключения состояний
class AppStateManager final : public SubsystemIndex
{
private:
    // Инициализируется в конструкторе
    inline static AppStateManager* instance_ = nullptr;

    AppStateId current_app_state_id_ = AppStateId::null;
    AppStateId previous_app_state_id_ = AppStateId::null;
    AppStateId required_app_state_id_ = AppStateId::main_screen;

    vector<unique_ptr<AppStateBase>> app_states_;

public:
    static AppStateManager* instance() { return instance_; }

    AppStateManager();
    ~AppStateManager() final;

    AppStateId current_app_state_id() const { return current_app_state_id_; }
    AppStateId previous_app_state_id() const { return previous_app_state_id_; }

    AppStateId required_app_state_id() const { return required_app_state_id_; }
    void required_app_state_id(AppStateId id) { required_app_state_id_ = id; }

    unique_ptr<AppStateBase>& current_app_state()
    {
        assert(current_app_state_id_ != AppStateId::null);
        return app_states_[to_underlying(current_app_state_id_)];
    }

    // Меняет текущее состояние, если current_app_state_id_ != required_app_state_id_.
    // Этод метод нужно вызывать в начале кадра перед вызовом любых методов подсистем
    void apply();
};

#define APP_STATE_MANAGER (AppStateManager::instance())
