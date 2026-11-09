# Hotel simulation (PROI)

Discrete-time hotel management simulation: reception, housekeeping, meals, staff payroll, random failures, and configurable logging.

## Team project

Built by a team of two for the Object-Oriented Programming course (PROI) at Warsaw University of Technology.
Work split as documented in the final report ([docs/final_report_PL.pdf](docs/final_report_PL.pdf), section 10):

**Konrad Orlikowski** ([@orlik11-code](https://github.com/orlik11-code))
- Room hierarchy: abstract `Room` with `SingleRoom`, `DoubleRoom`, `TripleRoom` and `Apartment`;
  cleaning/repair states, nightly price and staffing calculations
- Guests: `Guest` hierarchy (`Family`, `Business`, `VIP`) with satisfaction multipliers;
  `Reservation`, `Bill` and `HotelReport` with rating thresholds
- Random events: `EventGenerator` using the shared `simulation_rng`, `HotelEvent` classes
  (`PowerOutage`, `FireAlarm`)
- `SimEventLog` structures and binary search by tick (`lower_bound`) used by the `Logger`

**Bartosz Grzanka** ([@Turbotoster7](https://github.com/Turbotoster7))
- Simulation engine: `Simulation` class and the domain modules `Simulation*.cpp`
- Staff: `Staff` hierarchy, `Task` with `make_*` factories, fatigue model, per-tick payroll,
  schedule validation
- Logging: two-layer throttling (console vs `SimEventLog`), export via `Logger::prepare_log`
- Configuration loading: `ConfigLoader` (sections + CLI), `HotelLoader`, `StaffLoader`
- Demo scenario `config/hotel.txt`: 18 rooms, 19 staff members, 3 simulated days

**Together**
- Unit tests (16 files) and CTest integration
- Tuning the demo scenario's staffing based on no-show log analysis
- Agreeing on interfaces between domain classes and `Simulation`
- Final documentation and diagrams

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
| `log_every_n_ticks` | Print `[time]` summary every N ticks (other events print immediately) |
| `log_file` | Export event log to this path after the run |
| `log_file_every_n_ticks` | File export step (`0` = same as console) |

Demo dataset: `config/hotel.txt` (Hotel Grand Aurora, ~3 simulated days).


--hotel <path> whole scenato
--config <path> additional configurations key=value
--staff <path> only personel
--max-ticks N ile tickow
--minutes-per-tick N min to tick
--event-probability X 0.3 = 0,3%, 5 = 5% poer tick
--log-every-n-ticks N periot dime
--log-file <path> log file
--log-file-every-n-ticks N interval