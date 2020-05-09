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
  parameter object for one setting in AP_OSD
 */

#include "AP_OSD.h"

const AP_Param::GroupInfo AP_OSD_Setting::var_info[] = {
    // @Param: _EN
    // @DisplayName: Enable
    // @Description: Enable setting
    // @Values: 0:Disabled,1:Enabled
    // @User: Standard
    AP_GROUPINFO("_EN", 1, AP_OSD_Setting, enabled, 0),

    // @Param: _X
    // @DisplayName: X position
    // @Description: Horizontal position on screen
    // @Range: 0 29
    // @User: Standard
    AP_GROUPINFO("_X", 2, AP_OSD_Setting, xpos, 0),

    // @Param: _Y
    // @DisplayName: Y position
    // @Description: Vertical position on screen
    // @Range: 0 15
    // @User: Standard
    AP_GROUPINFO("_Y", 3, AP_OSD_Setting, ypos, 0),

    AP_GROUPEND
};

const AP_Param::GroupInfo AP_OSD_ParamSetting::var_info[] = {
    // @Param: _EN
    // @DisplayName: Enable
    // @Description: Enable setting
    // @Values: 0:Disabled,1:Enabled
    // @User: Standard
    AP_GROUPINFO("_EN", 1, AP_OSD_ParamSetting, enabled, 0),

    // @Param: _X
    // @DisplayName: X position
    // @Description: Horizontal position on screen
    // @Range: 0 29
    // @User: Standard
    AP_GROUPINFO("_X", 2, AP_OSD_ParamSetting, xpos, 0),

    // @Param: _Y
    // @DisplayName: Y position
    // @Description: Vertical position on screen
    // @Range: 0 15
    // @User: Standard
    AP_GROUPINFO("_Y", 3, AP_OSD_ParamSetting, ypos, 0),

    // @Param: _GRP
    // @DisplayName: Parameter group
    // @Description: Group of the parameter to be displayed and modified
    // @User: Standard
    AP_GROUPINFO("_GRP", 4, AP_OSD_ParamSetting, _param_group, 0),

    // @Param: _KEY
    // @DisplayName: Parameter key
    // @Description: Key of the parameter to be displayed and modified
    // @User: Standard
    AP_GROUPINFO("_KEY", 5, AP_OSD_ParamSetting, _param_key_idx, 0),

    // @Param: _MIN
    // @DisplayName: Parameter minimum
    // @Description: Minimum value of the parameter to be displayed and modified
    // @User: Standard
    AP_GROUPINFO("_MIN", 6, AP_OSD_ParamSetting, _param_min, 0.0f),

    // @Param: _MAX
    // @DisplayName: Parameter maximum
    // @Description: Maximum of the parameter to be displayed and modified
    // @User: Standard
    AP_GROUPINFO("_MAX", 7, AP_OSD_ParamSetting, _param_max, 1.0f),

    // @Param: _INCR
    // @DisplayName: Parameter increment
    // @Description: Increment of the parameter to be displayed and modified
    // @User: Standard
    AP_GROUPINFO("_INCR", 8, AP_OSD_ParamSetting, _param_incr, 0.001f),

    AP_GROUPEND
};

#define PARAM_INDEX(key, group) (uint32_t((int16_t(key) << 18) | int32_t(group)))

extern const AP_HAL::HAL& hal;

// constructor
AP_OSD_Setting::AP_OSD_Setting(bool _enabled, uint8_t x, uint8_t y)
{
    enabled = _enabled;
    xpos = x;
    ypos = y;
}

// constructor
AP_OSD_ParamSetting::AP_OSD_ParamSetting(uint8_t param_number, bool _enabled, uint8_t x, uint8_t y, uint32_t idx, float min, float max, float incr)
    : AP_OSD_Setting(_enabled, x, y), _param_number(param_number)
{
    _param_group = idx & 0x3FFFF;
    _param_key_idx = (idx & 0xFFFC0000) >> 18;
}

// constructor
AP_OSD_ParamSetting::AP_OSD_ParamSetting(uint8_t param_number, bool _enabled, uint8_t x, uint8_t y, const char* name, float min, float max, float incr)
    : AP_OSD_Setting(_enabled, x, y), _param_number(param_number), _param_name_default(true)
{
    _param_group = -1;
    _param_key_idx = -1;
    strncpy(_param_name, name, 16);
}

// update the contained parameter
void AP_OSD_ParamSetting::update()
{
    if (_current_index == PARAM_INDEX(_param_key_idx, _param_group) && _param_key_idx >= 0) {
        return;
    }
    // if a parameter was configured then use that
    AP_Param::ParamToken token {};
    if (_param_group.configured() && _param_key_idx.configured()) {
        uint32_t key  = uint32_t(_param_key_idx.get()) >> 5;
        uint32_t idx = uint32_t(_param_key_idx.get()) | 0x1F;
       // token.key = key - 1;
        for (param = AP_Param::first(&token, &_param_type);
            param && (token.key != key || token.idx != idx || token.group_element != uint32_t(_param_group.get()));
            param = AP_Param::next_scalar(&token, &_param_type)) {
        }
    // a name was specified as the default
    } else if (_param_name_default) {
        param = AP_Param::find(_param_name, &_param_type);
        for (AP_Param* ap=AP_Param::first(&token, &_param_type);
            ap != param;
            ap=AP_Param::next_scalar(&token, &_param_type)) {
        }
        _param_group = token.group_element;
        _param_key_idx = token.key << 5 | token.idx;
    }

    _current_index = PARAM_INDEX(_param_key_idx, _param_group);

    if (param == nullptr) {
        enabled = false;
    } else {
        param->copy_name_token(token, _param_name, 16);
        _param_name[16] = 0;
    }
}
