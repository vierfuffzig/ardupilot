/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * AP_OSD partially based on betaflight and inav osd.c implemention.
 * clarity.mcm font is taken from inav configurator.
 * Many thanks to their authors.
 */
/*
  parameter settings for one screen
 */
#include "AP_OSD.h"
#include "AP_OSD_Backend.h"

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/Util.h>
#include <limits.h>
#include <ctype.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_RCMapper/AP_RCMapper.h>
#include <AP_Arming/AP_Arming.h>

const AP_Param::GroupInfo AP_OSD_ParamScreen::var_info[] = {

    // @Param: ENABLE
    // @DisplayName: Enable screen
    // @Description: Enable this screen
    // @Values: 0:Disabled,1:Enabled
    // @User: Standard
    AP_GROUPINFO_FLAGS("ENABLE", 1, AP_OSD_ParamScreen, enabled, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: CHAN_MIN
    // @DisplayName: Transmitter switch screen minimum pwm
    // @Description: This sets the PWM lower limit for this screen
    // @Range: 900 2100
    // @User: Standard
    AP_GROUPINFO("CHAN_MIN", 2, AP_OSD_ParamScreen, channel_min, 900),

    // @Param: CHAN_MAX
    // @DisplayName: Transmitter switch screen maximum pwm
    // @Description: This sets the PWM upper limit for this screen
    // @Range: 900 2100
    // @User: Standard
    AP_GROUPINFO("CHAN_MAX", 3, AP_OSD_ParamScreen, channel_max, 2100),

    // @Param: PARAM1_EN
    // @DisplayName: PARAM1_EN
    // @Description: Enables display of parameter 1
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM1_X
    // @DisplayName: PARAM1_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM1_Y
    // @DisplayName: PARAM1_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[0], "PARAM1", 4, AP_OSD_ParamScreen, AP_OSD_ParamSetting),
    
    // @Param: PARAM2_EN
    // @DisplayName: PARAM21_EN
    // @Description: Enables display of parameter 2
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM2_X
    // @DisplayName: PARAM2_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM2_Y
    // @DisplayName: PARAM2_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[1], "PARAM2", 5, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    // @Param: PARAM3_EN
    // @DisplayName: PARAM3_EN
    // @Description: Enables display of parameter 3
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM3_X
    // @DisplayName: PARAM3_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM3_Y
    // @DisplayName: PARAM3_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[2], "PARAM3", 6, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    // @Param: PARAM4_EN
    // @DisplayName: PARAM4_EN
    // @Description: Enables display of parameter 4
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM4_X
    // @DisplayName: PARAM4_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM4_Y
    // @DisplayName: PARAM4_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[3], "PARAM4", 7, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    // @Param: PARAM5_EN
    // @DisplayName: PARAM5_EN
    // @Description: Enables display of parameter 5
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM5_X
    // @DisplayName: PARAM5_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM5_Y
    // @DisplayName: PARAM5_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[4], "PARAM5", 8, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    // @Param: PARAM6_EN
    // @DisplayName: PARAM6_EN
    // @Description: Enables display of parameter 6
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM6_X
    // @DisplayName: PARAM6_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM6_Y
    // @DisplayName: PARAM6_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[5], "PARAM6", 9, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    // @Param: PARAM7_EN
    // @DisplayName: PARAM7_EN
    // @Description: Enables display of parameter 7
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM7_X
    // @DisplayName: PARAM7_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM7_Y
    // @DisplayName: PARAM7_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[6], "PARAM7", 10, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    // @Param: PARAM8_EN
    // @DisplayName: PARAM8_EN
    // @Description: Enables display of parameter 8
    // @Values: 0:Disabled,1:Enabled

    // @Param: PARAM8_X
    // @DisplayName: PARAM8_X
    // @Description: Horizontal position on screen
    // @Range: 0 29

    // @Param: PARAM8_Y
    // @DisplayName: PARAM8_Y
    // @Description: Vertical position on screen
    // @Range: 0 15
    AP_SUBGROUPINFO(params[7], "PARAM8", 11, AP_OSD_ParamScreen, AP_OSD_ParamSetting),

    AP_GROUPEND
};

#define OSD_HOLD_BUTTON_PRESS_DELAY 100

#define OSD_PARAM_DEBUG 0

#if OSD_PARAM_DEBUG
static const char* event_names[5] = {
        "NONE", "MENU_ENTER", "MENU_UP", "MENU_DOWN", "IN_MENU_EXIT"
};
#define debug(fmt, args ...) do { hal.console->printf("OSD: " fmt, args); } while (0)
#else
#define debug(fmt, args ...)
#endif

extern const AP_HAL::HAL& hal;

void AP_OSD_ParamScreen::draw_parameter(uint8_t number, uint8_t x, uint8_t y)
{
    AP_OSD_ParamSetting& setting = params[number-1];
    setting.update();

    ap_var_type type = setting._param_type;
    AP_Param* p =  setting.param;
    if (p != nullptr) {
        const bool selected = number == _selected_param;
        uint16_t value_pos = strlen(setting._param_name) + 3;
        backend->write(x, y, selected && !_modify_param, "%s:", setting._param_name);

        const bool blink = selected && _modify_param;
        switch (type) {
        case AP_PARAM_INT8:
            backend->write(value_pos, y, blink, "%hhd", ((AP_Int8*)p)->get());
            break;
        case AP_PARAM_INT16:
            backend->write(value_pos, y, blink, "%hd", ((AP_Int16*)p)->get());
            break;
        case AP_PARAM_INT32:
            backend->write(value_pos, y, blink, "%d", ((AP_Int32*)p)->get());
            break;
        case AP_PARAM_FLOAT:
            backend->write(value_pos, y, blink, "%.3f", ((AP_Float*)p)->get());
            break;
        case AP_PARAM_VECTOR3F:
        {
            Vector3f vec = ((AP_Vector3f*)p)->get();
            backend->write(x, y, blink, "[%.3f,%.3f,%.3f]", vec.x, vec.y, vec.z);
        }
            break;
        case AP_PARAM_NONE:
        case AP_PARAM_GROUP:
            break;
        }
    }
}

// modify the selected parameter number
void AP_OSD_ParamScreen::modify_parameter(uint8_t number, Event ev)
{
    const AP_OSD_ParamSetting& setting = params[number-1];
    AP_Param* p = setting.param;

    if (p->is_read_only()) {
        return;
    }

    float incr = setting._param_incr * ((ev == Event::MENU_DOWN) ? -1.0f : 1.0f);
    int32_t incr_int = int32_t(roundf(incr));
    int32_t max_int = int32_t(roundf(setting._param_max));
    int32_t min_int = int32_t(roundf(setting._param_min));

    if (p != nullptr) {
        switch (setting._param_type) {
        // there is no way to validate the ranges, so as a rough guess prevent
        // integer types going below -1;
        case AP_PARAM_INT8: 
        {
            AP_Int8* param = (AP_Int8*)p;
            param->set(constrain_int16(param->get() + incr_int, min_int, max_int));
        }
            break;
        case AP_PARAM_INT16:
        {
            AP_Int16* param = (AP_Int16*)p;
            param->set(constrain_int16(param->get() + incr_int, min_int, max_int));
        }
            break;
        case AP_PARAM_INT32:
        {
            AP_Int32* param = (AP_Int32*)p;
            param->set(constrain_int32(param->get() + incr_int, min_int, max_int));
        }
            break;
        case AP_PARAM_FLOAT:
        {
            AP_Float* param = (AP_Float*)p;
            param->set(constrain_float(param->get() + incr, setting._param_min, setting._param_max));
        }
            break;
        case AP_PARAM_VECTOR3F:
        {
            AP_Vector3f* param = (AP_Vector3f*)p;
            Vector3f vec = param->get();
            vec.x = constrain_float(vec.x + incr, setting._param_min, setting._param_max);
            vec.y = constrain_float(vec.y + incr, setting._param_min, setting._param_max);
            vec.z = constrain_float(vec.z + incr, setting._param_min, setting._param_max);
            param->set(vec);
        }
            break;
        case AP_PARAM_NONE:
        case AP_PARAM_GROUP:
            break;
        }
    }
}

// modify the selected parameter number
void AP_OSD_ParamScreen::save_parameter(uint8_t number)
{
    AP_Param* p = params[number-1].param;
    if (p != nullptr) {
        p->save();
    }
}

// return radio values as LOW, MIDDLE, HIGH
RC_Channel::aux_switch_pos_t AP_OSD_ParamScreen::get_channel_pos(uint8_t rcmapchan) const
{
    RC_Channel::aux_switch_pos_t position = RC_Channel::LOW;
    const RC_Channel* chan = rc().channel(rcmapchan-1);
    if (chan == nullptr || !chan->read_3pos_switch(position)) {
        return RC_Channel::LOW;
    }

    return position;
}

// map rc input to an event
AP_OSD_ParamScreen::Event AP_OSD_ParamScreen::map_rc_input_to_event() const
{
    const RC_Channel::aux_switch_pos_t throttle = get_channel_pos(AP::rcmap()->throttle());
    const RC_Channel::aux_switch_pos_t yaw = get_channel_pos(AP::rcmap()->yaw());
    const RC_Channel::aux_switch_pos_t roll = get_channel_pos(AP::rcmap()->roll());
    const RC_Channel::aux_switch_pos_t pitch = get_channel_pos(AP::rcmap()->pitch());

    Event result = Event::NONE;

    if (yaw != RC_Channel::MIDDLE || throttle != RC_Channel::LOW) {
        return result;
    }

    if (pitch == RC_Channel::MIDDLE && roll == RC_Channel::LOW) {
        result = Event::MENU_EXIT;
    } else if (pitch == RC_Channel::MIDDLE && roll == RC_Channel::HIGH) {
        result = Event::MENU_ENTER;
    } else if (pitch == RC_Channel::LOW && roll == RC_Channel::MIDDLE) {
        result = Event::MENU_UP;
    } else if (pitch == RC_Channel::HIGH && roll == RC_Channel::MIDDLE) {
        result = Event::MENU_DOWN;
    } else {
        // OSD option has not changed so assume stick re-centering
        result = Event::NONE;
    }
    return result;
}

// update the state machine when disarmed
void AP_OSD_ParamScreen::update_state_machine()
{
    const uint32_t now = AP_HAL::millis();
    if ((now - _transition_start_ms) < _transition_timeout_ms) {
        return;
    }

    const Event ev = map_rc_input_to_event();
    // only take action on transitions
    if (ev == Event::NONE && ev == _last_rc_event) {
        return;
    }

    debug("update_state_machine(%s)\n", event_names[int(ev)]);

    _transition_start_ms = now;
    if (ev == _last_rc_event) {
        _transition_timeout_ms = OSD_HOLD_BUTTON_PRESS_DELAY;
    } else {
        _transition_timeout_ms = osd->button_delay_ms;
    }
    _last_rc_event = ev;

    // if we were armed then there is no selected parameter - so find one
    if (_selected_param == 0) {
        _selected_param = 1;
        for (uint8_t i = 0; i < NUM_PARAMS && !params[_selected_param-1].enabled; i++) {
            _selected_param++;
        }
    }

    switch (ev) {
    case Event::MENU_ENTER:
        _modify_param = true;
        break;
    case Event::MENU_UP:
        if (_modify_param) {
            modify_parameter(_selected_param, ev);
        } else {
            _selected_param = (_selected_param + NUM_PARAMS - 2) % NUM_PARAMS + 1;
            // skip over parameters that are not enabled
            for (uint8_t i = 0; i < NUM_PARAMS && !params[_selected_param-1].enabled; i++) {
                _selected_param = (_selected_param + NUM_PARAMS - 2) % NUM_PARAMS + 1;
            }
        }
        break;
    case Event::MENU_DOWN:
        if (_modify_param) {
            modify_parameter(_selected_param, ev);
        } else {
            _selected_param = _selected_param % NUM_PARAMS + 1;
            // skip over parameters that are not enabled
            for (uint8_t i = 0; i < NUM_PARAMS && !params[_selected_param-1].enabled; i++) {
                _selected_param = _selected_param % NUM_PARAMS + 1;
            }
        }
        break;
    case Event::MENU_EXIT:
        if (_modify_param) {
            save_parameter(_selected_param);
            _modify_param = false;
        }
        break;
    case Event::NONE:
        break;
    }
}

void AP_OSD_ParamScreen::draw(void)
{
    if (!enabled || !backend) {
        return;
    }

    // first update the state machine
    if (!AP::arming().is_armed()) {
        update_state_machine();
    } else {
        _selected_param = 0;
    }

    for (uint8_t i = 0; i < NUM_PARAMS; i++) {
        AP_OSD_ParamSetting n = params[i];
        if (n.enabled) {
            draw_parameter(n._param_number, n.xpos, n.ypos);
        }
    }
}

// pre_arm_check - returns true if all pre-takeoff checks have completed successfully
bool AP_OSD::pre_arm_check(char *failure_msg, const uint8_t failure_msg_len) const
{
    // currently in the OSD menu, do not allow arming
    if (!is_readonly_screen()) {
        hal.util->snprintf(failure_msg, failure_msg_len, "In OSD menu");
        return false;
    }

    // if we got this far everything must be ok
    return true;
}

