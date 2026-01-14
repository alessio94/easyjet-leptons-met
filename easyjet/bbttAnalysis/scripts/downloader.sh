#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <taskname-pattern> <channel> [days]"
    echo "Example: $0 'user.lesplend.EJ_0_35_0_v7_VBF*' LepHad 45"
    exit 1
fi

TASKNAME_PATTERN="$1"
SUFFIX_TAG="$2"
DAYS="${3:-60}"

SUFFIX="_output-tree_${SUFFIX_TAG}.root"

pbook show fin taskname="$TASKNAME_PATTERN" format='plain' days="$DAYS" | awk -v suffix="$SUFFIX" '
BEGIN { RS="________________________________________________________________+"; FS="\n" }
/taskname/ && /status/ {
    taskname = ""; status = "";
    for (i = 1; i <= NF; i++) {
        if ($i ~ /^taskname[ \t]*:[ \t]*/) {
            sub(/^taskname[ \t]*:[ \t]*/, "", $i);
            taskname = $i;
        } else if ($i ~ /^status[ \t]*:[ \t]*/) {
            sub(/^status[ \t]*:[ \t]*/, "", $i);
            status = $i;
        }
    }

    if (taskname != "" && (status == "done" || status == "finished")) {
        sub(/\/$/, "", taskname);        # Remove trailing slash
        taskname = taskname suffix;      # Append suffix
        print taskname;                  # Output for Bash to use
    }
}' | while read -r task; do
    echo "Running: rucio download $task"
    rucio download "$task"
done

