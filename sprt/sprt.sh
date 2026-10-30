#!/usr/bin/env bash
# Run an SPRT match between two engines with fastchess.
set -euo pipefail

# ------------------------------- settings ------------------------------------
NEW_ENGINE="./azalea_0.1.13_impr"          # engine under test
NEW_NAME="azalea_0.1.13_impr"
BASE_ENGINE="./azalea_0.1.12"        # reference engine
BASE_NAME="azalea_0.1.12"

BOOK="./UHO_Lichess_4852_v1.epd"
BOOK_FORMAT="epd"

TC="5+0.05"                        # seconds per game + increment per move
HASH=16                            # MB per engine
CONCURRENCY=8                      # parallel games, below your physical core count
ROUNDS=20000                       # upper limit, 2 games per round

ELO0=0                             # gainer: 0 / 5, simplification: -5 / 0
ELO1=5
ALPHA=0.05
BETA=0.05

RATING_INTERVAL=20                 # print Elo/LLR every N games
LOG_DIR="./sprt-logs"
# -----------------------------------------------------------------------------

# sanity checks
command -v fastchess >/dev/null || { echo "error: fastchess not found in PATH" >&2; exit 1; }
for f in "$NEW_ENGINE" "$BASE_ENGINE"; do
    [[ -x "$f" ]] || { echo "error: engine '$f' not found or not executable" >&2; exit 1; }
done
[[ -f "$BOOK" ]] || { echo "error: opening book '$BOOK' not found" >&2; exit 1; }

mkdir -p "$LOG_DIR"
STAMP="$(date +%Y%m%d-%H%M%S)"
BASENAME="${LOG_DIR}/${NEW_NAME}_vs_${BASE_NAME}_${STAMP}"
LOG_FILE="${BASENAME}.log"
PGN_FILE="${BASENAME}.pgn"

cat <<EOF
==================== SPRT ====================
New engine   : ${NEW_NAME}  (${NEW_ENGINE})
Base engine  : ${BASE_NAME}  (${BASE_ENGINE})
Opening book : ${BOOK} (${BOOK_FORMAT})
Time control : ${TC}
Hash         : ${HASH} MB
Concurrency  : ${CONCURRENCY}
Max rounds   : ${ROUNDS} (x2 games)
SPRT bounds  : elo0=${ELO0} elo1=${ELO1} alpha=${ALPHA} beta=${BETA}
Log file     : ${LOG_FILE}
PGN file     : ${PGN_FILE}
==============================================
EOF

fastchess \
    -engine cmd="$NEW_ENGINE" name="$NEW_NAME" \
    -engine cmd="$BASE_ENGINE" name="$BASE_NAME" \
    -each tc="$TC" option.Hash="$HASH" \
    -openings file="$BOOK" format="$BOOK_FORMAT" order=random \
    -games 2 -rounds "$ROUNDS" -repeat \
    -concurrency "$CONCURRENCY" \
    -sprt elo0="$ELO0" elo1="$ELO1" alpha="$ALPHA" beta="$BETA" \
    -ratinginterval "$RATING_INTERVAL" \
    -pgnout file="$PGN_FILE" \
    -draw movenumber=40 movecount=8 score=10 \
    -resign movecount=3 score=600 twosided=true \
    -recover \
    2>&1 | tee "$LOG_FILE"
