Import("env")

import os
from configparser import ConfigParser
from pathlib import Path


MODULE_FLAGS = {
    "logging": "FEATURE_MODULE_LOGGING",
    "sd_logging": "FEATURE_MODULE_SD_LOGGING",
    "secondary_serial": "FEATURE_MODULE_SECONDARY_SERIAL",
    "comms_extended": "FEATURE_MODULE_COMMS_EXTENDED",
    "can": "FEATURE_MODULE_CAN",
    "table_switching": "FEATURE_MODULE_TABLE_SWITCHING",
    "engine_protection": "FEATURE_MODULE_ENGINE_PROTECTION",
    "launch_flatshift": "FEATURE_MODULE_LAUNCH_FLATSHIFT",
    "launch_control": "FEATURE_MODULE_LAUNCH_CONTROL",
    "launch_control": "FEATURE_MODULE_LAUNCH_CONTROL",
    "etb": "FEATURE_MODULE_ETB",
    "fan_aircon": "FEATURE_MODULE_FAN_AIRCON",
    "programmable_io": "FEATURE_MODULE_PROGRAMMABLE_IO",
    "nitrous": "FEATURE_MODULE_NITROUS",
    "knock": "FEATURE_MODULE_KNOCK",
    "advanced_engine": "FEATURE_MODULE_ADVANCED_ENGINE",
}

ADVANCED_FLAGS = {
    "boost": "FEATURE_ADVANCED_BOOST",
    "vvt": "FEATURE_ADVANCED_VVT",
    "wmi": "FEATURE_ADVANCED_WMI",
    "launch_flatshift": "FEATURE_ADVANCED_LAUNCH_FLATSHIFT",
}

MODULE_SOURCES = {
    "logging": [
        "modules/logging/module_logging.cpp",
        "modules/logging/page_registry_logging.cpp",
        "modules/logging/logger.cpp",
        "modules/logging/logger_readable.cpp",
        "modules/logging/logger_controls.cpp",
    ],
    "sd_logging": [
        "modules/sd_logging/module_sd_logging.cpp",
        "modules/sd_logging/page_registry_sd_logging.cpp",
        "modules/sd_logging/logger_status.cpp",
        "modules/sd_logging/SD_logger.cpp",
        "modules/sd_logging/rtc_common.cpp",
        "modules/sd_logging/TS_CommandButtonHandler.cpp",
    ],
    "secondary_serial": [
        "modules/secondary_serial/module_secondary_serial.cpp",
        "modules/secondary_serial/secondary_serial.cpp",
    ],
    "comms_extended": [
        "modules/comms_extended/module_comms_extended.cpp",
        "modules/comms_extended/page_registry_comms.cpp",
    ],
    "can": [
        "modules/can/module_can.cpp",
        "modules/can/page_registry_can.cpp",
        "modules/can/can.cpp",
        "modules/can/can_transport.cpp",
    ],
    "table_switching": [
        "modules/table_switching/module_table_switching.cpp",
        "modules/table_switching/page_registry_tables.cpp",
        "modules/table_switching/secondaryTables.cpp",
    ],
    "etb": [
        "modules/etb/module_etb.cpp",
        "modules/etb/page_registry_etb.cpp",
        "modules/etb/etb_storage.cpp",
    ],
    "knock": [
        "modules/knock/module_knock.cpp",
        "modules/knock/page_registry_knock.cpp",
    ],
    "engine_protection": [
        "modules/engine_protection/engine_protection.cpp",
    ],
    "launch_flatshift": [
        "modules/launch_flatshift/module_launch_flatshift.cpp",
        "modules/launch_flatshift/launch_flatshift.cpp",
    ],
    "launch_control": [
        "modules/launch_control/module_launch_control.cpp",
        "modules/launch_control/launch_control.cpp",
    ],
    "launch_control": [
        "modules/launch_control/module_launch_control.cpp",
        "modules/launch_control/launch_control.cpp",
    ],
    "advanced_engine": [
        "modules/advanced_engine/module_advanced_engine.cpp",
        "modules/advanced_engine/page_registry_advanced.cpp",
        "modules/advanced_engine/boost.cpp",
        "modules/advanced_engine/vvt.cpp",
        "modules/advanced_engine/wmi.cpp",
    ],
    "fan_aircon": [
        "modules/fan_aircon/module_fan_aircon.cpp",
        "modules/fan_aircon/fan_aircon.cpp",
    ],
    "nitrous": [
        "modules/nitrous/module_nitrous.cpp",
        "modules/nitrous/nitrous.cpp",
    ],
}


def _bool_from_text(value, default=False):
    if value is None:
        return default
    normalized = str(value).strip().lower()
    if normalized in ("1", "on", "true", "yes", "y", "enabled"):
        return True
    if normalized in ("0", "off", "false", "no", "n", "disabled"):
        return False
    return default


def _load_ini(path):
    parser = ConfigParser()
    parser.optionxform = str
    parser.read(path, encoding="utf-8")
    return parser


def _merge_sections(base, overlay, section):
    if not overlay.has_section(section):
        return
    if not base.has_section(section):
        base.add_section(section)
    for key, value in overlay.items(section):
        base.set(section, key, value)


def _resolve_config(project_dir):
    config_path = Path(os.getenv("SPEEDUINO_FEATURE_CONFIG", str(project_dir / "firmware.config.ini")))
    preset_dir = config_path.parent / "firmware.presets"
    override_profile = os.getenv("SPEEDUINO_FEATURE_PROFILE", "").strip()

    parser = ConfigParser()
    parser.optionxform = str

    if config_path.exists():
        parser.read(config_path, encoding="utf-8")

    profile = "full"
    if parser.has_section("meta"):
        profile = parser.get("meta", "profile", fallback=profile).strip() or profile
    if override_profile:
        profile = override_profile

    preset_path = preset_dir / f"{profile}.ini"
    if preset_path.exists():
        preset_parser = _load_ini(preset_path)
        for section in preset_parser.sections():
            _merge_sections(parser, preset_parser, section)

    # Re-apply local config so explicit values override the preset.
    if config_path.exists():
        local_parser = _load_ini(config_path)
        for section in local_parser.sections():
            _merge_sections(parser, local_parser, section)

    return parser, config_path, preset_path, profile


def _resolve_flags(config):
    modules = {}
    for key, define in MODULE_FLAGS.items():
        modules[define] = _bool_from_text(
            config.get("modules", key, fallback="on"),
            default=True,
        )

    advanced = {}
    advanced_enabled = modules["FEATURE_MODULE_ADVANCED_ENGINE"]
    for key, define in ADVANCED_FLAGS.items():
        advanced[define] = _bool_from_text(
            config.get("advanced_engine", key, fallback="on"),
            default=True,
        ) if advanced_enabled else False

    return modules, advanced


def _build_src_filter(modules):
    filters = ["+<*>"]
    for key, define in MODULE_FLAGS.items():
        if modules[define]:
            continue
        filters.extend(["-" + pattern for pattern in MODULE_SOURCES[key]])
    return filters


def _write_generated_header(path, modules, advanced, profile, board):
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "#pragma once",
        "",
        "// Generated by tools/pio_feature_config.py",
        f"// profile: {profile}",
        f"// board: {board}",
        "",
    ]
    for define, enabled in modules.items():
        lines.append(f"#undef {define}")
        lines.append(f"#define {define} {1 if enabled else 0}")
        lines.append("")
    for define, enabled in advanced.items():
        lines.append(f"#undef {define}")
        lines.append(f"#define {define} {1 if enabled else 0}")
        lines.append("")
    lines.append("#undef SPEEDUINO_FEATURE_CONFIGURED")
    lines.append("#define SPEEDUINO_FEATURE_CONFIGURED 1")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


config, config_path, preset_path, profile = _resolve_config(Path(env.subst("$PROJECT_DIR")))
board = env.GetProjectOption("board") or config.get("meta", "board", fallback="")
modules, advanced = _resolve_flags(config)

env.Append(CPPDEFINES=[("SPEEDUINO_FEATURE_CONFIGURED", 1)])
env.Append(CPPPATH=[str(Path(env.subst("$PROJECT_DIR")) / "speeduino")])

generated_header = Path(env.subst("$PROJECT_DIR")) / "speeduino" / "generated" / "generated_feature_config.h"
_write_generated_header(generated_header, modules, advanced, profile, board)

print("Speeduino feature config:")
print(f"  config: {config_path}")
print(f"  preset: {preset_path}")
print(f"  profile: {profile}")
print(f"  board: {board}")
