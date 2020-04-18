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

    // @Param: _NAME
    // @DisplayName: Parameter index
    // @Description: Index of the parameter to be displayed and modified
    // @User: Standard
    AP_GROUPINFO("_IDX", 4, AP_OSD_ParamSetting, _param_index, 0),

    AP_GROUPEND
};

extern const AP_HAL::HAL& hal;

// constructor
AP_OSD_Setting::AP_OSD_Setting(bool _enabled, uint8_t x, uint8_t y)
{
    enabled = _enabled;
    xpos = x;
    ypos = y;
}

// constructor
AP_OSD_ParamSetting::AP_OSD_ParamSetting(uint8_t param_number, bool _enabled, uint8_t x, uint8_t y, uint16_t idx)
    : AP_OSD_Setting(_enabled, x, y), _param_number(param_number)
{
    _param_index = idx;
}

// constructor
AP_OSD_ParamSetting::AP_OSD_ParamSetting(uint8_t param_number, bool _enabled, uint8_t x, uint8_t y, const char* name)
    : AP_OSD_Setting(_enabled, x, y), _param_number(param_number)
{
    _param_index = -1;
    strncpy(_param_name, name, 16);
}

// update the contained parameter
void AP_OSD_ParamSetting::update()
{
    if (_current_index == _param_index && _param_index >= 0) {
        return;
    }

    AP_Param::ParamToken token;
    // a name was specified as the default
    if (_param_name[0] && !_param_index.configured()) {
        param = AP_Param::find(_param_name, &_param_type);
        uint16_t count = 0;
        for (AP_Param* p=AP_Param::first(&token, &_param_type); p && param != p; p=AP_Param::next_scalar(&token, &_param_type)) {
            count++;
        }
        _param_index = count;
    } else {
        uint16_t count=0;
        for (param=AP_Param::first(&token, &_param_type);
            param && count < _param_index;
            param=AP_Param::next_scalar(&token, &_param_type)) {
            count++;
        }
    }

    _current_index = _param_index;

    if (param == nullptr) {
        enabled = false;
    } else {
        param->copy_name_token(token, _param_name, 16);
        _param_name[16] = 0;
    }
}
