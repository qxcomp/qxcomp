
mydir=$(dirname $(realpath $0))
set -x
cd "$mydir" && git pull --ff --rebase
