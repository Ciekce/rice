cutechess-cli -engine cmd="./Rice" name="dev" proto="uci" -engine cmd="./binaries/RiceHCE" name="master" proto="uci" -each tc=inf/6+0.06 -openings file=./pgns/8moves_v3.pgn order=random  -games 2 -rounds 2500 -repeat 2 -maxmoves 200 -sprt elo0=0 elo1=5   alpha=0.05 beta=0.05 -concurrency 6 -ratinginterval 10 -pgnout "./pgns/vshce.pgn" 