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

// common labels
// all strings must be upper case
static const char* SERIAL_PROTOCOL_VALUES[] = {
    "", "MAV", "MAV2", "FSKY_D", "FSKY_S", "GPS", "ALEX", "STORM", "RNG", "FSKY_TX",
    "LID360", "BEACN", "VOLZ", "SBUS", "ESC_TLM", "DEV_TLM", "OPTFLW", "RBTSRV",
    "NMEA", "WNDVNE", "SLCAN", "RCIN", "MGSQRT", "LTM", "RUNCAM", "HOT_TLM", "SCRIPT"
};

static const char* SERVO_FUNCTIONS[] = {
    "NONE", "RCPASS", "FLAP", "FLAP_AUTO", "AIL", "", "MNT_PAN", "MNT_TLT", "MNT_RLL", "MNT_OPEN", 
    "CAM_TRG", "", "MNT2_PAN", "MNT2_TLT", "MNT2_RLL", "MNT2_OPEN", "DIF_SPL_L1", "DIF_SPL_R1", "", "ELE",
    "", "RUD", "SPR_PMP", "SPR_SPIN", "FLPRON_L", "FLPRON_R", "GRND_STEER", "PARACHT", "GRIP", "GEAR",
    "ENG_RUN_EN", "HELI_RSC", "HELI_TAIL_RSC", "MOT_1", "MOT_2", "MOT_3", "MOT_4", "MOT_5", "MOT_6", "MOT_7",
    "MOT_8", "MOT_TLT", "", "", "", "", "", "", "", "",
    "", "RCIN_1", "RCIN_2", "RCIN_3", "RCIN_4", "RCIN_5", "RCIN_6", "RCIN_7", "RCIN_8", "RCIN_9",
    "RCIN_10", "RCIN_11", "RCIN_12", "RCIN_13", "RCIN_14", "RCIN_15", "RCIN_16", "IGN", "", "START",
    "THR", "TRCK_YAW", "TRCK_PIT", "THR_L", "THR_R", "TLTMOT_L", "TLTMOT_R", "ELEVN_L", "ELEVN_R", "VTAIL_L",
    "VTAIL_R", "BOOST_THR", "MOT_9", "MOT_10", "MOT_11", "MOT_12", "DIF_SPL_L2", "DIF_SPL_R2", "", "MAIN_SAIL",
    "CAM_ISO", "CAM_APTR", "CAM_FOC", "CAM_SH_SPD", "SCRPT_1", "SCRPT_2", "SCRPT_3", "SCRPT_4", "SCRPT_5", "SCRPT_6",
    "SCRPT_7", "SCRPT_8", "SCRPT_9", "SCRPT_10", "SCRPT_11", "SCRPT_12", "SCRPT_13", "SCRPT_14", "SCRPT_15", "SCRPT_16",
    "", "", "", "", "", "", "", "", "", "",
    "NEOPX_1", "NEOPX_2", "NEOPX_3", "NEOPX_4", "RAT_RLL", "RAT_PIT","RAT_THRST", "RAT_YAW", "WSAIL_EL", "PRLED_1",
    "PRLED_2", "PRLED_3", "PRLED_CLK"
};

#if APM_BUILD_TYPE(APM_BUILD_ArduCopter)
// copter labels

static const char* AUX_OPTIONS[] = {
    "NONE", "", "FLIP", "SIMP", "RTL", "SAV_TRM", "", "SAV_WP", "", "CAM_TRG",
    "RNG", "FENCE", "", "SSIMP", "ACRO_TRN", "SPRAY", "AUTO", "AUTOTN", "LAND", "GRIP",
    "", "CHUTE_EN", "CHUTE_RL", "CHUTE_3P", "MIS_RST", "ATT_FF", "ATT_ACC", "RET_MNT", "RLY", "LAND_GR",
    "LOST_SND", "M_ESTOP", "M_ILOCK", "BRAKE", "RLY2", "RLY3", "RLY4", "THROW", "OA_ADSB", "PR_LOIT",
    "OA_PROX", "ARM/DS", "SMRT_RTL", "INVERT", "", "", "RC_OVRD", "USR1", "USR2", "USR3",
    "", "", "ACRO", "", "", "GUIDE", "LOIT", "FOLLOW", "CLR_WP", "",
    "ZZAG", "ZZ_SVWP", "COMP_LRN", "", "", "GPS_DIS", "RLY5", "RLY6", "STAB", "PHOLD",
    "AHOLD", "FHOLD", "CIRCLE", "DRIFT", "", "", "STANDBY", "", "RCAM_CTL", "RCAM_OSD",
    "VISO_CAL", "DISARM", "", "ZZ_Auto", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "",
    "KILLIMU1", "KILLIMU2", "CAM_MOD_TOG"
};

static const char* FLTMODES[] = {
    "STAB", "ACRO", "ALTHOLD", "AUTO", "GUIDED", "LOIT", "RTL", "CIRC", "", "LAND",
    "", "DRFT", "", "SPORT", "FLIP", "ATUN", "POSHLD", "BRAKE", "THROW", "AVD_ADSB",
    "GUID_NOGPS", "SMRTRTL", "FLOHOLD", "FOLLOW", "ZIGZAG", "SYSID", "HELI_ARO"
};

static const char* FS_OPTIONS[] = {
    "NONE", "CONT_RCFS", "CONT_GCSFS", "CONT_RC/GCSFS", "", "", "", "CONT_LAND", "",
    "", "", "", "", "", "", "CONT_CTRL_GCS", "", "", "CONTNUE"
};

#elif APM_BUILD_TYPE(APM_BUILD_ArduPlane)
// plane labels

static const char* AUX_OPTIONS[] = {
    "NONE", "", "", "", "RTL", "", "", "", "", "CAM_TRG",
    "", "", "", "", "", "", "AUTO", "", "", "",
    "", "", "", "", "MIS_RST", "", "", "", "RLY", "LAND_GR",
    "LOST_SND", "M_ESTOP", "", "", "", "RLY3", "RLY4", "", "OA_ADSB", "",
    "", "ARM/DS", "", "INVERT", "", "", "RC_OVRD", "", "", "",
    "", "MANUAL", "", "", "", "GUIDE", "LOIT", "", "CLR_WP", "",
    "", "", "COMP_LRN", "", "REV_THR", "GPS_DIS", "RLY5", "RLY6", "", "",
    "", "", "CIRCLE", "", "", "", "", "TAKEOFF", "RCAM_CTL", "RCAM_OSD",
    "", "DSARM", "QASS3POS", "", "", "", "", "", "", "", 
    "", "", "", "", "", "", "", "", "", "",
    "KILLIMU1", "KILLIMU2", "CAM_TOG"
};

static const char* FLTMODES[] = {
    "MAN", "CIRC", "STAB", "TRAIN", "ACRO", "FBWA", "FBWB", "CRUISE", "ATUNE", "", "AUTO",
    "RTL", "LOIT", "TKOF", "ADSB", "GUID", "", "QSTAB", "QHOV", "QLOIT", "QLAND",
    "QRTL", "QTUNE", "QACRO"
};

static const char* FS_SHRT_ACTNS[] = {
    "CRC_NOCHNGE", "CIRC", "FBWA", "DSABLE"
};

static const char* FS_LNG_ACTNS[] = {
    "CNTNUE", "RTL", "GLIDE", "PARACHT"
};

#endif

// at the cost of a little flash, we can create much better ranges and values for certain important settings
#if APM_BUILD_TYPE(APM_BUILD_ArduPlane)
// plane parameters
const AP_OSD_ParamSetting::ParamMetadata AP_OSD_ParamSetting::_param_metadata[] = {
    
    { PARAM_INDEX(3, 0, 11), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES }, // SERIAL_PROTOCOL0
    { PARAM_INDEX(3, 0, 1), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL1
    { PARAM_INDEX(3, 0, 3), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL2
    { PARAM_INDEX(3, 0, 5), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL3
    { PARAM_INDEX(3, 0, 7), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL4
    { PARAM_INDEX(3, 0, 9), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL5
    { PARAM_INDEX(3, 0, 12), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES }, // SERIAL_PROTOCOL6

    { PARAM_INDEX(146, 0, 20550), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO1_FUNCTION
    { PARAM_INDEX(146, 0, 20614), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO2_FUNCTION
    { PARAM_INDEX(146, 0, 20678), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO3_FUNCTION
    { PARAM_INDEX(146, 0, 20742), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO4_FUNCTION
    { PARAM_INDEX(146, 0, 20806), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO5_FUNCTION
    { PARAM_INDEX(146, 0, 20870), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO6_FUNCTION
    { PARAM_INDEX(146, 0, 20934), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO7_FUNCTION
    { PARAM_INDEX(146, 0, 20998), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO8_FUNCTION
    { PARAM_INDEX(146, 0, 21062), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO9_FUNCTION
    { PARAM_INDEX(146, 0, 21126), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO10_FUNCTION
    { PARAM_INDEX(146, 0, 21190), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO11_FUNCTION
    { PARAM_INDEX(146, 0, 21254), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO12_FUNCTION
    { PARAM_INDEX(146, 0, 21318), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO13_FUNCTION
    { PARAM_INDEX(146, 0, 21382), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO14_FUNCTION
    { PARAM_INDEX(146, 0, 21446), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO15_FUNCTION
    { PARAM_INDEX(146, 0, 21510), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO16_FUNCTION
    
    { PARAM_INDEX(146, 0, 24647), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC1_OPTION
    { PARAM_INDEX(146, 0, 24711), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC2_OPTION
    { PARAM_INDEX(146, 0, 24775), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC3_OPTION
    { PARAM_INDEX(146, 0, 24839), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC4_OPTION
    { PARAM_INDEX(146, 0, 24903), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC5_OPTION
    { PARAM_INDEX(146, 0, 24967), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC6_OPTION
    { PARAM_INDEX(146, 0, 25031), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC7_OPTION
    { PARAM_INDEX(146, 0, 25095), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC8_OPTION
    { PARAM_INDEX(146, 0, 25159), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC9_OPTION
    { PARAM_INDEX(146, 0, 25223), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC10_OPTION
    { PARAM_INDEX(146, 0, 25287), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC11_OPTION
    { PARAM_INDEX(146, 0, 25351), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC12_OPTION
    { PARAM_INDEX(146, 0, 25415), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC13_OPTION
    { PARAM_INDEX(146, 0, 25479), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC14_OPTION
    { PARAM_INDEX(146, 0, 25543), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC15_OPTION
    { PARAM_INDEX(146, 0, 25607), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC16_OPTION

    { PARAM_INDEX(65, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE1
    { PARAM_INDEX(66, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE2
    { PARAM_INDEX(67, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE3
    { PARAM_INDEX(68, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE4
    { PARAM_INDEX(69, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE5
    { PARAM_INDEX(70, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE6
    { PARAM_INDEX(71, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // INITIAL_MODE

    { PARAM_INDEX(59, 0, 0), 0, 3, 1, ParamMetadata::StringValues, ARRAY_SIZE(FS_SHRT_ACTNS), FS_SHRT_ACTNS }, // FS_SHRT_ACTN
    { PARAM_INDEX(61, 0, 0), 0, 3, 1, ParamMetadata::StringValues, ARRAY_SIZE(FS_LNG_ACTNS), FS_LNG_ACTNS } // FS_LNG_ACTN

};

#elif APM_BUILD_TYPE(APM_BUILD_ArduCopter)
// copter parameters
const AP_OSD_ParamSetting::ParamMetadata AP_OSD_ParamSetting::_param_metadata[] = {
    
    { PARAM_INDEX(6, 0, 11), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES }, // SERIAL_PROTOCOL0
    { PARAM_INDEX(6, 0, 1), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL1
    { PARAM_INDEX(6, 0, 3), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL2
    { PARAM_INDEX(6, 0, 5), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL3
    { PARAM_INDEX(6, 0, 7), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL4
    { PARAM_INDEX(6, 0, 9), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES },  // SERIAL_PROTOCOL5
    { PARAM_INDEX(6, 0, 12), -1, 28, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERIAL_PROTOCOL_VALUES), SERIAL_PROTOCOL_VALUES }, // SERIAL_PROTOCOL6

    { PARAM_INDEX(96, 0, 20560), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO1_FUNCTION
    { PARAM_INDEX(96, 0, 20624), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO2_FUNCTION
    { PARAM_INDEX(96, 0, 20688), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO3_FUNCTION
    { PARAM_INDEX(96, 0, 20752), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO4_FUNCTION
    { PARAM_INDEX(96, 0, 20816), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO5_FUNCTION
    { PARAM_INDEX(96, 0, 20880), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO6_FUNCTION
    { PARAM_INDEX(96, 0, 20944), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO7_FUNCTION
    { PARAM_INDEX(96, 0, 21008), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO8_FUNCTION
    { PARAM_INDEX(96, 0, 21072), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO9_FUNCTION
    { PARAM_INDEX(96, 0, 21136), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO10_FUNCTION
    { PARAM_INDEX(96, 0, 21200), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO11_FUNCTION
    { PARAM_INDEX(96, 0, 21264), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO12_FUNCTION
    { PARAM_INDEX(96, 0, 21328), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO13_FUNCTION
    { PARAM_INDEX(96, 0, 21392), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO14_FUNCTION
    { PARAM_INDEX(96, 0, 21456), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO15_FUNCTION
    { PARAM_INDEX(96, 0, 21520), 0, 132, 1, ParamMetadata::StringValues, ARRAY_SIZE(SERVO_FUNCTIONS), SERVO_FUNCTIONS }, // SERVO16_FUNCTION
    
    { PARAM_INDEX(96, 0, 24657), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC1_OPTION
    { PARAM_INDEX(96, 0, 24721), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC2_OPTION
    { PARAM_INDEX(96, 0, 24785), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC3_OPTION
    { PARAM_INDEX(96, 0, 24849), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC4_OPTION
    { PARAM_INDEX(96, 0, 24913), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC5_OPTION
    { PARAM_INDEX(96, 0, 24977), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC6_OPTION
    { PARAM_INDEX(96, 0, 25041), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC7_OPTION
    { PARAM_INDEX(96, 0, 25105), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC8_OPTION
    { PARAM_INDEX(96, 0, 25169), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC9_OPTION
    { PARAM_INDEX(96, 0, 25233), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC10_OPTION
    { PARAM_INDEX(96, 0, 25297), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC11_OPTION
    { PARAM_INDEX(96, 0, 25361), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC12_OPTION
    { PARAM_INDEX(96, 0, 25425), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC13_OPTION
    { PARAM_INDEX(96, 0, 25489), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC14_OPTION
    { PARAM_INDEX(96, 0, 25553), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC15_OPTION
    { PARAM_INDEX(96, 0, 25617), 0, 104, 1, ParamMetadata::StringValues, ARRAY_SIZE(AUX_OPTIONS), AUX_OPTIONS },  // RC16_OPTION

    { PARAM_INDEX(27, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE1
    { PARAM_INDEX(28, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE2
    { PARAM_INDEX(29, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE3
    { PARAM_INDEX(30, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE4
    { PARAM_INDEX(31, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE5
    { PARAM_INDEX(32, 0, 0), 0, 23, 1, ParamMetadata::StringValues, ARRAY_SIZE(FLTMODES), FLTMODES }, // FLTMODE6

    { PARAM_INDEX(96, 0, 36), 0, 3, 1, ParamMetadata::StringValues, ARRAY_SIZE(FS_OPTIONS), FS_OPTIONS }  // FS_OPTIONS

};

#endif

extern const AP_HAL::HAL& hal;

// constructor
AP_OSD_ParamSetting::AP_OSD_ParamSetting(uint8_t param_number, bool _enabled, uint8_t x, uint8_t y, int32_t group, int16_t key, float min, float max, float incr)
    : AP_OSD_Setting(_enabled, x, y), _param_number(param_number), _metadata_index(-1)
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

    if (param == nullptr) {
        hal.console->printf("Couldn't find param for key/idx: %d, group: %d\n", int(_param_key_idx.get()), int(_param_group.get()));
        enabled = false;
    } else {
        guess_ranges();
    }
}

// guess the ranges and increment for the selected parameter
void AP_OSD_ParamSetting::guess_ranges(bool force)
{
    if (param->is_read_only()) {
        return;
    }

    // check for statically configured setting metadata
    if (set_from_metadata()) {
        return;
    }

    // nothing statically configured so guess some appropriate values
    float min = -1, max = 127, incr = 1;

    if (param != nullptr) {
        switch (_param_type) {
        case AP_PARAM_INT8:
            break;
        case AP_PARAM_INT16:
        {
            AP_Int16* p = (AP_Int16*)param;
            min = -1;
            uint8_t digits = 0;
            for (int16_t int16p = p->get(); int16p > 0; int16p /= 10) {
                digits++;
            }
            incr = MAX(1, powf(10, digits - 2));
            max = powf(10, digits + 1);
            debug("Guessing range for value %d as %f -> %f, %f\n", p->get(), min, max, incr);
        }
            break;
        case AP_PARAM_INT32:
        {
            AP_Int32* p = (AP_Int32*)param;
            min = -1;
            uint8_t digits = 0;
            for (int32_t int32p = p->get(); int32p > 0; int32p /= 10) {
                digits++;
            }
            incr = MAX(1, powf(10, digits - 2));
            max = powf(10, digits + 1);
            debug("Guessing range for value %d as %f -> %f, %f\n", p->get(), min, max, incr);
        }
            break;
        case AP_PARAM_FLOAT:
        {
            AP_Float* p = (AP_Float*)param;

            uint8_t digits = 0;
            for (float floatp = p->get(); floatp > 1.0f; floatp /= 10) {
                digits++;
            }
            float floatp = p->get();
            if (digits < 1) {
                if (!is_zero(floatp)) {
                    incr = floatp / 100.0f; // move in 1% increments
                } else {
                    incr = 0.01f; // move in absolute 1% increments
                }
                max = 1.0;
                min = 0.0f;
            } else {
                if (!is_zero(floatp)) {
                    incr = floatp / 100.0f; // move in 1% increments
                } else {
                    incr = MAX(1, powf(10, digits - 2));
                }
                max = powf(10, digits + 1);
                min = 0.0f;
            }
            debug("Guessing range for value %f as %f -> %f, %f\n", p->get(), min, max, incr);
        }
            break;
        case AP_PARAM_VECTOR3F:
        case AP_PARAM_NONE:
        case AP_PARAM_GROUP:
            break;
        }

        if (!_param_min.configured() || force) {
            _param_min = min;
        }
        if (!_param_max.configured() || force) {
            _param_max = max;
        }
        if (!_param_incr.configured() || force) {
            _param_incr = incr;
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
    _metadata_index = -1;
    return false;
}

// modify the selected parameter values
void AP_OSD_ParamSetting::save_as_new()
{
    _param_group.save();
    _param_key_idx.save();
    // the user has configured the range and increment, but the parameter
    // is no longer valid so reset these to guessed values
    guess_ranges(true);
    if (_param_min.configured()) {
        _param_min.save();
    }
    if (_param_max.configured()) {
        _param_max.save();
    }
    if (_param_incr.configured()) {
        _param_incr.save();
    }
}

