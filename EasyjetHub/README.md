# EasyjetHub package folder structure

- `bin/`: Executables
  - `easyjet-ntupler`
  - `easyjet-gridsubmit`
  - `easyjet-test`
  - `metadata-check`
- `datasets/`: Text files holding dataset lists for grid submission
- `python/`: Python modules
- `share/`: Run configuration yaml files
- `src/`: C++ code

## Notes on dependencies

This module should not depend on any `*Analysis` packages!
Each analysis is expected to depend only on this and the other non-analysis packages in easyjet.
