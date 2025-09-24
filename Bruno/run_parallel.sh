#!/usr/bin/env bash
set -euo pipefail

BIN="./knapSA"            # ajuste se necessário
ROOTDIR="./problemInstances"
RESULTS_CSV="results.csv"

# Check binary
if [ ! -x "$BIN" ]; then
  echo "Executable $BIN not found or not executable. Compile knapSA.cpp and place binary here."
  exit 1
fi

# Use /usr/bin/time if available for detailed resource usage
TIME_BIN="/usr/bin/time"
USE_TIME=0
if [ -x "$TIME_BIN" ]; then
  USE_TIME=1
fi

# Gather directories
mapfile -t DIRS < <(find "$ROOTDIR" -mindepth 1 -maxdepth 1 -type d | sort)

if [ ${#DIRS[@]} -eq 0 ]; then
  echo "No subdirectories found in $ROOTDIR"
  exit 1
fi

# Worker function for one directory
run_one(){
  d="$1"
  infile="${d}/test.in"
  outfile="${d}/outp.out"
  timefile="${d}/time.out"

  if [ ! -f "$infile" ]; then
    echo "No test.in in $d, skipping"
    return
  fi

  rm -f "$outfile" "$timefile"

  if [ "$USE_TIME" -eq 1 ]; then
    # format: real_seconds, max_rss_kb
    $TIME_BIN -f "real_seconds=%e\nmax_rss_kb=%M" -o "$timefile" "$BIN" "$infile" > "$outfile" 2>>"$timefile"
  else
    { time "$BIN" "$infile" > "$outfile"; } 2> "$timefile"
  fi
  echo "DONE: $d"
}

export -f run_one
export BIN ROOTDIR TIME_BIN USE_TIME

# number of parallel jobs = number of processors (fallback to 4)
NPROCS=4
if command -v nproc >/dev/null 2>&1; then
  NPROCS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
  NPROCS=$(sysctl -n hw.ncpu)
fi

echo "Running on $NPROCS jobs in parallel..."

# If GNU parallel available, prefer it
if command -v parallel >/dev/null 2>&1; then
  printf "%s\n" "${DIRS[@]}" | parallel -j "$NPROCS" run_one {}
else
  # fallback: xargs -P
  printf "%s\n" "${DIRS[@]}" | xargs -I{} -P "$NPROCS" bash -c 'run_one "$@"' _ {}
fi

echo "All runs finished. Aggregating results to $RESULTS_CSV ..."

# Write header
echo "instance,greedy,sa,time_seconds,max_rss_kb" > "$RESULTS_CSV"

for d in "${DIRS[@]}"; do
  name=$(basename "$d")
  outfile="${d}/outp.out"
  timefile="${d}/time.out"
  if [ ! -f "$outfile" ]; then
    echo "${name},,," >> "$RESULTS_CSV"
    continue
  fi

  gval=$(grep -m1 '^Greedy:' "$outfile" 2>/dev/null | awk '{print $2}')
  saval=$(grep -m1 '^S.A:' "$outfile" 2>/dev/null | awk '{print $2}')
  tsec=$(grep '^real_seconds=' "$timefile" 2>/dev/null | cut -d'=' -f2)
  maxrss=$(grep '^max_rss_kb=' "$timefile" 2>/dev/null | cut -d'=' -f2)

  # Ensure empty fields if missing
  gval=${gval:-}
  saval=${saval:-}
  tsec=${tsec:-}
  maxrss=${maxrss:-}

  echo "${name},${gval},${saval},${tsec},${maxrss}" >> "$RESULTS_CSV"
done

echo "Wrote $RESULTS_CSV"
echo "All done."