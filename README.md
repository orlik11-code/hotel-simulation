# Hotel simulation (PROI)

Discrete-time hotel management simulation: reception, housekeeping, meals, staff payroll, random failures, and configurable logging.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

## Run

Preferred: load everything from one hotel config file.

```bash
./build/HotelApp --hotel config/hotel.txt
```

Override simulation settings from CLI:

```bash
./build/HotelApp --hotel config/hotel.txt --max-ticks 100 --log-every-n-ticks 5
```

Separate config file (optional):

```bash
./build/HotelApp --hotel config/hotel.txt --config config/simulation.example.conf
```

## Key config keys (`[simulation]` section or `--config` file)

| Key | Description |
|-----|-------------|
| `minutes_per_tick` | Model minutes advanced per tick (default 60) |
| `max_ticks` | Run length; `0` = one simulated day |
| `event_probability` | Failure chance per tick, 0.5–30 (e.g. `0.5` = 0.5%) |
| `date_start` | Calendar start `YYYY-MM-DD` |
| `log_every_n_ticks` | Print `[czas]` summary every N ticks (other events print immediately) |
| `log_file` | Export event log to this path after the run |
| `log_file_every_n_ticks` | File export step (`0` = same as console) |

Demo dataset: `config/hotel.txt` (Hotel Grand Aurora, ~3 simulated days).
