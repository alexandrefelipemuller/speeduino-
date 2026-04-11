#!/usr/bin/env python3

from __future__ import annotations

import argparse
import configparser
import os
import re
import subprocess
import sys
from pathlib import Path


MODULE_FLAGS = {
    "logging": "FEATURE_MODULE_LOGGING",
    "secondary_serial": "FEATURE_MODULE_SECONDARY_SERIAL",
    "comms_extended": "FEATURE_MODULE_COMMS_EXTENDED",
    "table_switching": "FEATURE_MODULE_TABLE_SWITCHING",
    "engine_protection": "FEATURE_MODULE_ENGINE_PROTECTION",
    "launch_flatshift": "FEATURE_MODULE_LAUNCH_FLATSHIFT",
    "fan_aircon": "FEATURE_MODULE_FAN_AIRCON",
    "programmable_io": "FEATURE_MODULE_PROGRAMMABLE_IO",
    "nitrous": "FEATURE_MODULE_NITROUS",
    "knock": "FEATURE_MODULE_KNOCK",
    "advanced_engine": "FEATURE_MODULE_ADVANCED_ENGINE",
}

ADVANCED_FEATURE_FLAGS = {
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
        "modules/logging/SD_logger.cpp",
        "modules/logging/rtc_common.cpp",
        "modules/logging/TS_CommandButtonHandler.cpp",
    ],
    "secondary_serial": [
        "modules/secondary_serial/module_secondary_serial.cpp",
        "modules/secondary_serial/secondary_serial.cpp",
    ],
    "comms_extended": [
        "modules/comms_extended/module_comms_extended.cpp",
        "modules/comms_extended/page_registry_comms.cpp",
        "modules/comms_extended/comms_CAN.cpp",
        "modules/comms_extended/can_transport.cpp",
    ],
    "table_switching": [
        "modules/table_switching/module_table_switching.cpp",
        "modules/table_switching/page_registry_tables.cpp",
        "modules/table_switching/secondaryTables.cpp",
    ],
    "knock": [
        "modules/knock/module_knock.cpp",
        "modules/knock/page_registry_knock.cpp",
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
    "engine_protection": [
        "modules/engine_protection/engine_protection.cpp",
    ],
    "launch_flatshift": [
        "modules/launch_flatshift/module_launch_flatshift.cpp",
        "modules/launch_flatshift/launch_flatshift.cpp",
    ],
}

DEFAULT_SERVICE_PROVIDERS = {
    "aux_pwm": "advanced_engine.vvt",
}


def _bool(value: str | None, default: bool = False) -> bool:
    if value is None:
        return default
    normalized = value.strip().lower()
    if normalized in ("1", "on", "true", "yes", "y", "enabled"):
        return True
    if normalized in ("0", "off", "false", "no", "n", "disabled"):
        return False
    return default


def _load_config(path: Path) -> configparser.ConfigParser:
    parser = configparser.ConfigParser()
    parser.optionxform = str
    if path.exists():
        parser.read(path, encoding="utf-8")
    return parser


def _apply_preset(base: configparser.ConfigParser, preset_path: Path) -> None:
    if not preset_path.exists():
        return
    preset = _load_config(preset_path)
    for section in preset.sections():
        if not base.has_section(section):
            base.add_section(section)
        for key, value in preset.items(section):
            base.set(section, key, value)


def _resolve_config(project_dir: Path, config_path: Path, override_profile: str | None):
    config = _load_config(config_path)
    profile = config.get("meta", "profile", fallback="full").strip() or "full"
    if override_profile:
        profile = override_profile
    _apply_preset(config, project_dir / "firmware.presets" / f"{profile}.ini")
    return config, profile


def _split_list(value: str | None) -> list[str]:
    if not value:
        return []
    return [item for item in re.split(r"[,\s]+", value) if item]


def _resolve_module_states(config: configparser.ConfigParser) -> tuple[dict[str, bool], dict[str, bool]]:
    modules = {}
    for key in MODULE_FLAGS:
        modules[key] = _bool(config.get("modules", key, fallback="on"), default=True)

    advanced = {}
    advanced_enabled = modules["advanced_engine"]
    for key in ADVANCED_FEATURE_FLAGS:
        advanced[key] = _bool(config.get("advanced_engine", key, fallback="on"), default=True) if advanced_enabled else False

    return modules, advanced


def _resolve_service_providers(config: configparser.ConfigParser) -> dict[str, str]:
    services = dict(DEFAULT_SERVICE_PROVIDERS)
    if config.has_section("services"):
        for key, value in config.items("services"):
            services[key] = value.strip() or services.get(key, "")
    return services


def _is_module_enabled(name: str, modules: dict[str, bool], advanced: dict[str, bool]) -> bool:
    if name in modules:
        return modules[name]
    if name in advanced:
        return modules["advanced_engine"] and advanced[name]
    return False


def _service_is_enabled(name: str, services: dict[str, str], modules: dict[str, bool], advanced: dict[str, bool]) -> bool:
    provider = services.get(name, "")
    if not provider:
        return False
    if "." in provider:
        owner, feature = provider.split(".", 1)
        if owner == "advanced_engine":
            return modules.get(owner, False) and advanced.get(feature, False)
        return False
    return _is_module_enabled(provider, modules, advanced)


def _validate_dependencies(config: configparser.ConfigParser, modules: dict[str, bool], advanced: dict[str, bool], services: dict[str, str]) -> None:
    errors: list[str] = []

    for key, enabled in advanced.items():
        if enabled and not modules["advanced_engine"]:
            errors.append(f"advanced_engine subfeature '{key}' requires advanced_engine = on")

    if config.has_section("dependencies"):
        for feature, dep_text in config.items("dependencies"):
            feature = feature.strip()
            if not feature:
                continue
            feature_enabled = _is_module_enabled(feature, modules, advanced)
            if not feature_enabled:
                continue
            for dep in _split_list(dep_text):
                if dep in services:
                    if not _service_is_enabled(dep, services, modules, advanced):
                        provider = services.get(dep, "<unset>")
                        errors.append(f"{feature} requires service '{dep}' (provider: {provider})")
                elif not _is_module_enabled(dep, modules, advanced):
                    errors.append(f"{feature} requires '{dep}'")

    if errors:
        raise SystemExit("Invalid feature configuration:\n- " + "\n- ".join(errors))


def _write_generated_header(project_dir: Path, config: configparser.ConfigParser, profile: str, board: str) -> Path:
    generated_header = project_dir / "speeduino" / "generated" / "generated_feature_config.h"
    generated_header.parent.mkdir(parents=True, exist_ok=True)

    lines = [
        "#pragma once",
        "",
        "// Generated by tools/build_firmware.py",
        f"// profile: {profile}",
        f"// board: {board}",
        "",
    ]
    for key, define in MODULE_FLAGS.items():
        enabled = _bool(config.get("modules", key, fallback="on"), default=True)
        lines.append(f"#undef {define}")
        lines.append(f"#define {define} {1 if enabled else 0}")
        lines.append("")
    for key, define in ADVANCED_FEATURE_FLAGS.items():
        enabled = _bool(config.get("advanced_engine", key, fallback="on"), default=True)
        lines.append(f"#undef {define}")
        lines.append(f"#define {define} {1 if enabled else 0}")
        lines.append("")
    lines.append("#undef SPEEDUINO_FEATURE_CONFIGURED")
    lines.append("#define SPEEDUINO_FEATURE_CONFIGURED 1")
    lines.append("")
    generated_header.write_text("\n".join(lines), encoding="utf-8")
    return generated_header


def _write_generated_ini(project_dir: Path, config: configparser.ConfigParser, profile: str) -> Path:
    generated_ini = project_dir / "local.feature.ini"
    build_flags = ["${env:megaatmega2560.build_flags}"]
    for key, define in MODULE_FLAGS.items():
        enabled = _bool(config.get("modules", key, fallback="on"), default=True)
        build_flags.append(f"-D{define}={1 if enabled else 0}")
    for key, define in ADVANCED_FEATURE_FLAGS.items():
        enabled = _bool(config.get("advanced_engine", key, fallback="on"), default=True)
        build_flags.append(f"-D{define}={1 if enabled else 0}")
    build_flags.append("-DSPEEDUINO_FEATURE_CONFIGURED=1")

    lines = [
        "[env:megaatmega2560-configurable]",
        "build_flags =",
        *[f"    {flag}" for flag in build_flags],
        "build_src_filter =",
        "    +<*>",
    ]
    for key in MODULE_FLAGS:
        enabled = _bool(config.get("modules", key, fallback="on"), default=True)
        if enabled:
            continue
        for pattern in MODULE_SOURCES[key]:
            lines.append(f"    -<{pattern}>")
    lines.extend(["", f"; generated profile: {profile}", ""])
    generated_ini.write_text("\n".join(lines), encoding="utf-8")
    return generated_ini


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Generate Speeduino feature config and launch PlatformIO.")
    parser.add_argument("-e", "--env", default="megaatmega2560-configurable", help="PlatformIO environment to build.")
    parser.add_argument("--config", default="firmware.config.ini", help="Feature config ini file.")
    parser.add_argument("--profile", default="", help="Override the profile from the config file.")
    parser.add_argument("--platformio", default="platformio", help="PlatformIO executable.")
    parser.add_argument("platformio_args", nargs=argparse.REMAINDER, help="Extra arguments passed to PlatformIO.")
    args = parser.parse_args(argv)

    project_dir = Path(__file__).resolve().parents[1]
    config_path = Path(args.config)
    if not config_path.is_absolute():
        config_path = project_dir / config_path

    config, profile = _resolve_config(project_dir, config_path, args.profile or None)
    board = config.get("meta", "board", fallback="") or args.env
    modules, advanced = _resolve_module_states(config)
    services = _resolve_service_providers(config)
    _validate_dependencies(config, modules, advanced, services)
    generated_header = _write_generated_header(project_dir, config, profile, board)
    generated_ini = _write_generated_ini(project_dir, config, profile)

    env = os.environ.copy()
    env["SPEEDUINO_FEATURE_CONFIG"] = str(config_path)
    env["SPEEDUINO_FEATURE_PROFILE"] = profile

    print("Speeduino feature config:")
    print(f"  config: {config_path}")
    print(f"  preset: {project_dir / 'firmware.presets' / (profile + '.ini')}")
    print(f"  profile: {profile}")
    print(f"  board: {board}")
    print(f"  header: {generated_header}")
    print(f"  ini: {generated_ini}")

    cmd = [args.platformio, "run", "-e", args.env]
    if args.platformio_args and args.platformio_args[0] == "--":
        cmd.extend(args.platformio_args[1:])
    elif args.platformio_args:
        cmd.extend(args.platformio_args)

    result = subprocess.run(cmd, cwd=project_dir, env=env)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
