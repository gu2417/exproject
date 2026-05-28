#!/usr/bin/env python3
"""Generate bus route CSVs from Asan route and dispatch CSV files.

Inputs used:
- client_data/*노선운행정보_20250901.csv
- client_data/*버스운행정보_20250901.csv

The vehicle status CSV (*버스정보_20250901.csv) is intentionally not used
because it does not contain route-stop mappings.
"""

from __future__ import annotations

import csv
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CLIENT_DATA_DIR = ROOT / "client_data"
DATA_DIR = ROOT / "data"

ROUTE_HEADER_0 = "\ub178\uc120\uc815\ubcf4\uadf8\ub8f9"
DISPATCH_HEADER_0 = "\ubc30\ucc28\uadf8\ub8f9ID"


def find_csv_by_first_header(expected: str) -> Path:
    for path in sorted(CLIENT_DATA_DIR.glob("*.csv")) + sorted(CLIENT_DATA_DIR.glob("*.CSV")):
        try:
            with path.open("r", encoding="cp949", newline="") as fp:
                reader = csv.reader(fp)
                header = next(reader, [])
        except UnicodeDecodeError:
            continue
        if header and header[0].strip() == expected:
            return path
    raise FileNotFoundError(f"client_data CSV with first header {expected!r} not found")


def read_cp949_rows(path: Path, min_columns: int) -> list[list[str]]:
    rows: list[list[str]] = []
    with path.open("r", encoding="cp949", newline="") as fp:
        reader = csv.reader(fp)
        header = next(reader, None)
        if not header or len(header) < min_columns:
            raise ValueError(f"unexpected header in {path}")
        for row in reader:
            if len(row) >= min_columns:
                rows.append([cell.strip() for cell in row])
    return rows


def read_stop_names() -> dict[str, str]:
    names: dict[str, str] = {}
    path = DATA_DIR / "bus_stops.csv"
    if not path.exists():
        return names
    with path.open("r", encoding="utf-8-sig", newline="") as fp:
        for row in csv.DictReader(fp):
            stop_id = row.get("stop_id", "").strip()
            stop_name = row.get("stop_name", "").strip()
            if stop_id:
                names[stop_id] = stop_name
    return names


def normalize_route_name(group_name: str, description: str) -> str:
    name = group_name.strip()
    desc = description.strip()

    if re.fullmatch(r"\d{1,4}", name):
        return f"{name}\ubc88"
    if re.fullmatch(r"\uc21c\ud658\s*\d{1,4}", name):
        return f"{name.replace(' ', '')}\ubc88"
    if name:
        return name

    match = re.search(r"(\d{1,4})\s*\ubc88", desc)
    if match:
        return f"{match.group(1)}\ubc88"
    return desc or "\ub178\uc120\uba85 \ubbf8\uc0c1"


def normalize_direction(description: str, end_stop_id: str, stop_names: dict[str, str]) -> str:
    desc = description.strip().replace("_", " ")
    if "\ubc29\uba74" in desc:
        return desc
    end_name = stop_names.get(end_stop_id, "").strip()
    if end_name:
        return f"{end_name} \ubc29\uba74"
    return desc or "\ubc29\uba74 \ubbf8\uc0c1"


def is_inactive_text(value: str) -> bool:
    lowered = value.lower()
    blocked = [
        "test",
        "\ud14c\uc2a4\ud2b8",
        "\uc2e0\uaddc\ud14c\uc2a4\ud2b8",
        "\uacbd\ub85c \uc218\uc815",
        "\ubbf8\uc0c1",
        "\uc0ad\uc81c",
    ]
    return any(word in lowered for word in blocked)


def build_rows(
    route_rows: list[list[str]],
    dispatch_names: dict[str, str],
    stop_names: dict[str, str],
) -> tuple[list[list[str | int]], list[list[str | int]], int]:
    bus_routes: list[list[str | int]] = []
    route_stops: list[list[str | int]] = []
    skipped = 0

    for row in sorted(route_rows, key=lambda item: item[0]):
        route_no = row[0]
        description = row[1]
        start_stop_id = row[4]
        end_stop_id = row[5]

        if route_no == "0" or not route_no or not start_stop_id or not end_stop_id:
            skipped += 1
            continue
        if is_inactive_text(description):
            skipped += 1
            continue

        route_name = normalize_route_name(dispatch_names.get(route_no, ""), description)
        direction = normalize_direction(description, end_stop_id, stop_names)

        bus_routes.append([route_no, route_name, start_stop_id, end_stop_id, 0])
        route_stops.append([route_no, 1, start_stop_id, direction])
        if end_stop_id != start_stop_id:
            route_stops.append([route_no, 2, end_stop_id, direction])

    return bus_routes, route_stops, skipped


def write_csv(path: Path, header: list[str], rows: list[list[str | int]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as fp:
        writer = csv.writer(fp)
        writer.writerow(header)
        writer.writerows(rows)


def main() -> None:
    route_source = find_csv_by_first_header(ROUTE_HEADER_0)
    dispatch_source = find_csv_by_first_header(DISPATCH_HEADER_0)
    route_rows = read_cp949_rows(route_source, 7)
    dispatch_rows = read_cp949_rows(dispatch_source, 5)
    stop_names = read_stop_names()

    dispatch_names = {
        row[0]: row[1]
        for row in dispatch_rows
        if row[0] and row[3] == "1"
    }

    bus_routes, route_stops, skipped = build_rows(route_rows, dispatch_names, stop_names)

    write_csv(
        DATA_DIR / "bus_routes.csv",
        ["route_no", "route_name", "start_stop_id", "end_stop_id", "interval_min"],
        bus_routes,
    )
    write_csv(
        DATA_DIR / "route_stops.csv",
        ["route_no", "seq", "stop_id", "direction"],
        route_stops,
    )

    print(f"route_source={route_source}")
    print(f"dispatch_source={dispatch_source}")
    print(f"route_source_rows={len(route_rows)}")
    print(f"dispatch_routes={len(dispatch_names)}")
    print(f"bus_routes={len(bus_routes)}")
    print(f"route_stops={len(route_stops)}")
    print(f"skipped_route_rows={skipped}")
    print("vehicle_status_source=not used")


if __name__ == "__main__":
    main()
