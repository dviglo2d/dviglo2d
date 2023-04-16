#include "app_state_manager.hpp"

#include "app_state_main_screen.hpp"
#include "app_state_spacewar.hpp"

#include <dv_log.hpp>


AppStateManager::AppStateManager()
{
    assert(!instance_); // Объект должен быть только один
    instance_ = this;

    app_states_.emplace_back(nullptr); // AppStateId::app_state_null
    app_states_.emplace_back(std::make_unique<AppStateMainScreen>());
    app_states_.emplace_back(std::make_unique<AppStateSpacewar>());
}

AppStateManager::~AppStateManager()
{
    instance_ = nullptr;
}

void AppStateManager::apply()
{
    if (required_app_state_id_ == current_app_state_id_)
        return;

    assert(required_app_state_id_ != AppStateId::null);

    if (current_app_state_id_ != AppStateId::null)
    {
        unique_ptr<AppStateBase>& current_app_state = app_states_[to_underlying(current_app_state_id_)];
        current_app_state->on_leave();
    }

    previous_app_state_id_ = current_app_state_id_;
    current_app_state_id_ = required_app_state_id_;

    unique_ptr<AppStateBase>& required_app_state = app_states_[to_underlying(required_app_state_id_)];
    required_app_state->on_enter();
}
