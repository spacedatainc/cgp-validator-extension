# Copyright 2023 NVIDIA CORPORATION
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e
CWD="$( cd "$( dirname "$0" )" && pwd )"

# options defining what the script runs
HELP=false
CLEAN=false
CONFIG=release
HELP_EXIT_CODE=0

DIRECTORIES_TO_CLEAN=(
    _install
    _build
)

while [ $# -gt 0 ]
do
    if [[ "$1" == "--clean" ]]
    then
        CLEAN=true
    fi
    if [[ "$1" == "--debug" ]]
    then
        CONFIG=debug
    fi
    if [[ "$1" == "--relwithdebinfo" ]]
    then
        CONFIG=relwithdebinfo
    fi
    if [[ "$1" == "--help" ]]
    then
        HELP=true
    fi
    shift
done

# requesting how to run the script
if [[ "$HELP" == "true" ]]
then
    echo "build.sh [--clean] [--debug | --relwithdebinfo] [--help]"
    echo "--clean: Removes the following directories (customize as needed):"
    for dir_to_clean in "${DIRECTORIES_TO_CLEAN[@]}" ; do
        echo "      $dir_to_clean"
    done
    echo "--debug: Performs the steps with a debug configuration instead of release"
    echo "    (default = release)"
    echo "--relwithdebinfo: Performs the steps with a relwithdebinfo configuration instead of release"
    echo "    (default = release)"
    echo "--help: Display this help message"
    exit $HELP_EXIT_CODE
fi

# do we need to clean?
if [[ "$CLEAN" == "true" ]]
then
    for dir_to_clean in "${DIRECTORIES_TO_CLEAN[@]}" ; do
        rm -rf "$CWD/$dir_to_clean"
    done

    exit 0
fi

# perform a simple build seqeuence (customize as needed for your environment)
# pull OpenUSD and python dependencies as well as some helper cmake scripts
$CWD/tools/packman/python.sh scripts/setup.py --config=$CONFIG

cmake -B ./_build/cmake -DCMAKE_BUILD_TYPE=$CONFIG -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build ./_build/cmake --config $CONFIG --target install
