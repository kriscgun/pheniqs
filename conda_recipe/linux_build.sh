#!/usr/bin/env bash

echo "Building conda package on linux"

set -x -e

env

if [ "$TRAVIS_BRANCH" == "master" ]; then
	docker run \
		-e ANACONDA_API_TOKEN=$ANACONDA_API_TOKEN \
		-e TRAVIS_OS_NAME=$TRAVIS_OS_NAME \
		-e TRAVIS_BRANCH=$TRAVIS_BRANCH \
		-it -v `pwd`:/bioconda condaforge/linux-anvil /bioconda/conda_build.sh
else
	echo "This is not the master branch, no build"
	exit 0
fi
