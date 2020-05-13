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

#define PARAM_COMPOSITE_INDEX(key, group) (uint32_t((int16_t(key) << 18) | int32_t(group)))

#define OSD_PARAM_DEBUG 0
#if OSD_PARAM_DEBUG
#define debug(fmt, args ...) do { hal.console->printf("OSD: " fmt, args); } while (0)
#else
#define debug(fmt, args ...)
#endif

static const char* SERIAL_PROTOCOL_VALUES[] = {
    "", "MAVLink", "MAVLink2", "Frsky D", "Frsky SPort", "GPS", "Alexmos", "SToRM32", "Rangefinder", "FrSky OpenTX",
    "Lidar360" "Beacon", "Volz", "SBus out", "ESC Telem", "Devo Teleme", "OpticalFlow", "RobotisServo",
    "NMEA Output", "WindVane", "SLCAN", "RCIN", "MegaSquirt", "LTM", "RunCam", "HottTelem", "Scripting"
};

// at the cost of a little flash, we can create much better ranges for certain
// important settings
const AP_OSD_ParamSetting::ParamMetadata AP_OSD_ParamSetting::_param_metadata[] = {
    { PARAM_INDEX(6, 0, 11), -1.0f, 28.0f, 1.0f, ParamMetadata::StringValues, 28, SERIAL_PROTOCOL_VALUES }
};

extern const AP_HAL::HAL& hal;

// constructor
AP_OSD_ParamSetting::AP_OSD_ParamSetting(uint8_t param_number, bool _enabled, uint8_t x, uint8_t y, int32_t group, int16_t key, float min, float max, float incr)
    : AP_OSD_Setting(_enabled, x, y), _param_number(param_number)
{
    _param_group = group;
    _param_key_idx = key;
}

// update the contained parameter
void AP_OSD_ParamSetting::update()
{
    if (PARAM_TOKEN_INDEX(_current_token) == PARAM_COMPOSITE_INDEX(_param_key_idx, _param_group) && _param_key_idx >= 0) {
        return;
    }
    // if a parameter was configured then use that
    _current_token = AP_Param::ParamToken {};
    uint32_t key  = uint32_t(_param_key_idx.get()) >> 5;
    uint32_t idx = uint32_t(_param_key_idx.get()) & 0x1F;
    // surely there is a more efficient way than brute-force search
    for (param = AP_Param::first(&_current_token, &_param_type);
        param && (_current_token.key != key || _current_token.idx != idx || _current_token.group_element != uint32_t(_param_group.get()));
        param = AP_Param::next_scalar(&_current_token, &_param_type)) {
    }
    guess_ranges();

    if (param == nullptr) {
        enabled = false;
    }
}

// guess the ranges and increment for the selected parameter
void AP_OSD_ParamSetting::guess_ranges()
{
    if (param->is_read_only()) {
        return;
    }

    // check for statically configured setting metadata
    if (set_from_metadata()) {
        return;
    }

    // nothing statically configured so guess some appropriate values
    _metadata_index = -1;

    if (param != nullptr) {
        switch (_param_type) {
        case AP_PARAM_INT8:
        {
            _param_incr = 1;
            _param_max = 127;
            _param_min = -1;
        }
            break;
        case AP_PARAM_INT16:
        {
            AP_Int16* p = (AP_Int16*)param;
            _param_min = -1;
            uint8_t digits = 0;
            for (int16_t int16p = p->get(); int16p > 0; int16p /= 10) {
                digits++;
            }
            _param_incr = MAX(1, powf(10, digits - 2));
            _param_max = powf(10, digits + 1);
            debug("Guessing range for value %d as %f -> %f, %f\n",
                p->get(), _param_min.get(), _param_max.get(), _param_incr.get());
        }
            break;
        case AP_PARAM_INT32:
        {
            AP_Int32* p = (AP_Int32*)param;
            _param_min = -1;
            uint8_t digits = 0;
            for (int32_t int32p = p->get(); int32p > 0; int32p /= 10) {
                digits++;
            }
            _param_incr = MAX(1, powf(10, digits - 2));
            _param_max = powf(10, digits + 1);
            debug("Guessing range for value %d as %f -> %f, %f\n",
                p->get(), _param_min.get(), _param_max.get(), _param_incr.get());
        }
            break;
        case AP_PARAM_FLOAT:
        {
            AP_Float* p = (AP_Float*)param;

            uint8_t digits = 0;
            for (float floatp = p->get(); floatp > 1.0f; floatp /= 10) {
                digits++;
            }
            if (digits < 1) {
                _param_incr = 0.001f;
                _param_max = 1.0;
                _param_min = 0.0f;
            } else {
                _param_incr = MAX(1, powf(10, digits - 2));
                _param_max = powf(10, digits + 1);
                _param_min = 0.0f;
            }
            debug("Guessing range for value %f as %f -> %f, %f\n",
                p->get(), _param_min.get(), _param_max.get(), _param_incr.get());
        }
            break;
        case AP_PARAM_VECTOR3F:
        case AP_PARAM_NONE:
        case AP_PARAM_GROUP:
            break;
        }
    }
}

bool AP_OSD_ParamSetting::set_from_metadata()
{
    // check for statically configured setting metadata
    for (uint16_t i = 0; i < ARRAY_SIZE(_param_metadata); i++) {
        if (_param_metadata[i].index == PARAM_TOKEN_INDEX(_current_token)) {
            _param_incr = _param_metadata[i].increment;
            _param_min = _param_metadata[i].min_value;
            _param_max = _param_metadata[i].max_value;
            _metadata_index = i;
            return true;
        }
    }
    return false;
}

