#pragma once

#include "app_state_base.hpp"

#include <dv_subsystem_base.hpp>
#include <vector>

using namespace dviglo;
using namespace std;


enum AppStateId
{
    app_state_null = 0, // Ещё ни одно состояние не выбрано
    app_state_main_screen,
    app_state_spacewar,
    app_state_number // Число состояний
};


// Подсистема для переключения состояний
class AppStateManager final : public SubsystemBase<AppStateManager>
{
private:
    AppStateId current_app_state_id_ = app_state_null;
    AppStateId previous_app_state_id_ = app_state_null;
    AppStateId required_app_state_id_ = app_state_main_screen;

    vector<unique_ptr<AppStateBase>> app_states_;

public:
    AppStateManager();
    ~AppStateManager();

    AppStateId get_current_app_state_id() const { return current_app_state_id_; }
    AppStateId get_previous_app_state_id() const { return previous_app_state_id_; }

    AppStateId get_required_app_state_id() const { return required_app_state_id_; }
    void set_required_app_state_id(AppStateId id) { required_app_state_id_ = id; }

    unique_ptr<AppStateBase>& get_current_app_state()
    {
        assert(current_app_state_id_ != AppStateId::app_state_null);
        return app_states_[current_app_state_id_];
    }

    // Меняет текущее состояние, если current_app_state_id_ != required_app_state_id_.
    // Этод метод нужно вызывать в начале кадра перед вызовом любых методов подсистем
    void apply();
};

#define APP_STATE_MANAGER (AppStateManager::instance())
