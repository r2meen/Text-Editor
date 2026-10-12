
#!/usr/bin/env bash

# CS-218 | Commit Leaderboard

set -e

AUTHORS=("r2meen" "shifafareed79" "Muhammad-Saif07" "ibad-mirza")

# Terminal colors
RESET='\033[0m'
BOLD='\033[1m'
GOLD='\033[1;33m'
SILVER='\033[1;37m'
BRONZE='\033[0;33m'
CYAN='\033[1;36m'
GREEN='\033[1;32m'
PURPLE='\033[1;35m'

git fetch --all --quiet

declare -A commits
total=0

for author in "${AUTHORS[@]}"; do
    count=$(git log --all --no-merges \
        --author="$author" --format='%H' | sort -u | wc -l)

    commits["$author"]=$((count))
    total=$((total + count))
done

mapfile -t ranking < <(
    for author in "${AUTHORS[@]}"; do
        printf "%s %s\n" "${commits[$author]}" "$author"
    done | sort -rn -k1,1 -k2,2 | cut -d' ' -f2-
)

printf "\n${CYAN}${BOLD}"
echo "                CS-218 COMMIT LEADERBOARD"
printf "${RESET}\n"

printf "${BOLD}%-8s %-22s %12s %12s${RESET}\n" \
    "RANK" "USERNAME" "COMMITS" "SHARE"

echo "----------------------------------------------------------"

rank=1

for author in "${ranking[@]}"; do
    count=${commits[$author]}

    if (( total > 0 )); then
        share=$(awk -v c="$count" -v t="$total" \
            'BEGIN { printf "%.1f", c * 100 / t }')
    else
        share="0.0"
    fi

    printf "${CYAN}%-8s${RESET} %-22s %12s %11s%%\n" \
        "#$rank" "$author" "$count" "$share"

    rank=$((rank + 1))
done

echo "----------------------------------------------------------"
printf "${GREEN}${BOLD}TOTAL COMMITS: %s${RESET}\n\n" "$total"