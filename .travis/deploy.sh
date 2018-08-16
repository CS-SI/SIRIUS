#!/bin/sh

set -ev

if [ "x${TRAVIS_PULL_REQUEST}" = "xfalse" ]; then

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
