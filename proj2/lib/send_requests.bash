#!/usr/bin/bash
set -o pipefail

# Optional: warn if extra args provided
if [ "$#" -gt 3 ]; then
  echo "Usage: $0 [max iterations] [clients per second] [client prefix] [datagram host]"
  exit 1
fi

# CLI Params
MAX_ITS=${1:-1000}
CLIENTS_PER_SEC=${2:-50}
CLIENT_PREFIX="${3:-client_endpoint}"
DATAGRAM_HOST="${4:-datagram_host}"

# Global Consts
CLIENT="bin/proj2-client"
CLIENT_CMD=${CLIENT_CMD:-"$CLIENT"}  # command to run (single string)
SAFETY_MARGIN=${SAFETY_MARGIN:-100}  # keep this many process slots free
ULIMIT_MAX=$(ulimit -u)              # per-user process limit

# Global mutables
g_running=0
g_launched_in_window=0
g_window_start_ns=$(date +%s%N)
g_i=0

if [[ -z "$ULIMIT_MAX" || "$ULIMIT_MAX" -le 0 ]]; then
  echo "Failed to read ulimit -u" >&2
  exit 1
fi

MAX_CONCURRENCY=$(( ULIMIT_MAX - SAFETY_MARGIN ))
if (( MAX_CONCURRENCY < 1 )); then
  echo "ULIMIT too small ($ULIMIT_MAX) after safety margin $SAFETY_MARGIN" >&2
  exit 1
fi

print_dashboard() {
  clear
  echo "                        Sending Requests"
  echo "-----------------------------------------------------------------------"
  echo "Datagram host: $DATAGRAM_HOST"
  echo "Client($CLIENT) endpoint prefix: ${CLIENT_PREFIX}_{1...$MAX_ITS}"
  echo "Max clients: $MAX_ITS"
  echo "Max clients per second: $CLIENTS_PER_SEC"
  echo File Set: dat/low{1..5}.dat dat/high{1..5}.dat | fold -s | sed '2,$s/^/    /'
  echo
  percent=$(( 100 * g_i / MAX_ITS ))
  echo "Clients Launched: (${percent}%)"
  echo
}

select_files() {
  local -a low_pool=(dat/low{1..5}.dat)
  local -a high_pool=(dat/high{1..5}.dat)
  local -a selection=()
  local n index select_low roll

  # Random N in [2,10]  (if you change max files in ./dat/ update the 10)
  n=$(( RANDOM % 9 + 2 ))

  while (( ${#selection[@]} < n )); do
    # one pool is empty: must pick from the other
    if (( ${#low_pool[@]} == 0 )); then
      select_low=0
    elif (( ${#high_pool[@]} == 0 )); then
      select_low=1
    # neither empty
    else
      if (( n < 4 )); then  # if few files, send larger
        # biased: 75% choose high, 25% choose low
        roll=$(( RANDOM % 4 ))
        if (( roll < 3 )); then
          select_low=0
        else
          select_low=1
        fi
      else
        # "fair" coin
        select_low=$(( RANDOM % 2 ))
      fi
    fi
  
    if (( select_low )); then
      index=$(( RANDOM % ${#low_pool[@]} ))
      selection+=( "${low_pool[$index]}" )
      # update pool w/out selected index
      low_pool=( "${low_pool[@]:0:$index}" "${low_pool[@]:$((index+1))}" )
    else
      index=$(( RANDOM % ${#high_pool[@]} ))
      selection+=( "${high_pool[$index]}" )
      # update pool w/out selected index
      high_pool=( "${high_pool[@]:0:$index}" "${high_pool[@]:$((index+1))}" )
    fi
  done
  printf '%s ' "${selection[@]}"
}


# Helper to get elapsed ns in the current window
elapsed_ns() { echo $(( $(date +%s%N) - g_window_start_ns )); }

# Main loop
print_dashboard
for ((; g_i<MAX_ITS; g_i++)); do
  # reset 1s window when elapsed >= 1e9 ns, i.e., 1s
  if (( $(elapsed_ns) >= 1000000000 )); then
    g_launched_in_window=0
    g_window_start_ns=$(date +%s%N)
    print_dashboard
  fi

  # launched desired count in this 1s, wait for at least one child (no busy-waitin)
  if (( g_launched_in_window >= CLIENTS_PER_SEC )); then
    print_dashboard
    wait -n || true  # child crash does not halt script
    (( g_running-- ))
    continue
  fi

  # if at concurrency cap, wait for one child to exit before launching more
  if (( g_running >= MAX_CONCURRENCY )); then
    print_dashboard
    wait -n || true  # child crash does not halt script
    (( g_running-- ))
    continue
  fi

  # capture as a space-delimited string
  selection_str=$(select_files)
  
  # convert back to an array if needed
  read -r -a selection <<< "$selection_str"

  client_host="${CLIENT_PREFIX}_${g_i}"

  # Launch and check that the launch succeeded (start child in background)
  if "$CLIENT_CMD" "$client_host" "$DATAGRAM_HOST" "${selection[@]}" > /dev/null & then
    (( g_running++ ))
    (( g_launched_in_window++ ))
  else
    printf 'failed to start %s\n' "$CLIENT_CMD" >&2
    # do not increment counts
  fi
done
print_dashboard

wait
print_dashboard
echo "All Clients Terminated"
