#!/bin/bash

passed=0
failed=0
quality=""

function run_test () {
    echo -n "running test \"$1\""
    #expect $AZADIR/scripts/testpos.exp "$2" "$3" > /dev/null
    expect $AZADIR/scripts/testpos.exp "$2" "$3" "$quality" >> test_protocoll.out
    if [ $? -eq 0 ];
    then
	echo "   PASSED"
	((passed=passed+1))
    else
	echo "   FAILED"
	echo "   with $2"
	echo "   expected $3"
	((failed=failed+1))
fi
}

echo "  ==============================="
echo "  ==  starting tactical tests  =="
echo "  ==============================="
echo ""

echo " :: RUNNING SIMPLE 1-2 MOVER"
quality="movetime 100"
run_test "simple promotion" "fen 8/6P1/8/8/8/k7/8/K7 w - - 0 1" "g7g8q"
run_test "simple knight fork" "fen 8/8/k3q3/8/8/3N4/5K2/8 w - - 0 1" "d3c5"
run_test "simple queen fork" "fen 5r2/8/1k6/2n5/8/6Q1/1K6/8 w - - 0 1" "g3d6"
run_test "simple rook capture" "fen 3r4/8/8/2k5/8/8/1K1R4/8 w - - 0 1" "d2d8"
run_test "simple queen capture" "fen 8/2k5/2q1p3/8/2PR4/2K2R2/8/8 b - - 0 1" "c6f3"
run_test "simple bishop mate" "fen k5B1/3N4/K7/8/8/8/8/8 w - - 0 1" "g8d5"
run_test "simple knight mate" "fen k7/3N4/K1N5/7p/8/8/8/8 w - - 0 1" "d7b6"
run_test "simple rook mate" "fen 2rk4/8/3K4/8/8/5R2/8/8 w - - 0 1" "f3f8"
run_test "simple queen mate" "fen 1k6/6n1/1K6/8/4Q3/8/8/8 w - - 0 1" "e4b7"
run_test "simple rook screwer" "fen 2r5/8/8/2k5/8/8/1K1R4/8 w - - 0 1" "d2c2"
run_test "simple bishop screwer" "fen 8/2b5/2k5/1b6/3K4/8/5Q2/8 b - - 0 1" "c7b6"

echo ""
echo " :: RUNNING MATES"
quality="movetime 1000"
run_test "mate in 1 #1" "fen r4b1r/ppp1kP2/3p1n1p/4p3/2BnP3/2NP1P1q/PPPQ3P/R1B2R1K b - - 2 13" "h3f1"
run_test "mate in 1 #2" "fen 8/1ppN1pp1/6k1/r2n2P1/6K1/8/p4P2/R7 w - - 2 42" "d7f8"
run_test "mate in 1 #3" "fen 8/2pk4/8/Rp2q3/6N1/BP5P/2PP4/2K5 b - - 6 41" "e5a1"
run_test "mate in 2 #1" "fen r2k1r2/ppp3Rp/2n1B3/3KB3/7P/8/PP3P2/n7 w - - 12 25" "e5c7"
run_test "mate in 2 #2" "fen 5r2/2R5/8/2p1k3/2PpB1P1/Pn1P2b1/1P2P3/3K4 b - - 0 36" "f8f1"
run_test "mate in 3 #1" "fen 5rk1/Q1R3p1/1n2p2p/1B1p1p2/3P1P2/4P1K1/P3q1PP/7R b - - 0 25" "e2e3"
run_test "mate in 3 #2" "fen r1bq1rk1/bpp1nppp/p7/P7/1P6/1B3Q2/1BpP2PP/RN3R1K w - - 0 16" "b3f7"
run_test "mate in 5 #1" "fen r1b3k1/3nqrp1/pp2pnQR/3p4/3P4/2P1P1P1/PP1N1PP1/2KR4 w - - 1 17" "d1h1"

echo ""
echo " :: RUNNING FREE PIECES"
quality="movetime 2000"
run_test "free knight" "fen r1bq1rk1/pppp1ppp/2n5/2b1N3/2B5/2P4P/PPP1Q1PK/R1B2R2 b - - 0 10" "c6e5"
run_test "free queen with mate in 4" "fen 8/8/8/8/2K2Q2/4k3/1p1q4/3b4 b - - 13 67" "e3f4"

echo ""
echo " :: RUNNING SACRIFICES"
quality="movetime 2000"
run_test "rook sac for queen" "fen 6k1/pq6/4pr2/6pp/P7/1P4PP/2Q2P2/4RK2 b - - 2 29" "b7h1"
run_test "exchange sac for fork" "fen r7/pkp4p/1pb2R2/8/1P4r1/P1N1bN2/2P3PP/3R3K w - - 1 22" "f6c6"
run_test "attack sac for queen" "fen 4r1k1/2R2ppp/3p4/B2n4/3N2q1/3Q2P1/1r3P1P/3R2K1 b - - 0 31" "d5f4"

echo ""
echo "total passed: $passed"
echo "total failed: $failed"
