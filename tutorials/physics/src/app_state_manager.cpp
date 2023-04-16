#include "app_state_manager.hpp"

#include "app_state_main_screen.hpp"
#include "app_state_spacewar.hpp"

#include <dv_log.hpp>


AppStateManager::AppStateManager()
{
    app_states_.emplace_back(nullptr); // AppStateId::app_state_null
    app_states_.emplace_back(std::make_unique<AppStateMainScreen>());
    app_states_.emplace_back(std::make_unique<AppStateSpacewar>());

    instance_ = this;
    Log::write_debug("AppStateManager constructed");
}

AppStateManager::~AppStateManager()
{
}

void AppStateManager::apply()
{
    if (required_app_state_id_ == current_app_state_id_)
        return;

    assert(required_app_state_id_ != AppStateId::app_state_null);

    if (current_app_state_id_ != AppStateId::app_state_null)
    {
        unique_ptr<AppStateBase>& current_app_state = app_states_[current_app_state_id_];
        current_app_state->on_leave();
    }

    previous_app_state_id_ = current_app_state_id_;
    current_app_state_id_ = required_app_state_id_;

    unique_ptr<AppStateBase>& required_app_state = app_states_[required_app_state_id_];
    required_app_state->on_enter();
}
