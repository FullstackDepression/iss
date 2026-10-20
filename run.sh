#!/usr/bin/env bash
# Обновить орбиту, собрать, запустить.
set -euo pipefail
cd "$(dirname "$0")"

# TLE старше 12 часов перекачиваю.
if [[ ! -s iss.tle ]] || [[ -n "$(find iss.tle -mmin +720 2>/dev/null)" ]]; then
    echo "Качаю TLE с CelesTrak..."
    curl -sf -m 30 'https://celestrak.org/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE' -o iss.tle.new \
        && mv iss.tle.new iss.tle \
        || { rm -f iss.tle.new; echo "Не скачалось, считаю по старому"; }
fi

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug >/dev/null
cmake --build build -j2 2>&1 | grep -E 'warning|error|предупреждение|ошибка' || true
./build/iss
