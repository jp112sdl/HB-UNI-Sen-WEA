#!/usr/bin/env bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd $DIR
cd ../../

mkdir -p sketches
cd sketches

# Sketches from Git-Repositories
GIT_REPOS=(
  "https://github.com/jp112sdl/HB-UNI-Sen-WEA/HB-UNI-Sen-WEA.git"
)

# Clone / update Repos in parallel
for REPO_URL in ${GIT_REPOS[*]}; do
  REPO="$(basename $REPO_URL | cut -d. -f1)"
  if [ -e "$REPO" ] ; then
    echo "Pull changes from $REPO"
    (cd $REPO && git pull -q --depth 1) &
  else
    echo "Clone from $REPO"
    git clone -q --no-tags --depth 1 "$REPO_URL" &
  fi
done
wait
