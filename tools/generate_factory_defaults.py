from pathlib import Path

Import("env")


def format_bytes(data: bytes) -> str:
    lines = []
    for start in range(0, len(data), 16):
        chunk = data[start:start + 16]
        lines.append("  " + ", ".join(f"0x{byte:02X}" for byte in chunk))
    return ",\n".join(lines)


def format_words(values):
    lines = []
    for start in range(0, len(values), 8):
        chunk = values[start:start + 8]
        lines.append("  " + ", ".join(str(value) for value in chunk))
    return ",\n".join(lines)


TEMP_PRESETS = {
    "Fiat": {
        "pullup_resistance": 2200.0,
        "points": [
            (0.0, 9000.0),
            (27.0, 2500.0),
            (100.0, 200.0),
        ],
    },
    "Hyundai HB20": {
        "pullup_resistance": 2200.0,
        "points": [
            (0.0, 9000.0),
            (40.0, 1100.0),
            (80.0, 320.0),
        ],
    },
}

DEFAULT_TEMP_PRESET = "Hyundai HB20"


def generate_temp_curve(preset_name: str):
    preset = TEMP_PRESETS[preset_name]
    points = preset["points"]
    pullup_resistance = preset["pullup_resistance"]
    min_temp_point = min(points, key=lambda point: point[0])
    max_temp_point = max(points, key=lambda point: point[0])
    max_res_point = max(points, key=lambda point: point[1])
    min_res_point = min(points, key=lambda point: point[1])

    def temperature_from_resistance(resistance):
        if resistance >= max_res_point[1]:
            return min_temp_point[0]
        if resistance <= min_res_point[1]:
            return max_temp_point[0]

        for (temp_a, res_a), (temp_b, res_b) in zip(points, points[1:]):
            if min(res_a, res_b) <= resistance <= max(res_a, res_b):
                span = res_b - res_a
                if span == 0:
                    return temp_a
                ratio = (resistance - res_a) / span
                return temp_a + ((temp_b - temp_a) * ratio)

        return max_temp_point[0]

    bins = [(index * 33) for index in range(31)] + [1023]
    values = []
    for adc in bins:
        if adc <= 0:
            value = int(round(max_temp_point[0] + 40.0))
        elif adc >= 1023:
            value = int(round(min_temp_point[0] + 40.0))
        else:
            resistance = pullup_resistance * adc / (1023.0 - adc)
            value = int(round(temperature_from_resistance(resistance) + 40.0))
        values.append(max(0, min(255, value)))

    return bins, values


def generate_wbo2_curve():
    bins = [index * 32 for index in range(32)]
    values = []
    for adc in bins:
        voltage = adc * 5.0 / 1023.0
        afr = 9.7 + (voltage - 1.0) * ((18.7 - 9.7) / (4.0 - 1.0))
        values.append(max(0, min(255, int(round(afr * 10.0)))))
    return bins, values


project_dir = Path(env["PROJECT_DIR"])
source_dir = project_dir / "kia_final"
output_dir = project_dir / "speeduino" / "generated"
output_file = output_dir / "factory_defaults_data.h"

page_files = sorted(source_dir.glob("page_*.bin"), key=lambda path: int(path.stem.split("_")[1]))
output_dir.mkdir(parents=True, exist_ok=True)

content = [
    "#pragma once",
    "#include <stdint.h>",
    "",
    "#ifndef PROGMEM",
    "#define PROGMEM",
    "#endif",
    "",
]

clt_bins, clt_values = generate_temp_curve(DEFAULT_TEMP_PRESET)
iat_bins, iat_values = generate_temp_curve(DEFAULT_TEMP_PRESET)
o2_bins, o2_values = generate_wbo2_curve()

for page_file in page_files:
    page_num = int(page_file.stem.split("_")[1])
    page_bytes = page_file.read_bytes()
    content.append(f"static const uint8_t factory_page_{page_num}[] PROGMEM = {{")
    content.append(format_bytes(page_bytes))
    content.append("};")
    content.append("")

content.append("static const uint16_t factory_clt_bins[] PROGMEM = {")
content.append(format_words(clt_bins))
content.append("};")
content.append("")
content.append("static const uint16_t factory_clt_values[] PROGMEM = {")
content.append(format_words(clt_values))
content.append("};")
content.append("")
content.append("static const uint16_t factory_iat_bins[] PROGMEM = {")
content.append(format_words(iat_bins))
content.append("};")
content.append("")
content.append("static const uint16_t factory_iat_values[] PROGMEM = {")
content.append(format_words(iat_values))
content.append("};")
content.append("")
content.append("static const uint16_t factory_o2_bins[] PROGMEM = {")
content.append(format_words(o2_bins))
content.append("};")
content.append("")
content.append("static const uint8_t factory_o2_values[] PROGMEM = {")
content.append(format_words(o2_values))
content.append("};")
content.append("")

new_content = "\n".join(content) + "\n"
if not output_file.exists() or output_file.read_text() != new_content:
    output_file.write_text(new_content)
