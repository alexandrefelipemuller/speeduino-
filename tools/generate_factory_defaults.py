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


def generate_thermistor_curve():
    points = [
        (-40.0 + 273.15, 100700.0),
        (30.0 + 273.15, 2238.0),
        (99.0 + 273.15, 177.0),
    ]
    logs = [__import__("math").log(resistance) for temperature, resistance in points]
    inv_t = [1.0 / temperature for temperature, resistance in points]
    matrix = [[1.0, logs[index], logs[index] ** 3] for index in range(3)]
    augmented = [row[:] + [inv_t[index]] for index, row in enumerate(matrix)]
    for pivot_index in range(3):
        pivot = max(range(pivot_index, 3), key=lambda row_index: abs(augmented[row_index][pivot_index]))
        augmented[pivot_index], augmented[pivot] = augmented[pivot], augmented[pivot_index]
        divisor = augmented[pivot_index][pivot_index]
        for column in range(pivot_index, 4):
            augmented[pivot_index][column] /= divisor
        for row_index in range(3):
            if row_index == pivot_index:
                continue
            factor = augmented[row_index][pivot_index]
            for column in range(pivot_index, 4):
                augmented[row_index][column] -= factor * augmented[pivot_index][column]

    a_coef, b_coef, c_coef = [augmented[index][3] for index in range(3)]

    def temperature_from_resistance(resistance):
        log_resistance = __import__("math").log(resistance)
        inv_temperature = a_coef + b_coef * log_resistance + c_coef * (log_resistance ** 3)
        return (1.0 / inv_temperature) - 273.15

    bins = [(index * 33) for index in range(31)] + [1023]
    values = []
    for adc in bins:
        if adc <= 0:
            values.append(255)
            continue
        if adc >= 1023:
            values.append(0)
            continue
        resistance = 2490.0 * adc / (1023.0 - adc)
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
source_dir = project_dir / "speeduino_kiasoul"
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

clt_bins, clt_values = generate_thermistor_curve()
iat_bins, iat_values = generate_thermistor_curve()
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
