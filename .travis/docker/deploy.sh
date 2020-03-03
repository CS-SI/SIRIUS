#!/bin/bash

# This script will update gh-pages with current doc and update sirius docker containers (sirius_built and sirius)

set -ev

if [[ ${TRAVIS_OS_NAME} != 'linux' ]];
    # noop if not linux
    exit 0
fi

cd "${TRAVIS_BUILD_DIR}"

# update gh-pages branch on CI-SI/SIRIUS repository only
if [ "x${TRAVIS_REPO_SLUG}" = "xCS-SI/SIRIUS" ]; then
    openssl aes-256-cbc -K $encrypted_f05cf190aee2_key -iv $encrypted_f05cf190aee2_iv -in .travis/github_deploy_key.enc -out github_deploy_key -d
    chmod 600 github_deploy_key
    eval `ssh-agent -s`
    ssh-add github_deploy_key

    mkdir doc/sphinx/build
    docker run -v $TRAVIS_BUILD_DIR:/data ldumas/sirius_dockerfile:sirius_pages /bin/sh -c "cd /data; mv doxy_xml doc/sphinx/source/ ; cd doc/sphinx/ ; ln -s source/doxy_xml . "
    docker run -v $TRAVIS_BUILD_DIR:/data ldumas/sirius_dockerfile:sirius_pages /bin/sh -c "cd /data/doc/sphinx; make html"
    docker run -v $TRAVIS_BUILD_DIR:/data ldumas/sirius_dockerfile:sirius_pages /bin/sh -c "cd /data; mv doxy_html doc/sphinx/build/"
    cd $TRAVIS_BUILD_DIR/doc/sphinx/build/
    touch .nojekyll
    git init
    git checkout -b gh-pages
    git add .
    git -c user.name='travis' -c user.email='travis' commit -m init
    git push -q -f git@github.com:${TRAVIS_REPO_SLUG}.git gh-pages &2>/dev/null

    cd "${TRAVIS_BUILD_DIR}"
fi

if [ "x${TRAVIS_PULL_REQUEST}" = "xfalse" ]  && [ "x$DOCKER_USERNAME" != "x" ] && [ "x$DOCKER_PASSWORD" != "x" ]; then
    docker login -u $DOCKER_USERNAME -p $DOCKER_PASSWORD

    # update sirius built container on docker hub
    docker commit --change "ENV PATH $PATH:/opt/sirius/bin" ${BUILD_CONTAINER_ID} sirius_built
    docker tag sirius_built ldumas/sirius_dockerfile:sirius_built
    docker push ldumas/sirius_dockerfile:sirius_built

    # create sirius container with sirius executable as entry point
    cat >Dockerfile <<EOF
FROM ldumas/sirius_dockerfile:sirius_built

ENTRYPOINT ["/opt/sirius/bin/sirius"]
EOF

    # build
    docker build -t sirius .
    # tag the image
    docker tag sirius ldumas/sirius_dockerfile:sirius
    # push all ldumas/sirius_dockerfile images
    docker push ldumas/sirius_dockerfile:sirius

fi
