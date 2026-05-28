#!/usr/bin/env python3
"""Generate subway station and distance CSVs from Korail public CSV files."""

from __future__ import annotations

import csv
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CLIENT_DATA_DIR = ROOT / "client_data"
DATA_DIR = ROOT / "data"

STATION_POSITION_HEADER_0 = "\ucca0\ub3c4\uc6b4\uc601\uae30\uad00"
KORAIL_DISTANCE_HEADER_0 = "\ucca0\ub3c4\uc6b4\uc601\uae30\uad00\uba85"
STATION_SUFFIX = "\uc5ed"
MAIN_LINE = "\uc218\ub3c4\uad8c \uc804\ucca0 1\ud638\uc120"
TRANSFER_LINES_BY_STATION = {
    "\uc544\uc0b0": ["\ucc9c\uc548\uc544\uc0b0\uc5ed(KTX)", "\ucc9c\uc548\uc544\uc0b0\uc5ed(SRT)"],
}

TARGET_STATIONS = [
    "\uc131\ud658",
    "\uc9c1\uc0b0",
    "\ub450\uc815",
    "\ucc9c\uc548",
    "\ubd09\uba85",
    "\uc30d\uc6a9",
    "\uc544\uc0b0",
    "\ud0d5\uc815",
    "\ubc30\ubc29",
    "\uc628\uc591\uc628\ucc9c",
    "\uc2e0\ucc3d",
]

ADDRESS_BY_STATION = {
    "\uc131\ud658": "\ucda9\uccad\ub0a8\ub3c4 \ucc9c\uc548\uc2dc \uc11c\ubd81\uad6c \uc131\ud658\uc74d",
    "\uc9c1\uc0b0": "\ucda9\uccad\ub0a8\ub3c4 \ucc9c\uc548\uc2dc \uc11c\ubd81\uad6c \uc9c1\uc0b0\uc74d",
    "\ub450\uc815": "\ucda9\uccad\ub0a8\ub3c4 \ucc9c\uc548\uc2dc \uc11c\ubd81\uad6c \ub450\uc815\ub3d9",
    "\ucc9c\uc548": "\ucda9\uccad\ub0a8\ub3c4 \ucc9c\uc548\uc2dc \ub3d9\ub0a8\uad6c \ub300\ud765\ub3d9",
    "\ubd09\uba85": "\ucda9\uccad\ub0a8\ub3c4 \ucc9c\uc548\uc2dc \ub3d9\ub0a8\uad6c \ubd09\uba85\ub3d9",
    "\uc30d\uc6a9": "\ucda9\uccad\ub0a8\ub3c4 \ucc9c\uc548\uc2dc \uc11c\ubd81\uad6c \uc30d\uc6a9\ub3d9",
    "\uc544\uc0b0": "\ucda9\uccad\ub0a8\ub3c4 \uc544\uc0b0\uc2dc \ubc30\ubc29\uc74d \uc7a5\uc7ac\ub9ac",
    "\ud0d5\uc815": "\ucda9\uccad\ub0a8\ub3c4 \uc544\uc0b0\uc2dc \ud0d5\uc815\uba74",
    "\ubc30\ubc29": "\ucda9\uccad\ub0a8\ub3c4 \uc544\uc0b0\uc2dc \ubc30\ubc29\uc74d",
    "\uc628\uc591\uc628\ucc9c": "\ucda9\uccad\ub0a8\ub3c4 \uc544\uc0b0\uc2dc \uc628\ucc9c\ub3d9",
    "\uc2e0\ucc3d": "\ucda9\uccad\ub0a8\ub3c4 \uc544\uc0b0\uc2dc \uc2e0\ucc3d\uba74",
}


def find_csv_by_first_header(expected: str) -> Path:
    for path in sorted(CLIENT_DATA_DIR.glob("*.csv")) + sorted(CLIENT_DATA_DIR.glob("*.CSV")):
        try:
            with path.open("r", encoding="cp949", newline="") as fp:
                header = next(csv.reader(fp), [])
        except UnicodeDecodeError:
            continue
        if header and header[0].strip() == expected:
            return path
    raise FileNotFoundError(f"client_data CSV with first header {expected!r} not found")


def station_key(name: str) -> str:
    clean = re.sub(r"\([^)]*\)", "", name or "").strip()
    if clean.endswith(STATION_SUFFIX):
        clean = clean[:-1]
    return clean


def read_position_rows(path: Path) -> dict[str, tuple[str, str]]:
    positions: dict[str, tuple[str, str]] = {}
    with path.open("r", encoding="cp949", newline="") as fp:
        reader = csv.reader(fp)
        next(reader, None)
        for row in reader:
            if len(row) < 5:
                continue
            name = station_key(row[2])
            lng = row[3].strip()
            lat = row[4].strip()
            if name and lat and lng:
                positions[name] = (lat, lng)
    return positions


def read_distance_rows(path: Path):
    line_by_station: dict[str, set[str]] = {}
    row_by_station: dict[str, list[str]] = {}
    with path.open("r", encoding="cp949", newline="") as fp:
        reader = csv.reader(fp)
        next(reader, None)
        for row in reader:
            if len(row) < 5:
                continue
            line_name = row[1].strip()
            name = station_key(row[2])
            if not name:
                continue
            line_by_station.setdefault(name, set()).add(line_name)
            row_by_station[name] = row
    return line_by_station, row_by_station


def minutes_from_distance(distance_km: str) -> int:
    try:
        distance = float(distance_km)
    except ValueError:
        return 2
    minutes = int(distance * 2.0 + 0.5)
    return max(2, minutes)


def line_display_name(_lines: set[str]) -> str:
    return MAIN_LINE


def transfer_display_name(station_name: str, lines: set[str]) -> str:
    transfers = list(TRANSFER_LINES_BY_STATION.get(station_name, []))
    transfers.extend(sorted(line for line in lines if line and not line.startswith("1\ud638\uc120")))
    return ",".join(transfers) if transfers else "-"


def write_csv(path: Path, header: list[str], rows: list[list[str | int]]) -> None:
    with path.open("w", encoding="utf-8", newline="") as fp:
        writer = csv.writer(fp)
        writer.writerow(header)
        writer.writerows(rows)


def main() -> None:
    position_source = find_csv_by_first_header(STATION_POSITION_HEADER_0)
    distance_source = find_csv_by_first_header(KORAIL_DISTANCE_HEADER_0)
    positions = read_position_rows(position_source)
    line_by_station, distance_by_station = read_distance_rows(distance_source)

    station_id_by_name = {
        name: f"ST{idx:03d}"
        for idx, name in enumerate(TARGET_STATIONS, start=1)
    }

    station_rows: list[list[str]] = []
    missing_positions: list[str] = []
    for name in TARGET_STATIONS:
        if name not in positions:
            missing_positions.append(name)
            continue
        lat, lng = positions[name]
        station_lines = line_by_station.get(name, set())
        station_rows.append(
            [
                station_id_by_name[name],
                f"{name}{STATION_SUFFIX}",
                line_display_name(station_lines),
                lat,
                lng,
                ADDRESS_BY_STATION.get(name, ""),
                transfer_display_name(name, station_lines),
            ]
        )

    distance_rows: list[list[str | int]] = []
    for current_name, next_name in zip(TARGET_STATIONS, TARGET_STATIONS[1:]):
        current_id = station_id_by_name[current_name]
        next_id = station_id_by_name[next_name]
        source_row = distance_by_station.get(current_name, [])
        distance_km = source_row[4].strip() if len(source_row) >= 5 and source_row[4].strip() else ""
        if not distance_km:
            source_row = distance_by_station.get(next_name, [])
            distance_km = source_row[3].strip() if len(source_row) >= 4 else ""
        if not distance_km:
            distance_km = "0"
        distance_rows.append(
            [
                current_id,
                next_id,
                MAIN_LINE,
                distance_km,
                minutes_from_distance(distance_km),
            ]
        )

    write_csv(
        DATA_DIR / "subway_stations.csv",
        ["station_id", "station_name", "line_no", "lat", "lng", "address", "transfer_lines"],
        station_rows,
    )
    write_csv(
        DATA_DIR / "subway_distances.csv",
        ["from_station_id", "to_station_id", "line_no", "distance_km", "estimated_minutes"],
        distance_rows,
    )

    print(f"position_source={position_source}")
    print(f"distance_source={distance_source}")
    print(f"subway_stations={len(station_rows)}")
    print(f"subway_distances={len(distance_rows)}")
    print(f"missing_positions={','.join(missing_positions) if missing_positions else '-'}")


if __name__ == "__main__":
    main()
