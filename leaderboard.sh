#!/usr/bin/env bash

# CS-218 | Commit Leaderboard
# Run from inside the project's git repository:  bash leaderboard.sh

set -uo pipefail

# Each entry: "Display name|pattern|pattern|..."
# Patterns are case-insensitive regexes matched against the commit author's
# NAME and EMAIL (not the GitHub username, which git doesn't store).
# If someone shows up under "not matched" below, add their name/email here.

MEMBERS=(
    "r2meen|r2meen|rameen|rameenzulfi0@gmail\.com"
    "shifafareed79|shifafareed79|shifa"
    "Muhammad-Saif07|muhammad-saif07|saif"
    "ibad-mirza|ibad-mirza|ibad"
)

# Terminal colors (real escape characters, so they work with printf %s and echo)
RESET=$'\033[0m'
BOLD=$'\033[1m'
GOLD=$'\033[1;33m'
SILVER=$'\033[1;37m'
BRONZE=$'\033[0;33m'
CYAN=$'\033[1;36m'
GREEN=$'\033[1;32m'
PURPLE=$'\033[1;35m'

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Error: run this script inside the project's git repository." >&2
    exit 1
fi

# A failed fetch (offline, no remote, auth) should not kill the leaderboard
if ! git fetch --all --quiet 2>/dev/null; then
    echo "Warning: git fetch failed - using local history only." >&2
fi

# One line per unique non-merge commit: hash<TAB>author name<TAB>author email
log=$(git log --all --no-merges --format='%H%x09%aN%x09%aE')

# Assign every commit to the first member whose patterns match (no double counting)
results=$(printf '%s\n' "$log" | MEMBERS_STR="$(printf '%s\n' "${MEMBERS[@]}")" awk -F'\t' '
BEGIN {
    n = split(ENVIRON["MEMBERS_STR"], lines, "\n")
    for (i = 1; i <= n; i++) {
        if (lines[i] == "") continue
        m++
        k = split(lines[i], parts, "|")
        name[m] = parts[1]; pat[m] = ""; count[m] = 0
        for (j = 2; j <= k; j++)
            pat[m] = pat[m] (pat[m] == "" ? "" : "|") tolower(parts[j])
    }
}
$1 == ""       { next }
seen[$1]++     { next }
{
    who = tolower($2 " " $3); matched = 0
    for (i = 1; i <= m; i++)
        if (who ~ pat[i]) { count[i]++; matched = 1; break }
    if (!matched) other[$2 " <" $3 ">"]++
}
END {
    for (i = 1; i <= m; i++) print "M\t" count[i] "\t" name[i]
    for (o in other)         print "U\t" other[o] "\t" o
}')

# Highest commits first; ties broken alphabetically
ranking=$(printf '%s\n' "$results" | awk -F'\t' '$1 == "M" { print $2 "\t" $3 }' \
    | sort -t$'\t' -k1,1nr -k2,2)
total=$(printf '%s\n' "$ranking" | awk -F'\t' '{ s += $1 } END { print s + 0 }')

printf "\n%s%s                CS-218 COMMIT LEADERBOARD%s\n\n" "$CYAN" "$BOLD" "$RESET"
printf "%s%-8s %-22s %12s %12s%s\n" "$BOLD" "RANK" "USERNAME" "COMMITS" "SHARE" "$RESET"
echo "----------------------------------------------------------"

rank=0
while IFS=$'\t' read -r count author; do
    [ -z "${author:-}" ] && continue
    rank=$((rank + 1))

    case $rank in
        1) color=$GOLD ;;
        2) color=$SILVER ;;
        3) color=$BRONZE ;;
        *) color=$CYAN ;;
    esac

    if [ "$total" -gt 0 ]; then
        share=$(awk -v c="$count" -v t="$total" 'BEGIN { printf "%.1f", c * 100 / t }')
    else
        share="0.0"
    fi

    printf "%s%-8s%s %-22s %12s %11s%%\n" "$color" "#$rank" "$RESET" "$author" "$count" "$share"
done <<< "$ranking"

echo "----------------------------------------------------------"
printf "%s%sTOTAL COMMITS: %s%s\n\n" "$GREEN" "$BOLD" "$total" "$RESET"

# Show commits that didn't match anyone, so the MEMBERS patterns can be fixed
unmatched=$(printf '%s\n' "$results" | awk -F'\t' '$1 == "U" { print $2 "\t" $3 }' \
    | sort -t$'\t' -k1,1nr)
if [ -n "$unmatched" ]; then
    printf "%sCommits not matched to any member:%s\n" "$PURPLE" "$RESET"
    while IFS=$'\t' read -r c who; do
        printf "  %5s  %s\n" "$c" "$who"
    done <<< "$unmatched"
    echo "Add these names/emails to MEMBERS at the top of the script to count them."
    echo
fi

// to stalk git history
// git log --all --no-merges --format='%aN <%aE>' | sort | uniq -c | sort -rn 